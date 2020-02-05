#if !defined(CORIO_THREAD_UNSAFE_COROUTINE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_COROUTINE_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/detail_/coroutine_cleanup_service_.hpp>
#include <corio/thread_unsafe/future.hpp>
#include <corio/core/is_execution_context.hpp>
#include <corio/core/is_executor.hpp>
#include <corio/core/error.hpp>
#include <corio/util/throw.hpp>
#include <corio/util/assert.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/executor.hpp>
#include <boost/config.hpp>
#include <type_traits>
#include <functional>
#include <utility>
#include <experimental/coroutine>


namespace corio::thread_unsafe{

template<typename R, typename Executor>
class basic_coroutine;

namespace detail_{

template<typename Executor, typename T>
struct enable_if_postable_executor_impl1_
{};

template<typename SourceExecutor, typename R, typename TargetExecutor>
struct enable_if_postable_executor_impl1_<SourceExecutor, corio::thread_unsafe::basic_coroutine<R, TargetExecutor> >
  : public std::enable_if<
      corio::is_executor_v<TargetExecutor> && std::is_constructible_v<TargetExecutor, SourceExecutor>,
      corio::thread_unsafe::basic_future<R, TargetExecutor> >
{};

template<typename Executor, typename F, typename... Args>
struct enable_if_postable_executor_impl0_
  : public enable_if_postable_executor_impl1_<Executor, std::invoke_result_t<F, Args...> >
{};

template<typename Executor, typename F, typename... Args>
struct enable_if_postable_executor_
  : public std::conditional_t<
      corio::is_executor_v<std::decay_t<Executor> > && std::is_invocable_v<F, Args...>,
      enable_if_postable_executor_impl0_<Executor, F, Args...>,
      std::enable_if<false> >
{};

template<typename Executor, typename F, typename... Args>
using enable_if_postable_executor_t_ = typename enable_if_postable_executor_<Executor, F, Args...>::type;

template<typename ExecutionContext, typename F, typename... Args>
struct enable_if_postable_execution_context_impl_
  : public enable_if_postable_executor_<typename ExecutionContext::executor_type, F, Args...>
{};

template<typename ExecutionContext, typename F, typename... Args>
struct enable_if_postable_execution_context_
  : public std::conditional_t<
      corio::is_execution_context_v<ExecutionContext>,
      enable_if_postable_execution_context_impl_<ExecutionContext, F, Args...>,
      std::enable_if<false> >
{};

template<typename ExecutionContext, typename F, typename... Args>
using enable_if_postable_execution_context_t_ = typename enable_if_postable_execution_context_< ExecutionContext, F, Args...>::type;

} // namespace detail_

template<typename Executor, typename F, typename... Args>
detail_::enable_if_postable_executor_t_<Executor, F, Args...> post(Executor &&executor, F &&f, Args &&... args)
{
  auto coro = std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
  coro.set_executor(std::forward<Executor>(executor));
  auto future = coro.get_future();
  auto e = coro.get_executor();
  boost::asio::post(std::move(e), [coro_ = std::move(coro)]() mutable -> void{ coro_.resume(); });
  return future;
}

template<typename ExecutionContext, typename F, typename... Args>
detail_::enable_if_postable_execution_context_t_<ExecutionContext, F, Args...>
post(ExecutionContext &context, F &&f, Args &&... args)
{
  return corio::thread_unsafe::post(context.get_executor(), std::forward<F>(f), std::forward<Args>(args)...);
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

  template<typename OtherExecutor, typename F, typename... Args>
  friend detail_::enable_if_postable_executor_t_<OtherExecutor, F, Args...>
  corio::thread_unsafe::post(OtherExecutor &&, F &&, Args &&...);

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
