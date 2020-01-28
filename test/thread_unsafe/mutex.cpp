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
  using executor_type = context_type::executor_type;
  using mutex = corio::basic_mutex<executor_type>;
  mutex mtx(ctx);

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(mutex, lock_then_unlock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using executor_type = context_type::executor_type;
  using mutex = corio::basic_mutex<executor_type>;
  mutex mtx(ctx);

  mutex::lock_type lock;
  int i = 0;

  mtx.async_lock([&lock, &i](auto l) -> void{ lock = std::move(l); ++i; });
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_EQ(i, 0);
  EXPECT_TRUE(!ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_TRUE(lock.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  lock = mutex::lock_type();
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(mutex, lock_then_lock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using executor_type = context_type::executor_type;
  using mutex = corio::basic_mutex<executor_type>;
  mutex mtx(ctx);

  mutex::lock_type lock;
  int i = 0;

  mtx.async_lock([&lock, &i](auto l) -> void{ lock = std::move(l); ++i; });
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_EQ(i, 0);
  EXPECT_TRUE(!ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_TRUE(lock.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  mutex::lock_type lock2;
  mtx.async_lock([&lock2, &i](auto l) -> void{ lock2 = std::move(l); ++i; });
  EXPECT_TRUE(lock.owns_lock());
  EXPECT_FALSE(lock2.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_TRUE(lock.owns_lock());
  EXPECT_FALSE(lock2.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  lock = mutex::lock_type();
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_FALSE(lock2.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_TRUE(lock2.owns_lock());
  EXPECT_EQ(i, 2);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  lock2 = mutex::lock_type();
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_FALSE(lock2.owns_lock());
  EXPECT_EQ(i, 2);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_FALSE(lock2.owns_lock());
  EXPECT_EQ(i, 2);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(mutex, lock_then_try_lock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using executor_type = context_type::executor_type;
  using mutex = corio::basic_mutex<executor_type>;
  mutex mtx(ctx);

  mutex::lock_type lock;
  int i = 0;

  mtx.async_lock([&lock, &i](auto l) -> void{ lock = std::move(l); ++i; });
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_EQ(i, 0);
  EXPECT_FALSE(ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_TRUE(lock.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  EXPECT_FALSE(mtx.try_lock());
  EXPECT_TRUE(lock.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_TRUE(lock.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  lock = mutex::lock_type();
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(mutex, try_lock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using executor_type = context_type::executor_type;
  using mutex = corio::basic_mutex<executor_type>;
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
  using executor_type = context_type::executor_type;
  using mutex = corio::basic_mutex<executor_type>;
  mutex mtx(ctx);

  EXPECT_TRUE(mtx.try_lock());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  mutex::lock_type lock;
  int i = 0;

  mtx.async_lock([&lock, &i](auto l) -> void{ lock = std::move(l); ++i; });
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_EQ(i, 0);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_EQ(i, 0);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  mtx.unlock();
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_TRUE(lock.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  lock = mutex::lock_type();
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(mutex, try_lock_then_try_lock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using executor_type = context_type::executor_type;
  using mutex = corio::basic_mutex<executor_type>;
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

TEST(mutex_te_dca, construct)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using mutex = corio::mutex;
  mutex mtx(ctx);

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(mutex_te_dca, lock_then_unlock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using mutex = corio::mutex;
  mutex mtx(ctx);

  mutex::lock_type lock;
  int i = 0;

  mtx.async_lock([&lock, &i](auto l) -> void{ lock = std::move(l); ++i; });
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_EQ(i, 0);
  EXPECT_TRUE(!ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_TRUE(lock.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  lock = mutex::lock_type();
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(mutex_te_dca, lock_then_lock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using mutex = corio::mutex;
  mutex mtx(ctx);

  mutex::lock_type lock;
  int i = 0;

  mtx.async_lock([&lock, &i](auto l) -> void{ lock = std::move(l); ++i; });
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_EQ(i, 0);
  EXPECT_TRUE(!ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_TRUE(lock.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  mutex::lock_type lock2;

  mtx.async_lock([&lock2, &i](auto l) -> void{ lock2 = std::move(l); ++i; });
  EXPECT_TRUE(lock.owns_lock());
  EXPECT_FALSE(lock2.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_TRUE(lock.owns_lock());
  EXPECT_FALSE(lock2.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  lock = mutex::lock_type();
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_FALSE(lock2.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_TRUE(lock2.owns_lock());
  EXPECT_EQ(i, 2);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  lock2 = mutex::lock_type();
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_FALSE(lock2.owns_lock());
  EXPECT_EQ(i, 2);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_FALSE(lock2.owns_lock());
  EXPECT_EQ(i, 2);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(mutex_te_dca, lock_then_try_lock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using mutex = corio::mutex;
  mutex mtx(ctx);

  mutex::lock_type lock;
  int i = 0;

  mtx.async_lock([&lock, &i](auto l) -> void{ lock = std::move(l); ++i; });
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_EQ(i, 0);
  EXPECT_FALSE(ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_TRUE(lock.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  EXPECT_FALSE(mtx.try_lock());
  EXPECT_TRUE(lock.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_TRUE(lock.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  lock = mutex::lock_type();
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(mutex_te_dca, try_lock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using mutex = corio::mutex;
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

TEST(mutex_te_dca, try_lock_then_lock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using mutex = corio::mutex;
  mutex mtx(ctx);

  EXPECT_TRUE(mtx.try_lock());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  mutex::lock_type lock;
  int i = 0;

  mtx.async_lock([&lock, &i](auto l){ lock = std::move(l); ++i; });
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_EQ(i, 0);
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_EQ(i, 0);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  mtx.unlock();
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_TRUE(lock.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 1u);
  EXPECT_TRUE(ctx.stopped());

  ctx.restart();
  EXPECT_FALSE(ctx.stopped());

  lock = mutex::lock_type();
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_FALSE(ctx.stopped());

  count = ctx.run();
  EXPECT_FALSE(lock.owns_lock());
  EXPECT_EQ(i, 1);
  EXPECT_EQ(count, 0u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(mutex_te_dca, try_lock_then_try_lock)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using mutex = corio::mutex;
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
