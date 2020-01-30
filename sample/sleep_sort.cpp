#include <corio/thread_unsafe/post.hpp>
#include <corio/thread_unsafe/resume.hpp>
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

corio::coroutine<void> sleep_sort(boost::asio::executor const &, int n)
{
  boost::asio::system_timer timer(co_await corio::this_executor);
  timer.expires_after(n * std::chrono::milliseconds(100));
  co_await timer.async_wait(corio::resume(co_await corio::this_executor));
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

  corio::post(sleep_sort(ctx.get_executor(), 3));
  corio::post(sleep_sort(ctx.get_executor(), 7));
  corio::post(sleep_sort(ctx.get_executor(), 4));
  corio::post(sleep_sort(ctx.get_executor(), 9));
  corio::post(sleep_sort(ctx.get_executor(), 1));
  corio::post(sleep_sort(ctx.get_executor(), 2));
  corio::post(sleep_sort(ctx.get_executor(), 5));
  corio::post(sleep_sort(ctx.get_executor(), 8));
  corio::post(sleep_sort(ctx.get_executor(), 6));
  corio::post(sleep_sort(ctx.get_executor(), 0));

  ctx.run();
}
