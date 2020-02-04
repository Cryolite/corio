#include <corio/thread_unsafe/post.hpp>
#include <corio/thread_unsafe/coroutine.hpp>
#include <boost/asio/system_timer.hpp>
#include <boost/asio/executor.hpp>
#include <boost/asio/io_context.hpp>
#include <thread>
#include <iostream>
#include <chrono>


namespace corio
{ using namespace thread_unsafe; }

namespace{

double get_time_in_seconds()
{
  using clock_type = std::chrono::system_clock;
  clock_type::rep count = clock_type::now().time_since_epoch().count();
  return static_cast<double>(count) * clock_type::period::num / clock_type::period::den;
}

}

corio::coroutine<void> sleep_sort(int n)
{
  boost::asio::system_timer timer(CORIO_THIS_EXECUTOR());
  timer.expires_after(n * std::chrono::milliseconds(100));
  co_await timer.async_wait(CORIO_USE_FUTURE());
  std::cout << n << " (thread id: " << std::this_thread::get_id()
            << ", time: " << get_time_in_seconds() << "s)" << std::endl;
}

int main()
{
  std::cout << std::setprecision(15);
  std::cout << "main (thread id: " << std::this_thread::get_id()
            << ", time: " << get_time_in_seconds() << "s)" << std::endl;

  using context = boost::asio::io_context;
  context ctx;

  corio::post(ctx, sleep_sort, 3);
  corio::post(ctx, sleep_sort, 7);
  corio::post(ctx, sleep_sort, 4);
  corio::post(ctx, sleep_sort, 9);
  corio::post(ctx, sleep_sort, 1);
  corio::post(ctx, sleep_sort, 2);
  corio::post(ctx, sleep_sort, 5);
  corio::post(ctx, sleep_sort, 8);
  corio::post(ctx, sleep_sort, 6);
  corio::post(ctx, sleep_sort, 0);

  ctx.run();
}
