#include <corio/thread_unsafe/coroutine.hpp>

#include <gtest/gtest.h>
#include <boost/asio/io_service.hpp>
#include <experimental/coroutine>


namespace corio
{ using namespace thread_unsafe; }

namespace{

int i = 0;

corio::coroutine<void> void_co_return()
{
  ++i;
  co_return;
}

} // namespace *unnamed*

TEST(coroutine, void_co_return)
{
  using context = boost::asio::io_context;
  context ctx;
  corio::coroutine<void> coro = void_co_return();
  coro.set_executor(ctx.get_executor());
#if 0
  EXPECT_EQ(i, 0);
  ASSERT_FALSE(coro.done());
  coro.resume();
  EXPECT_EQ(i, 1);
  EXPECT_TRUE(coro.done());

  corio::coroutine<void>::future_type future = std::move(coro).get_future();
  int j = 0;
  future.async_get(
    [&j](auto v) -> void {
      static_assert(std::is_void_v<decltype(corio::get(v))>);
      corio::get(v);
      j = 42;
    });
  EXPECT_EQ(j, 0);

  context::count_type count = ctx.run();
  EXPECT_EQ(j, 42);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());
#endif
}

#if 0

namespace{

corio::coroutine<void> void_co_await()
{
  ++i;
  co_await std::experimental::suspend_always{};
  ++i;
  co_return;
}

} // namespace *unnamed*

TEST(coroutine, void_co_await)
{
  i = 0;

  using context = boost::asio::io_context;
  context ctx;
  corio::coroutine<void> coro = void_co_await();
  coro.set_executor(ctx.get_executor());
  EXPECT_EQ(i, 0);
  ASSERT_FALSE(coro.done());

  coro.resume();
  EXPECT_EQ(i, 1);
  ASSERT_FALSE(coro.done());

  coro.resume();
  EXPECT_EQ(i, 2);
  ASSERT_TRUE(coro.done());

  corio::coroutine<void>::future_type future = std::move(coro).get_future();
  int j = 0;
  future.async_get(
    [&](auto v) -> void{
      static_assert(std::is_void_v<decltype(corio::get(v))>);
      corio::get(v);
      j = 42;
    });
  EXPECT_EQ(j, 0);

  context::count_type count = ctx.run();
  EXPECT_EQ(j, 42);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());
}

#endif
