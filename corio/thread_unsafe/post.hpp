#if !defined(CORIO_THREAD_UNSAFE_POST_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_POST_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/coroutine.hpp>
#include <corio/core/enable_if_executor.hpp>
#include <corio/core/enable_if_execution_context.hpp>
#include <corio/core/error.hpp>
#include <corio/util/throw.hpp>
#include <boost/asio/post.hpp>
#include <type_traits>


namespace corio::thread_unsafe{

namespace detail_{

template<typename T>
class coroutine_iteration_handler_;

template<typename R, typename Executor>
class coroutine_iteration_handler_<corio::thread_unsafe::basic_coroutine<R, Executor> >
{
public:
  using coroutine_type = corio::thread_unsafe::basic_coroutine<R, Executor>;

  explicit coroutine_iteration_handler_(coroutine_type &&coro)
    : coro_(std::move(coro))
  {}

  coroutine_iteration_handler_(coroutine_iteration_handler_ const &) = delete;

  coroutine_iteration_handler_(coroutine_iteration_handler_ &&rhs) = default;

  coroutine_iteration_handler_ &operator=(coroutine_iteration_handler_ const &) = delete;

  void operator()()
  {
    if (!coro_.done()) {
      coro_.resume();
      boost::asio::post(coro_.get_executor(), std::move(*this));
    }
  }

private:
  coroutine_type coro_;
}; // class coroutine_iteration_handler_<corio::thread_unsafe::basic_coroutine<R, Executor> >

} // namespace detail_

template<typename Executor0, typename R, typename Executor1>
void post(
  Executor0 const &executor,
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
  using coroutine_type = corio::thread_unsafe::basic_coroutine<R, Executor1>;
  using handler_type = detail_::coroutine_iteration_handler_<coroutine_type>;
  handler_type h(std::move(coro));
  boost::asio::post(executor, std::move(h));
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
