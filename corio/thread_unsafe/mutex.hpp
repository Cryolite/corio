#if !defined(CORIO_THREAD_UNSAFE_MUTEX_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_MUTEX_HPP_INCLUDE_GUARD

#include <corio/core/enable_if_executor.hpp>
#include <corio/core/is_executor.hpp>
#include <corio/core/enable_if_execution_context.hpp>
#include <corio/util/enable_if_constructible.hpp>
#include <corio/util/exception_guard.hpp>
#include <corio/util/assert.hpp>
#include <boost/asio/defer.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/executor.hpp>
#include <boost/asio/async_result.hpp>
#include <mutex>
#include <deque>
#include <type_traits>
#include <utility>


namespace corio::thread_unsafe{

template<typename Executor>
class basic_mutex;

namespace detail_{

class mutex_completion_handler_
{
private:
  class impl_base_
  {
  public:
    virtual ~impl_base_()
    {}

    virtual void adopt_lock(void *p) noexcept = 0;

    virtual void operator()() = 0;
  }; // class impl_base_

  template<typename F, typename Executor>
  class impl_ final
    : public impl_base_
  {
  private:
    using decayed_type_ = std::decay_t<F>;
    using executor_type_ = Executor;
    static_assert(corio::is_executor_v<executor_type_>);
    using mutex_type_ = basic_mutex<executor_type_>;
    using lock_type_ = std::unique_lock<mutex_type_>;

  public:
    explicit impl_(F &&f)
      : f_(std::move(f)),
        lock_()
    {}

    impl_(impl_ const &) = delete;

    impl_ &operator=(impl_ const &) = delete;

  private:
    virtual void adopt_lock(void *p) noexcept override final
    {
      // The only callers, `mutex_type_::async_lock` and `mutex_type_::unlock`,
      // guarantee that the dynamic type of the object pointed to by `p` is
      // `mutex_type_`.
      CORIO_ASSERT(lock_.mutex() == nullptr);
      CORIO_ASSERT(p != nullptr);
      mutex_type_ &mutex = *static_cast<mutex_type_ *>(p);
      lock_ = lock_type_(mutex, std::adopt_lock);
    }

    virtual void operator()() override final
    {
      CORIO_ASSERT(lock_.mutex() != nullptr);
      CORIO_ASSERT(lock_.owns_lock());
      std::move(f_)(std::move(lock_));
    }

  private:
    decayed_type_ f_;
    lock_type_ lock_;
  }; // class impl_

public:
  template<typename F, typename Executor>
  explicit mutex_completion_handler_(
    F &&f, std::type_identity<Executor>, corio::enable_if_executor_t<Executor> * = nullptr)
    : p_(new impl_<F, Executor>(std::forward<F>(f)))
  {}

  mutex_completion_handler_(mutex_completion_handler_ const &) = delete;

  mutex_completion_handler_(mutex_completion_handler_ &&rhs) noexcept
    : p_(rhs.p_)
  {
    rhs.p_ = nullptr;
  }

  ~mutex_completion_handler_()
  {
    if (p_ != nullptr) {
      delete p_;
    }
  }

  mutex_completion_handler_ &operator=(mutex_completion_handler_ const &) = delete;

  void adopt_lock(void *p) noexcept
  {
    CORIO_ASSERT(p_ != nullptr);
    CORIO_ASSERT(p != nullptr);
    p_->adopt_lock(p);
  }

  void operator()()
  {
    CORIO_ASSERT(p_ != nullptr);
    (*p_)();
  }

private:
  impl_base_ *p_;
}; // class mutex_completion_handler_

} // namespace detail_

template<typename Executor>
class basic_mutex
{
public:
  using executor_type = Executor;
  static_assert(corio::is_executor_v<executor_type>);
  using lock_type = std::unique_lock<basic_mutex<executor_type> >;

private:
  using handler_queue_type_ = std::deque<detail_::mutex_completion_handler_>;

public:
  explicit basic_mutex(executor_type const &executor)
    : basic_mutex(executor_type(executor))
  {}

  explicit basic_mutex(executor_type &&executor)
    : executor_(std::move(executor)),
      handler_queue_(),
      locked_()
  {}

  template<typename ExecutionContext>
  explicit basic_mutex(
    ExecutionContext &ctx,
    corio::enable_if_execution_context_t<ExecutionContext> * = nullptr,
    corio::disable_if_executor_t<ExecutionContext> * = nullptr,
    corio::enable_if_constructible_t<executor_type, typename ExecutionContext::executor_type> * = nullptr)
    : basic_mutex(ctx.get_executor())
  {}

  basic_mutex(basic_mutex const &) = delete;

  basic_mutex &operator=(basic_mutex const &) = delete;

  executor_type get_executor() const
  {
    return executor_;
  }

  template<typename CompletionToken>
  auto async_lock(CompletionToken &&token)
  {
    using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void(lock_type)>;
    using completion_handler_type = typename async_result_type::completion_handler_type;
    completion_handler_type completion_handler(std::forward<CompletionToken>(token));
    async_result_type async_result(completion_handler);
    if (!locked_) {
      locked_ = true;
      CORIO_EXCEPTION_GUARD(eg){
        locked_ = false;
      };
      detail_::mutex_completion_handler_ h(std::move(completion_handler), std::type_identity<executor_type>());
      h.adopt_lock(this);
      eg.dismiss();
      boost::asio::defer(executor_, std::move(h));
    }
    else {
      detail_::mutex_completion_handler_ h(std::move(completion_handler), std::type_identity<executor_type>());
      handler_queue_.emplace_back(std::move(h));
    }
    return async_result.get();
  }

  bool try_lock()
  {
    return locked_ ? false : (locked_ = true);
  }

  void unlock()
  {
    CORIO_ASSERT(locked_);
    locked_ = false;
    if (!handler_queue_.empty()) {
      locked_ = true;
      CORIO_EXCEPTION_GUARD(eg){
        locked_ = false;
      };
      detail_::mutex_completion_handler_ h = std::move(handler_queue_.front());
      handler_queue_.pop_front();
      h.adopt_lock(this);
      eg.dismiss();
      // Pray to God that the following line does not throw an exception.
      // Otherwise, because `h` already owns the lock, `basic_mutex::unlock` is
      // called again from the `h`'s destructor during stack unwinding, which
      // may in turn evaluates the following line again...
      boost::asio::post(executor_, std::move(h));
    }
  }

private:
  executor_type executor_;
  handler_queue_type_ handler_queue_;
  bool locked_;
}; // class basic_mutex

using mutex = basic_mutex<boost::asio::executor>;

} // namespace corio::thread_unsafe

#endif // !defined(CORIO_THREAD_UNSAFE_MUTEX_HPP_INCLUDE_GUARD)
