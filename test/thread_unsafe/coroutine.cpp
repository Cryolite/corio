#include <corio/thread_unsafe/coroutine.hpp>

#include <corio/core/get.hpp>
#include <gtest/gtest.h>
#include <boost/asio/io_service.hpp>
#include <experimental/coroutine>


namespace corio
{ using namespace thread_unsafe; }

namespace{

int i = 0;

corio::basic_coroutine<void, boost::asio::io_service::executor_type>
  void_co_return(boost::asio::io_service &)
{
  ++i;
  co_return;
}

} // namespace *unnamed*

TEST(coroutine, void_co_return)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using executor_type = context_type::executor_type;
  using coroutine = corio::basic_coroutine<void, executor_type>;
  coroutine coro = void_co_return(ctx);
  EXPECT_EQ(i, 0);
  ASSERT_FALSE(coro.done());
  coro.resume();
  EXPECT_EQ(i, 1);
  EXPECT_TRUE(coro.done());

  int j = 0;
  coro.async_get(
    [&j](auto v) -> void {
      static_assert(std::is_void_v<decltype(corio::get(v))>);
      corio::get(v);
      j = 42;
    });
  EXPECT_EQ(j, 0);

  context_type::count_type count = ctx.run();
  EXPECT_EQ(j, 42);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());
}

namespace{

corio::basic_coroutine<void, boost::asio::io_service::executor_type>
  void_co_await(boost::asio::io_service &)
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

  using context_type = boost::asio::io_context;
  context_type ctx;
  using executor_type = context_type::executor_type;
  using coroutine = corio::basic_coroutine<void, executor_type>;
  coroutine coro = void_co_await(ctx);
  EXPECT_EQ(i, 0);
  ASSERT_FALSE(coro.done());

  coro.resume();
  EXPECT_EQ(i, 1);
  ASSERT_FALSE(coro.done());

  coro.resume();
  EXPECT_EQ(i, 2);
  ASSERT_TRUE(coro.done());

  int j = 0;
  coro.async_get(
    [&](auto v) -> void{
      static_assert(std::is_void_v<decltype(corio::get(v))>);
      corio::get(v);
      j = 42;
    });
  EXPECT_EQ(j, 0);

  context_type::count_type count = ctx.run();
  EXPECT_EQ(j, 42);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());
}
