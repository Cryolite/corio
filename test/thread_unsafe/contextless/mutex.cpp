#include <corio/thread_unsafe/contextless/mutex.hpp>

#include <gtest/gtest.h>
#include <boost/asio/io_context.hpp>


namespace corio
{ using namespace thread_unsafe::contextless; }

TEST(contextless_mutex, construct)
{
  corio::mutex mtx;

  using context_type = boost::asio::io_context;
  context_type ctx;
  mtx.set_context(ctx);

  boost::asio::io_context::count_type count = ctx.poll();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(contextless_mutex, lock_unlock)
{
  corio::mutex mtx;

  using context_type = boost::asio::io_context;
  context_type ctx;
  mtx.set_context(ctx);

  int i = 0;

  mtx.async_lock([&]{ ++i; });
  EXPECT_EQ(i, 0);
  EXPECT_TRUE(!ctx.stopped());

  boost::asio::io_context::count_type count = ctx.poll();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());

  mtx.unlock();
  EXPECT_EQ(i, 1);

  ctx.restart();
  count = ctx.poll();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(contextless_mutex, lock_after_lock)
{
  corio::mutex mtx;

  using context_type = boost::asio::io_context;
  context_type ctx;
  mtx.set_context(ctx);

  int i = 0;

  mtx.async_lock([&]{ ++i; });
  EXPECT_EQ(i, 0);
  EXPECT_TRUE(!ctx.stopped());

  boost::asio::io_context::count_type count = ctx.poll();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());

  mtx.async_lock([&]{ ++i; });
  EXPECT_EQ(i, 1);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_TRUE(!ctx.stopped());

  count = ctx.poll();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  mtx.unlock();
  EXPECT_EQ(i, 1);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_TRUE(!ctx.stopped());

  count = ctx.poll();
  EXPECT_EQ(i, 2);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());

  mtx.unlock();
  EXPECT_EQ(i, 2);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_TRUE(!ctx.stopped());

  count = ctx.poll();
  EXPECT_EQ(i, 2);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(contextless_mutex, two_objects)
{
  corio::mutex mtxi;
  corio::mutex mtxj;

  using context_type = boost::asio::io_context;
  context_type ctx;
  mtxi.set_context(ctx);
  mtxj.set_context(ctx);

  int i = 0;
  int j = 0;

  mtxi.async_lock([&](){ ++i; });
  EXPECT_EQ(i, 0);
  EXPECT_TRUE(!ctx.stopped());

  mtxj.async_lock([&](){ ++j; });
  EXPECT_EQ(i, 0);
  EXPECT_TRUE(!ctx.stopped());

  boost::asio::io_context::count_type count = ctx.poll();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(j, 1);
  EXPECT_EQ(count, 2u);
  EXPECT_TRUE(ctx.stopped());

  mtxi.unlock();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(j, 1);

  ctx.restart();
  count = ctx.poll();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(j, 1);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  mtxj.unlock();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(j, 1);

  ctx.restart();
  count = ctx.poll();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(j, 1);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(contextless_mutex, try_lock)
{
  corio::mutex mtx;

  using context_type = boost::asio::io_context;
  context_type ctx;
  mtx.set_context(ctx);

  EXPECT_TRUE(mtx.try_lock());

  boost::asio::io_context::count_type count = ctx.poll();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  EXPECT_FALSE(mtx.try_lock());

  count = ctx.poll();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  mtx.unlock();

  count = ctx.poll();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  EXPECT_TRUE(mtx.try_lock());

  count = ctx.poll();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  mtx.unlock();

  count = ctx.poll();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}
