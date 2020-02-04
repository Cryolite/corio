#if !defined(CORIO_THREAD_UNSAFE_CONDITION_VARIABLE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_CONDITION_VARIABLE_HPP_INCLUDE_GUARD

#include <corio/core/enable_if_executor.hpp>
#include <corio/core/is_executor.hpp>
#include <corio/core/enable_if_execution_context.hpp>
#include <corio/util/enable_if_constructible.hpp>
#include <corio/util/assert.hpp>
#include <boost/asio/basic_waitable_timer.hpp>
#include <boost/asio/defer.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/executor.hpp>
#include <boost/system/error_code.hpp>
#include <condition_variable>
#include <chrono>
#include <list>
#include <type_traits>
#include <functional>
#include <memory>
#include <tuple>
#include <utility>
#include <cstddef>


namespace corio::thread_unsafe{

template<typename Executor>
class basic_condition_variable
{
public:
  using executor_type = Executor;
  static_assert(corio::is_executor_v<Executor>);

private:
  using handler_type_ = std::function<void(std::cv_status)>;
  using predicate_type_ = std::function<bool()>;
  using work_guard_type_ = boost::asio::executor_work_guard<executor_type>;
  using value_type_ = std::tuple<
    handler_type_, predicate_type_, std::shared_ptr<void>, work_guard_type_, bool>;
  using handler_list_type_ = std::list<value_type_>;
  using iterator_type_ = typename handler_list_type_::iterator;
  using lists_type_ = std::tuple<handler_list_type_, handler_list_type_>;

public:
  explicit basic_condition_variable(executor_type const &executor)
    : basic_condition_variable(executor_type(executor))
  {}

  explicit basic_condition_variable(executor_type &&executor)
    : executor_(std::move(executor)),
      p_lists_(std::make_shared<lists_type_>())
  {}

  template<typename ExecutionContext>
  explicit basic_condition_variable(
    ExecutionContext &ctx,
    corio::enable_if_execution_context_t<ExecutionContext> * = nullptr,
    corio::disable_if_executor_t<ExecutionContext> * = nullptr,
    corio::enable_if_constructible_t<executor_type, typename ExecutionContext::executor_type> * = nullptr)
    : executor_(ctx.get_executor()),
      p_lists_(std::make_shared<lists_type_>())
  {}

  basic_condition_variable(basic_condition_variable const &) = delete;

  basic_condition_variable &operator=(basic_condition_variable const &) = delete;

  executor_type get_executor() const
  {
    return executor_;
  }

  void notify_one()
  {
    auto &[handler_queue, disposal_list] = *p_lists_;
    if (!handler_queue.empty()) {
      auto &[handler, predicate, p_timer, work_guard, flag] = handler_queue.front();
      if (predicate()) {
        boost::asio::post(executor_, std::bind(std::move(handler), std::cv_status::no_timeout));
        if (flag) {
          // `handler_queue_.begin()` is not subject to timeout.
          handler_queue.pop_front();
        }
        else {
          // `handler_queue_.begin()` is subject to timeout.
          p_timer.reset();
          work_guard.reset();
          flag = true;
          disposal_list.splice(disposal_list.cend(), handler_queue, handler_queue.cbegin());
        }
      }
      else {
        handler_queue.splice(handler_queue.cend(), handler_queue, handler_queue.cbegin());
      }
    }
  }

  void notify_all()
  {
    auto &[handler_queue, disposal_list] = *p_lists_;
    std::size_t const size = handler_queue.size();
    for (std::size_t i = 0u; i < size; ++i) {
      notify_one();
    }
  }

  template<typename CompletionToken>
  auto async_wait(CompletionToken &&token)
  {
    using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void()>;
    using completion_handler_type = typename async_result_type::completion_handler_type;
    completion_handler_type completion_handler(std::move(token));
    async_result_type async_result(completion_handler);
    auto &[handler_queue, disposal_list] = *p_lists_;
    handler_queue.emplace_back(
      [h = std::move(completion_handler)]([[maybe_unused]] std::cv_status status) mutable -> void{
        CORIO_ASSERT(status == std::cv_status::no_timeout);
        std::move(h)();
      }, []() -> bool{ return true; }, nullptr, work_guard_type_(executor_), true);
    return async_result.get();
  }

  template<typename Predicate, typename CompletionToken>
  auto async_wait(Predicate predicate, CompletionToken &&token)
  {
    using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void()>;
    using completion_handler_type = typename async_result_type::completion_handler_type;
    completion_handler_type completion_handler(std::move(token));
    async_result_type async_result(completion_handler);
    if (predicate()) {
      boost::asio::defer(executor_, std::move(completion_handler));
      return async_result.get();
    }
    auto &[handler_queue, disposal_list] = *p_lists_;
    handler_queue.emplace_back(
      [h = std::move(completion_handler)]([[maybe_unused]] std::cv_status status) mutable -> void{
        CORIO_ASSERT(status == std::cv_status::no_timeout);
        std::move(h)();
      }, std::move(predicate), nullptr, work_guard_type_(executor_), true);
    return async_result.get();
  }

  template<typename Clock, typename Duration, typename CompletionToken>
  auto async_wait_until(std::chrono::time_point<Clock, Duration> const &abs_time, CompletionToken &&token)
  {
    using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void(std::cv_status)>;
    using completion_handler_type = typename async_result_type::completion_handler_type;
    completion_handler_type completion_handler(std::move(token));
    async_result_type async_result(completion_handler);
    auto &[handler_queue, disposal_list] = *p_lists_;
    handler_queue.emplace_back(
      [h = std::move(completion_handler)]([[maybe_unused]] std::cv_status status) mutable -> void{
        std::move(h)(status);
      }, []() -> bool{ return true; }, nullptr, work_guard_type_(executor_), false);
    iterator_type_ iter = handler_queue.end();
    --iter;
    using time_point_type = std::chrono::time_point<Clock, Duration>;
    using clock_type = typename time_point_type::clock;
    using timer_type = boost::asio::basic_waitable_timer<clock_type>;
    auto p_timer = std::make_shared<timer_type>(executor_, abs_time);
    p_timer->async_wait(
      [executor = executor_, p_lists = std::weak_ptr<lists_type_>(p_lists_), iter](
        [[maybe_unused]] boost::system::error_code const &ec) mutable -> void{
        if (auto p = p_lists.lock()) {
          auto &[handler_queue, disposal_list] = *p;
          auto &[handler, predicate, p_timer, work_guard, flag] = *iter;
          if (flag) {
            // The handler has been already invoked, and `iter` refers to an
            // element of `disposal_list`.
            CORIO_ASSERT(ec.value() == boost::system::errc::operation_canceled);
            disposal_list.erase(iter);
          }
          else {
            // The handler has not been invoked yet and is canceled, and `iter`
            // refers to an element of `handler_queue`.
            CORIO_ASSERT(!ec);
            boost::asio::post(std::move(executor), std::bind(std::move(handler), std::cv_status::timeout));
            handler_queue.erase(iter);
          }
        }
      });
    std::get<2u>(*iter) = std::move(p_timer);
    return async_result.get();
  }

  template<typename Clock, typename Duration, typename Predicate, typename CompletionToken>
  auto async_wait_until(std::chrono::time_point<Clock, Duration> const &abs_time,
                        Predicate predicate, CompletionToken &&token)
  {
    using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void(std::cv_status)>;
    using completion_handler_type = typename async_result_type::completion_handler_type;
    completion_handler_type completion_handler(std::move(token));
    async_result_type async_result(completion_handler);
    if (predicate()) {
      boost::asio::defer(executor_, std::bind(std::move(completion_handler), std::cv_status::no_timeout));
      return async_result.get();
    }
    auto &[handler_queue, disposal_list] = *p_lists_;
    handler_queue.emplace_back(
      [h = std::move(completion_handler)]([[maybe_unused]] std::cv_status status) mutable -> void{
        std::move(h)(status);
      }, std::move(predicate), nullptr, work_guard_type_(executor_), false);
    iterator_type_ iter = handler_queue.end();
    --iter;
    using time_point_type = std::chrono::time_point<Clock, Duration>;
    using clock_type = typename time_point_type::clock;
    using timer_type = boost::asio::basic_waitable_timer<clock_type>;
    auto p_timer = std::make_shared<timer_type>(executor_, abs_time);
    p_timer->async_wait(
      [executor = executor_, p_lists = std::weak_ptr<lists_type_>(p_lists_), iter](
        [[maybe_unused]] boost::system::error_code const &ec) mutable -> void{
          if (auto p = p_lists.lock()) {
            auto &[handler_queue, disposal_list] = *p;
            auto &[handler, predicate, p_timer, work_guard, flag] = *iter;
            if (flag) {
              // The handler has been already invoked, and `iter` refers to an
              // element of `disposal_list`.
              CORIO_ASSERT(ec.value() == boost::system::errc::operation_canceled);
              disposal_list.erase(iter);
            }
            else {
              // The handler has not been invoked yet and is canceled, and `iter`
              // refers to an element of `handler_queue`.
              CORIO_ASSERT(!ec);
              boost::asio::post(std::move(executor), std::bind(std::move(handler), std::cv_status::timeout));
              handler_queue.erase(iter);
            }
          }
        });
    std::get<2u>(*iter) = std::move(p_timer);
    return async_result.get();
  }

  template<typename Rep, typename Period, typename CompletionToken>
  auto async_wait_for(std::chrono::duration<Rep, Period> const &rel_time, CompletionToken &&token)
  {
    return async_wait_until(std::chrono::steady_clock::now() + rel_time, std::move(token));
  }

  template<typename Rep, typename Period, typename Predicate, typename CompletionToken>
  auto async_wait_for(std::chrono::duration<Rep, Period> const &rel_time,
                      Predicate predicate, CompletionToken &&token)
  {
    return async_wait_until(std::chrono::steady_clock::now() + rel_time,
                            std::move(predicate), std::move(token));
  }

private:
  executor_type executor_;
  std::shared_ptr<lists_type_> p_lists_;
}; // class basic_condition_variable

using condition_variable = basic_condition_variable<boost::asio::executor>;

} // namespace corio::thread_unsafe

#endif // !defined(CORIO_THREAD_UNSAFE_CONDITION_VARIABLE_HPP_INCLUDE_GUARD)
