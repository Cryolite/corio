#include <corio/thread_unsafe/contextless/promise.hpp>

#include <corio/thread_unsafe/contextless/future.hpp>
#include <boost/asio/io_context.hpp>
#include <gtest/gtest.h>


namespace corio
{ using namespace thread_unsafe::contextless; }

TEST(contextless_promise_value, constructor)
{
  corio::promise<int> prom;
}

TEST(contextless_promise_value, swap)
{
  boost::asio::io_context ctx;

  corio::promise<int> p0;
  corio::future<int> f0 = p0.get_future();
  f0.set_context(ctx);

  corio::promise<int> p1;
  corio::future<int> f1 = p1.get_future();
  f1.set_context(ctx);

  using std::swap;
  swap(p0, p1);

  p0.set_value(1);
  p1.set_value(2);

  int i = 0, j = 0;
  f0.async_get(
    [&i](auto v) -> void{
      i = corio::get(v);
    });
  f1.async_get(
    [&j](auto v) -> void{
      j = corio::get(v);
    });
  EXPECT_EQ(i, 0);
  EXPECT_EQ(j, 0);

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_EQ(count, 2u);
  EXPECT_TRUE(ctx.stopped());
  EXPECT_EQ(i, 2);
  EXPECT_EQ(j, 1);
}

TEST(contextless_promise_value, get_future_after_get_future)
{
  corio::promise<int> prom;
  prom.get_future();
  EXPECT_THROW(prom.get_future();, corio::future_already_retrieved_error);
}

TEST(contextless_promise_value, set_value_before_async_get)
{
  boost::asio::io_context ctx;
  corio::promise<int> prom;
  corio::future<int> fut = prom.get_future();
  fut.set_context(ctx);

  prom.set_value(42);

  int i = 0;
  fut.async_get(
    [&i](auto v) -> void{
      i = corio::get(v);
    });

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());
  EXPECT_EQ(i, 42);
}

TEST(contextless_promise_value, set_value_after_async_get)
{
  boost::asio::io_context ctx;
  corio::promise<int> prom;
  corio::future<int> fut = prom.get_future();
  fut.set_context(ctx);

  int i = 0;
  fut.async_get(
    [&i](auto v) -> void{
      i = corio::get(v);
    });

  prom.set_value(42);

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());
  EXPECT_EQ(i, 42);
}

TEST(contextless_promise_value, set_value_before_set_context)
{
  boost::asio::io_context ctx;
  corio::promise<int> prom;
  corio::future<int> fut = prom.get_future();

  EXPECT_THROW(prom.set_value(42);, corio::no_context_error);
}

TEST(contextless_promise_value, set_value_after_set_value)
{
  boost::asio::io_context ctx;
  corio::promise<int> prom;
  corio::future<int> fut = prom.get_future();
  fut.set_context(ctx);

  prom.set_value(42);
  EXPECT_THROW(prom.set_value(43);, corio::promise_already_satisfied_error);
}

TEST(contextless_promise_value, set_exception_before_async_get)
{
  boost::asio::io_context ctx;
  corio::promise<int> prom;
  corio::future<int> fut = prom.get_future();
  fut.set_context(ctx);

  auto p = std::make_exception_ptr(std::runtime_error(""));
  prom.set_exception(std::move(p));

  int i = 0;
  fut.async_get(
    [&i](auto v) -> void{
      i = corio::get(v);
    });

  boost::asio::io_context::count_type count = 0u;
  EXPECT_THROW(count = ctx.run();, std::runtime_error);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
  EXPECT_EQ(i, 0);
}

TEST(contextless_promise_value, set_exception_after_async_get)
{
  boost::asio::io_context ctx;
  corio::promise<int> prom;
  corio::future<int> fut = prom.get_future();
  fut.set_context(ctx);

  fut.async_get(
    [](auto v) -> void{
      corio::get(v);
    });

  auto p = std::make_exception_ptr(std::runtime_error(""));
  prom.set_exception(std::move(p));

  EXPECT_THROW(ctx.run();, std::runtime_error);
  EXPECT_TRUE(ctx.stopped());
}

TEST(contextless_promise_value, destructor_without_set_before_async_get)
{
  boost::asio::io_context ctx;
  corio::future<int> fut;

  {
    corio::promise<int> prom;
    corio::future<int> fut2 = prom.get_future();
    fut2.set_context(ctx);
    fut = std::move(fut2);
  }

  fut.async_get(
    [](auto v) -> void{
      corio::get(v);
    });

  EXPECT_THROW(ctx.run();, corio::broken_promise_error);
  EXPECT_TRUE(ctx.stopped());
}
