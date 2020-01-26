#if !defined(CORIO_THREAD_UNSAFE_POST_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_POST_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/coroutine.hpp>
#include <corio/core/enable_if_executor.hpp>
#include <corio/core/enable_if_execution_context.hpp>
#include <corio/core/error.hpp>
#include <corio/util/throw.hpp>
#include <boost/asio/post.hpp>
#include <utility>


namespace corio::thread_unsafe{

template<typename Executor0, typename R, typename Executor1>
void post(
  Executor0 &&executor,
  corio::thread_unsafe::basic_coroutine<R, Executor1> &&coro,
  corio::disable_if_execution_context_t<Executor0> * = nullptr,
  corio::enable_if_executor_t<Executor0> * = nullptr)
{
  if (coro.has_executor()) {
    if (coro.get_executor() != executor) {
      CORIO_THROW<corio::bad_executor_error>();
    }
  }
  else {
    coro.set_executor(executor);
  }
  boost::asio::post(std::move(executor), [coro_ = std::move(coro)]() mutable -> void { coro_.resume(); });
}

template<typename Executor0, typename R, typename Executor1>
void post(
  Executor0 const &executor,
  corio::thread_unsafe::basic_coroutine<R, Executor1> &&coro,
  corio::disable_if_execution_context_t<Executor0> * = nullptr,
  corio::enable_if_executor_t<Executor0> * = nullptr)
{
  corio::thread_unsafe::post(Executor0(executor), std::move(coro));
}

template<typename ExecutionContext, typename R, typename Executor>
void post(
  ExecutionContext &ctx,
  corio::thread_unsafe::basic_coroutine<R, Executor> &&coro,
  corio::enable_if_execution_context_t<ExecutionContext> * = nullptr,
  corio::disable_if_executor_t<ExecutionContext> * = nullptr)
{
  corio::thread_unsafe::post(ctx.get_executor(), std::move(coro));
}

} // namespace corio::thread_unsafe

#endif // !defined(CORIO_THREAD_UNSAFE_POST_HPP_INCLUDE_GUARD)
