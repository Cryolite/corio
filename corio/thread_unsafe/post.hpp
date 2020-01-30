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

template<typename R, typename Executor>
void post(corio::thread_unsafe::basic_coroutine<R, Executor> &&coro)
{
  boost::asio::post(coro.get_executor(), [coro_ = std::move(coro)]() mutable -> void { coro_.resume(); });
}

} // namespace corio::thread_unsafe

#endif // !defined(CORIO_THREAD_UNSAFE_POST_HPP_INCLUDE_GUARD)
