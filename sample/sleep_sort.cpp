#include <corio/thread_unsafe/post.hpp>
#include <corio/thread_unsafe/resume.hpp>
#include <corio/thread_unsafe/coroutine.hpp>
#include <boost/asio/system_timer.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <iostream>


namespace corio
{ using namespace thread_unsafe; }

corio::coroutine<void> sleep_sort(int n)
{
  boost::asio::system_timer timer(co_await corio::this_executor);
  timer.expires_after(n * std::chrono::milliseconds(100));
  co_await timer.async_wait(corio::resume);
  std::cout << n << std::endl;
}

int main()
{
  using context = boost::asio::io_context;
  context ctx;

#if 0
  auto coro3 = sleep_sort(3);
  coro3.set_executor(ctx.get_executor());
  boost::asio::post([&coro3]() -> void{ coro3.resume(); });
  auto coro7 = sleep_sort(7);
  coro7.set_executor(ctx.get_executor());
  boost::asio::post([&coro7]() -> void{ coro7.resume(); });
  auto coro4 = sleep_sort(4);
  coro4.set_executor(ctx.get_executor());
  boost::asio::post([&coro4]() -> void{ coro4.resume(); });
  auto coro9 = sleep_sort(9);
  coro9.set_executor(ctx.get_executor());
  boost::asio::post([&coro9]() -> void{ coro9.resume(); });
  auto coro1 = sleep_sort(1);
  coro1.set_executor(ctx.get_executor());
  boost::asio::post([&coro1]() -> void{ coro1.resume(); });
  auto coro2 = sleep_sort(2);
  coro2.set_executor(ctx.get_executor());
  boost::asio::post([&coro2]() -> void{ coro2.resume(); });
  auto coro5 = sleep_sort(5);
  coro5.set_executor(ctx.get_executor());
  boost::asio::post([&coro5]() -> void{ coro5.resume(); });
  auto coro8 = sleep_sort(8);
  coro8.set_executor(ctx.get_executor());
  boost::asio::post([&coro8]() -> void{ coro8.resume(); });
  auto coro6 = sleep_sort(6);
  coro6.set_executor(ctx.get_executor());
  boost::asio::post([&coro6]() -> void{ coro6.resume(); });
  auto coro0 = sleep_sort(0);
  coro0.set_executor(ctx.get_executor());
  boost::asio::post([coro = std::move(coro0)]() mutable -> void{ coro.resume(); });
#endif

  {
    //auto work_guard = boost::asio::make_work_guard(ctx);
    //ctx.run();
  }
}
