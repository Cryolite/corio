#include <corio/thread_unsafe/mutex.hpp>

#include <gtest/gtest.h>
#include <boost/asio/io_context.hpp>
#include <type_traits>


namespace corio
{ using namespace thread_unsafe; }

TEST(mutex, construct)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using mutex = corio::basic_mutex<context_type>;
  static_assert(!std::is_default_constructible_v<mutex>);
  mutex mtx(ctx);

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(mutex, lock_then_unlock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using mutex = corio::basic_mutex<context_type>;
  mutex mtx(ctx);

  int i = 0;

  mtx.async_lock([&i]() -> void{ ++i; });
  EXPECT_EQ(i, 0);
  EXPECT_TRUE(!ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  mtx.unlock();
  EXPECT_EQ(i, 1);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(mutex, lock_then_lock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using mutex = corio::basic_mutex<context_type>;
  mutex mtx(ctx);

  int i = 0;

  mtx.async_lock([&i]{ ++i; });
  EXPECT_EQ(i, 0);
  EXPECT_TRUE(!ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  mtx.async_lock([&i]{ ++i; });
  EXPECT_EQ(i, 1);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  mtx.unlock();
  EXPECT_EQ(i, 1);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_EQ(i, 2);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  mtx.unlock();
  EXPECT_EQ(i, 2);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_EQ(i, 2);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(mutex, lock_then_try_lock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using mutex = corio::basic_mutex<context_type>;
  mutex mtx(ctx);

  int i = 0;

  mtx.async_lock([&](){ ++i; });
  EXPECT_EQ(i, 0);
  EXPECT_FALSE(ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  EXPECT_FALSE(mtx.try_lock());
  EXPECT_EQ(i, 1);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  mtx.unlock();
  EXPECT_EQ(i, 1);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(mutex, try_lock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using mutex = corio::basic_mutex<context_type>;
  mutex mtx(ctx);

  EXPECT_TRUE(mtx.try_lock());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  mtx.unlock();
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(mutex, try_lock_then_lock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using mutex = corio::basic_mutex<context_type>;
  mutex mtx(ctx);

  EXPECT_TRUE(mtx.try_lock());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  int i = 0;

  mtx.async_lock([&](){ ++i; });
  EXPECT_EQ(i, 0);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_EQ(i, 0);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  mtx.unlock();
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  mtx.unlock();
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(mutex, try_lock_then_try_lock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using mutex = corio::basic_mutex<context_type>;
  mutex mtx(ctx);

  EXPECT_TRUE(mtx.try_lock());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  EXPECT_FALSE(mtx.try_lock());

  count = ctx.run();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  mtx.unlock();
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(mutex_deferred_context, construct)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using mutex = corio::basic_mutex<void>;
  mutex mtx;

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(mutex_deferred_context, lock_then_unlock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using mutex = corio::basic_mutex<void>;
  mutex mtx;

  mtx.set_context(ctx);

  int i = 0;

  mtx.async_lock([&i]() -> void{ ++i; });
  EXPECT_EQ(i, 0);
  EXPECT_TRUE(!ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  mtx.unlock();
  EXPECT_EQ(i, 1);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(mutex_deferred_context, lock_then_lock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using mutex = corio::basic_mutex<void>;
  mutex mtx;

  mtx.set_context(ctx);

  int i = 0;

  mtx.async_lock([&i]{ ++i; });
  EXPECT_EQ(i, 0);
  EXPECT_TRUE(!ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  mtx.async_lock([&i]{ ++i; });
  EXPECT_EQ(i, 1);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  mtx.unlock();
  EXPECT_EQ(i, 1);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_EQ(i, 2);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  mtx.unlock();
  EXPECT_EQ(i, 2);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_EQ(i, 2);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(mutex_deferred_context, lock_then_try_lock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using mutex = corio::basic_mutex<void>;
  mutex mtx;

  mtx.set_context(ctx);

  int i = 0;

  mtx.async_lock([&](){ ++i; });
  EXPECT_EQ(i, 0);
  EXPECT_FALSE(ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  EXPECT_FALSE(mtx.try_lock());
  EXPECT_EQ(i, 1);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  mtx.unlock();
  EXPECT_EQ(i, 1);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(mutex_deferred_context, try_lock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using mutex = corio::basic_mutex<void>;
  mutex mtx;

  mtx.set_context(ctx);

  EXPECT_TRUE(mtx.try_lock());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  mtx.unlock();
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(mutex_deferred_context, try_lock_then_lock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using mutex = corio::basic_mutex<void>;
  mutex mtx;

  mtx.set_context(ctx);

  EXPECT_TRUE(mtx.try_lock());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  int i = 0;

  mtx.async_lock([&](){ ++i; });
  EXPECT_EQ(i, 0);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_EQ(i, 0);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  mtx.unlock();
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  mtx.unlock();
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(mutex_deferred_context, try_lock_then_try_lock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using mutex = corio::basic_mutex<void>;
  mutex mtx;

  mtx.set_context(ctx);

  EXPECT_TRUE(mtx.try_lock());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  EXPECT_FALSE(mtx.try_lock());

  count = ctx.run();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  mtx.unlock();
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}
