#if !defined(CORIO_THREAD_UNSAFE_DETAIL_MUTEX_SERVICE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_DETAIL_MUTEX_SERVICE_HPP_INCLUDE_GUARD

#include <corio/util/assert.hpp>
#include <boost/asio/defer.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/execution_context.hpp>
#include <unordered_map>
#include <deque>
#include <type_traits>
#include <functional>
#include <tuple>
#include <utility>


namespace corio::thread_unsafe::detail_{

template<class ExecutionContext>
class mutex_service_
  : public boost::asio::execution_context::service
{
public:
  using context_type = ExecutionContext;

private:
  static_assert(!std::is_const_v<context_type>);
  static_assert(!std::is_volatile_v<context_type>);
  static_assert(!std::is_reference_v<context_type>);
  static_assert(std::is_convertible_v<context_type &, boost::asio::execution_context &>);

  using callback_queue_type_ = std::deque<std::function<void()> >;
  using callback_queue_map_type_ = std::unordered_map<bool *, callback_queue_type_>;
  using executor_type_ = typename context_type::executor_type;

public:
  static inline boost::asio::execution_context::id id;

  explicit mutex_service_(boost::asio::execution_context &ctx)
    : service(ctx),
      p_ctx_(),
      map_()
  {}

  mutex_service_(boost::asio::execution_context &ctx, context_type &ctxx)
    : service(ctx),
      p_ctx_(&ctxx),
      map_()
  {
    CORIO_ASSERT(&ctx == static_cast<boost::asio::execution_context *>(&ctxx));
  }

  mutex_service_(mutex_service_ const &) = delete;

  mutex_service_ &operator=(mutex_service_ const &) = delete;

  virtual void shutdown() noexcept override
  {
    map_.clear();
  }

  void async_lock(bool &locked, std::function<void()> &&h)
  {
    CORIO_ASSERT(p_ctx_ != nullptr);

    if (!locked) {
      executor_type_ ex = p_ctx_->get_executor();
      locked = true;
      boost::asio::defer(ex, std::move(h));
      return;
    }

    auto iter = map_.find(&locked);
    if (iter == map_.cend()) {
      std::tie(iter, std::ignore) = map_.emplace(&locked, callback_queue_type_());
    }
    iter->second.emplace_back(std::move(h));
  }

  void unlock(bool &locked)
  {
    CORIO_ASSERT(p_ctx_ != nullptr);
    CORIO_ASSERT(locked);

    auto iter = map_.find(&locked);
    if (iter == map_.cend() || iter->second.size() == 0u) {
      locked = false;
      return;
    }

    std::function<void()> h = std::move(iter->second.front());
    iter->second.pop_front();
    executor_type_ ex = p_ctx_->get_executor();
    boost::asio::post(ex, std::move(h));
  }

private:
  context_type *p_ctx_;
  callback_queue_map_type_ map_;
}; // class mutex_service_

} // namespace corio::thread_unsafe::detail_

#endif // !defined(CORIO_THREAD_UNSAFE_DETAIL_MUTEX_SERVICE_HPP_INCLUDE_GUARD)
