#include <corio/thread_unsafe/coroutine.hpp>

#include <corio/thread_unsafe/post.hpp>
#include <corio/thread_unsafe/resume.hpp>
#include <gtest/gtest.h>
#include <boost/asio/system_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/system/system_error.hpp>
#include <chrono>
#include <experimental/coroutine>


namespace corio
{ using namespace thread_unsafe; }

namespace{

int i = 0;

corio::coroutine<void> f()
{
  i *= 2;
  co_return;
}

} // namespace *unnamed*

TEST(coroutine, destruct)
{
  i = 42;
  corio::coroutine<void> coro = f();
  EXPECT_EQ(i, 42);
  EXPECT_FALSE(coro.done());
}

TEST(coroutine, done_then_destruct)
{
  i = 42;
  corio::coroutine<void> coro = f();
  EXPECT_EQ(i, 42);
  ASSERT_FALSE(coro.done());
  coro.resume();
  EXPECT_EQ(i, 84);
  EXPECT_TRUE(coro.done());
}

TEST(coroutine, transfer_then_destruct_then_shutdown)
{
  i = 42;
  {
    boost::asio::io_context ctx;
    {
      corio::coroutine<void> coro = f();
      EXPECT_EQ(i, 42);
      EXPECT_FALSE(coro.done());
      coro.set_executor(ctx.get_executor());
      EXPECT_EQ(i, 42);
      EXPECT_FALSE(coro.done());
    }
    EXPECT_EQ(i, 42);
  }
  EXPECT_EQ(i, 42);
}

TEST(coroutine, transfer_then_done_then_destruct_then_shutdown)
{
  i = 42;
  {
    boost::asio::io_context ctx;
    {
      corio::coroutine<void> coro = f();
      EXPECT_EQ(i, 42);
      ASSERT_FALSE(coro.done());
      coro.set_executor(ctx.get_executor());
      EXPECT_EQ(i, 42);
      ASSERT_FALSE(coro.done());
      coro.resume();
      EXPECT_EQ(i, 84);
      EXPECT_TRUE(coro.done());
    }
    EXPECT_EQ(i, 84);
  }
  EXPECT_EQ(i, 84);
}

TEST(coroutine, done_then_transfer_then_destruct_then_shutdown)
{
  i = 42;
  {
    boost::asio::io_context ctx;
    {
      corio::coroutine<void> coro = f();
      EXPECT_EQ(i, 42);
      ASSERT_FALSE(coro.done());
      coro.resume();
      EXPECT_EQ(i, 84);
      EXPECT_TRUE(coro.done());
      coro.set_executor(ctx.get_executor());
      EXPECT_EQ(i, 84);
      EXPECT_TRUE(coro.done());
    }
    EXPECT_EQ(i, 84);
  }
  EXPECT_EQ(i, 84);
}

namespace{

bool timer_cancel_throw_ = false;

corio::coroutine<void> async_wait_canceled_timer(boost::asio::system_timer &timer)
{
  try {
    static_assert(std::is_same_v<decltype(timer.async_wait(corio::resume)), corio::future<void> >);
    co_await timer.async_wait(corio::resume);
  }
  catch (boost::system::system_error const &e) {
    if (e.code().value() == boost::system::errc::operation_canceled) {
      timer_cancel_throw_ = true;
    }
  }
  co_return;
}

corio::coroutine<void> cancel_timer(boost::asio::system_timer &timer)
{
  timer.cancel();
  co_return;
}

} // namespace *unnamed*

TEST(coroutine, timer_cancel_throw)
{
  timer_cancel_throw_ = false;
  boost::asio::io_context ctx;
  boost::asio::system_timer timer(ctx);
  timer.expires_after(std::chrono::seconds(1));
  corio::post(ctx, async_wait_canceled_timer(timer));
  corio::post(ctx, cancel_timer(timer));
  ctx.run();
  EXPECT_TRUE(timer_cancel_throw_);
}

namespace{

bool timer_cancel_nothrow_ = false;

corio::coroutine<void> async_wait_canceled_timer_nothrow(boost::asio::system_timer &timer)
{
  static_assert(
    std::is_same_v<decltype(timer.async_wait(corio::resume(std::nothrow))),
                   corio::future<boost::system::error_code> >);
  boost::system::error_code ec = co_await timer.async_wait(corio::resume(std::nothrow));
  if (ec.value() == boost::system::errc::operation_canceled) {
    timer_cancel_nothrow_ = true;
  }
  co_return;
}

} // namespace *unnamed*

TEST(coroutine, timer_cancel_nothrow)
{
  timer_cancel_nothrow_ = false;
  boost::asio::io_context ctx;
  boost::asio::system_timer timer(ctx);
  timer.expires_after(std::chrono::seconds(1));
  corio::post(ctx, async_wait_canceled_timer_nothrow(timer));
  corio::post(ctx, cancel_timer(timer));
  ctx.run();
  EXPECT_TRUE(timer_cancel_throw_);
}
