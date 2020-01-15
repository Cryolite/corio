#include <corio/thread_unsafe/mutex.hpp>

#include <gtest/gtest.h>
#include <boost/asio/io_context.hpp>


namespace corio
{ using namespace thread_unsafe; }

TEST(mutex, construct)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  corio::mutex<context_type> mtx(ctx);

  boost::asio::io_context::count_type count = ctx.poll();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(mutex, lock_unlock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  corio::mutex<context_type> mtx(ctx);

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

TEST(mutex, lock_after_lock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  corio::mutex<context_type> mtx(ctx);

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

TEST(mutex, two_objects)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  corio::mutex<context_type> mtxi(ctx);
  corio::mutex<context_type> mtxj(ctx);

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

TEST(mutex, try_lock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  corio::mutex<context_type> mtx(ctx);

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
