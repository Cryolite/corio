#if !defined(CORIO_THREAD_UNSAFE_COROUTINE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_COROUTINE_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/future.hpp>
#include <corio/thread_unsafe/detail_/coroutine_cleanup_service_.hpp>
#include <corio/core/is_execution_context.hpp>
#include <corio/core/is_executor.hpp>
#include <corio/core/error.hpp>
#include <corio/util/throw.hpp>
#include <corio/util/assert.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/executor.hpp>
#include <boost/config.hpp>
#include <type_traits>
#include <utility>
#include <experimental/coroutine>


namespace corio::thread_unsafe{

template<typename R, typename Executor>
class basic_coroutine;

namespace detail_{

template<typename SourceExecutor, typename R, typename TargetExecutor, typename F, typename... Args>
struct enable_if_postable_executor_
  : public std::enable_if<
      corio::is_executor_v<std::decay_t<SourceExecutor> >
      && corio::is_executor_v<TargetExecutor>
      && std::is_constructible_v<TargetExecutor, SourceExecutor>
      && std::is_invocable_r_v<basic_coroutine<R, TargetExecutor>, F, Args &&...>,
      corio::thread_unsafe::basic_future<R, TargetExecutor> >
{};

template<typename SourceExecutor, typename R, typename TargetExecutor, typename F, typename... Args>
using enable_if_postable_executor_t_ = typename enable_if_postable_executor_<
  SourceExecutor, R, TargetExecutor, F, Args...>::type;

template<typename ExecutionContext, typename R, typename Executor, typename F, typename... Args>
struct enable_if_postable_execution_context_
  : public std::enable_if<
      corio::is_execution_context_v<ExecutionContext>
      && corio::is_executor_v<Executor>
/*&& std::is_constructible_v<Executor, typename ExecutionContext::executor_type>*/
      && std::is_invocable_r_v<basic_coroutine<R, Executor>, F, Args &&...>,
      corio::thread_unsafe::basic_future<R, Executor> >
{};

template<typename ExecutionContext, typename R, typename Executor, typename F, typename... Args>
using enable_if_postable_execution_context_t_ = typename enable_if_postable_execution_context_<
  ExecutionContext, R, Executor, F, Args...>::type;

} // namespace detail_

template<typename SourceExecutor, typename R, typename TargetExecutor, typename... Params, typename... Args>
auto post(SourceExecutor &&executor,
          corio::thread_unsafe::basic_coroutine<R, TargetExecutor> (*pf)(Params...),
          Args &&... args)
  -> detail_::enable_if_postable_executor_t_<SourceExecutor, R, TargetExecutor, decltype(pf), Args...>
{
  basic_coroutine coro = (*pf)(std::forward<Args>(args)...);
  coro.set_executor(std::forward<SourceExecutor>(executor));
  corio::thread_unsafe::basic_future<R, TargetExecutor> future = coro.get_future();
  TargetExecutor e = coro.get_executor();
  boost::asio::post(std::move(e), [coro_ = std::move(coro)]() mutable -> void{ coro_.resume(); });
  return future;
}

template<typename ExecutionContext, typename R, typename Executor, typename... Params, typename... Args>
auto post(ExecutionContext &context,
          corio::thread_unsafe::basic_coroutine<R, Executor> (*pf)(Params...),
          Args &&... args)
  -> detail_::enable_if_postable_execution_context_t_<ExecutionContext, R, Executor, decltype(pf), Args...>
{
  return corio::thread_unsafe::post(context.get_executor(), pf, std::forward<Args>(args)...);
}

template<typename R, typename Executor>
class coroutine_promise;

template<typename R, typename Executor>
class basic_coroutine
{
public:
  using executor_type = Executor;
  static_assert(corio::is_executor_v<executor_type>);
  using promise_type = coroutine_promise<R, executor_type>;
  using future_type = basic_future<R, executor_type>;

private:
  using raw_handle_type_ = std::experimental::coroutine_handle<promise_type>;
  using handle_type_ = corio::thread_unsafe::detail_::coroutine_handle_;
  friend class corio::thread_unsafe::coroutine_promise<R, executor_type>;
  template<typename SourceExecutor, typename RR, typename TargetExecutor, typename... Params, typename... Args>
  friend auto corio::thread_unsafe::post(
    SourceExecutor &&, basic_coroutine<RR, TargetExecutor> (*pf)(Params...), Args &&...)
    -> detail_::enable_if_postable_executor_t_<SourceExecutor, RR, TargetExecutor, decltype(pf), Args...>;

  explicit basic_coroutine(promise_type &promise) noexcept
    : p_(&promise),
      handle_()
  {
    p_->acquire();
  }

  template<typename OtherExecutor>
  void set_executor(OtherExecutor &&executor)
  {
    CORIO_ASSERT(p_ != nullptr);
    CORIO_ASSERT(!handle_.valid());
    handle_ = p_->set_executor(std::forward<OtherExecutor>(executor));
  }

  executor_type get_executor() const
  {
    if (BOOST_UNLIKELY(p_ == nullptr)) /*[[unlikely]]*/ {
      CORIO_THROW<corio::invalid_coroutine_error>();
    }
    CORIO_ASSERT(handle_.valid());
    if (BOOST_UNLIKELY(handle_.destroyed())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::coroutine_already_destroyed_error>();
    }
    return p_->get_executor();
  }

  future_type get_future()
  {
    CORIO_ASSERT(p_ != nullptr);
    CORIO_ASSERT(handle_.valid());
    CORIO_ASSERT(!handle_.destroyed());
    CORIO_ASSERT(!handle_.done());
    return p_->get_future();
  }

  void resume()
  {
    CORIO_ASSERT(p_ != nullptr);
    CORIO_ASSERT(handle_.valid());
    CORIO_ASSERT(!handle_.destroyed());
    CORIO_ASSERT(!handle_.done());
    handle_.resume();
  }

public:
  basic_coroutine() noexcept
    : p_(),
      handle_()
  {}

  basic_coroutine(basic_coroutine const &) = delete;

  basic_coroutine(basic_coroutine &&rhs) noexcept
    : p_(rhs.p_),
      handle_(std::move(rhs.handle_))
  {
    rhs.p_ = nullptr;
  }

  ~basic_coroutine()
  {
    CORIO_ASSERT(p_ != nullptr || !handle_.valid());
    if (p_ != nullptr) {
      if (!handle_.valid()) {
        // Only the case when `set_executor` throws an exception. In this case,
        // the coroutine has not been registered with the service yet. So, it
        // should be destroyed manually.
        if (p_->release() == 0u) {
          // `*this` is the last remaining object referring to the coroutine
          // state.
          raw_handle_type_ raw_handle = raw_handle_type_::from_promise(*p_);
          raw_handle.destroy();
        }
        return;
      }
      if (handle_.destroyed()) {
        // The coroutine state has been already destroyed by the call of
        // `corio::thread_unsafe::coroutine_cleanup_service_::shutdown`.
        return;
      }
      if (p_->release() == 0u && handle_.done()) {
        // The coroutine is suspended at the final suspend point, and `*this`
        // is the last remaining object referring to the coroutine promise
        // object. So, `*this` is responsible to destroy the coroutine state.
        handle_.destroy();
      }
    }
  }

  void swap(basic_coroutine &rhs) noexcept
  {
    using std::swap;
    swap(p_, rhs.p_);
  }

  friend void swap(basic_coroutine &lhs, basic_coroutine &rhs) noexcept
  {
    lhs.swap(rhs);
  }

  basic_coroutine &operator=(basic_coroutine const &) = delete;

  basic_coroutine &operator=(basic_coroutine &&rhs) noexcept
  {
    basic_coroutine(std::move(rhs)).swap(*this);
    return *this;
  }

private:
  promise_type *p_;
  handle_type_ handle_;
}; // class basic_coroutine

template<typename R>
using coroutine = basic_coroutine<R, boost::asio::executor>;

} // namespace corio::thread_unsafe

#include <corio/thread_unsafe/coroutine_promise.hpp>

#endif // !defined(CORIO_THREAD_UNSAFE_COROUTINE_HPP_INCLUDE_GUARD)
