#include <corio/thread_unsafe/post.hpp>
#include <corio/thread_unsafe/resume.hpp>
#include <corio/thread_unsafe/coroutine.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <iostream>


namespace corio
{ using namespace thread_unsafe; }

corio::coroutine<void> sleep_sort(int n)
{
  boost::asio::deadline_timer timer(co_await corio::this_executor);
  timer.expires_from_now(boost::posix_time::milliseconds(n * 100));
  co_await timer.async_wait(corio::resume);
  std::cout << n << std::endl;
}

int main()
{
  using context = boost::asio::io_context;
  context ctx;
  corio::post(ctx, sleep_sort(9));
  corio::post(ctx, sleep_sort(8));
  {
    //auto word_guard = boost::asio::make_work_guard(ctx);
    ctx.run();
  }
}
