#include <corio/thread_unsafe/coroutine.hpp>

#include <corio/core/error.hpp>
#include <gtest/gtest.h>
#include <boost/asio/io_service.hpp>
#include <boost/asio/executor.hpp>


namespace corio
{ using namespace thread_unsafe; }

namespace{

corio::coroutine<void> f(boost::asio::executor const &, int &i)
{
  i = 42;
  co_return;
}

class coroutine_ctor
  : public ::testing::Test
{
protected:
  using context_type = boost::asio::io_context;

  coroutine_ctor()
    : context(),
      i(),
      coro(f(context.get_executor(), i))
  {}

  context_type context;
  int i;
  corio::coroutine<void> coro;
}; // class coroutine_ctor

} // namespace *unnamed*

TEST_F(coroutine_ctor, get_executor)
{
  EXPECT_EQ(coro.get_executor(), context.get_executor());
}

TEST_F(coroutine_ctor, valid)
{
  EXPECT_TRUE(coro.valid());
}

TEST_F(coroutine_ctor, resume)
{
  coro.resume();
  EXPECT_EQ(i, 42);
}

TEST_F(coroutine_ctor, done)
{
  EXPECT_FALSE(coro.done());
}

namespace{

class coroutine_done
  : public ::testing::Test
{
protected:
  using context_type = boost::asio::io_context;

  coroutine_done()
    : context(),
      i(),
      coro(f(context.get_executor(), i))
  {
    coro.resume();
  }

  context_type context;
  int i;
  corio::coroutine<void> coro;
}; // class coroutine_done

} // namespace *unnamed*

TEST_F(coroutine_done, get_executor)
{
  EXPECT_EQ(coro.get_executor(), context.get_executor());
}

TEST_F(coroutine_done, valid)
{
  EXPECT_TRUE(coro.valid());
}

TEST_F(coroutine_done, resume)
{
  EXPECT_THROW(coro.resume();, corio::coroutine_already_done_error);
}

TEST_F(coroutine_done, done)
{
  EXPECT_TRUE(coro.done());
}

namespace{

class coroutine_dtor
  : public ::testing::Test
{
protected:
  using context_type = boost::asio::io_context;

  coroutine_dtor()
    : context(),
      i(),
      coro(f(context.get_executor(), i))
  {
    coro = corio::coroutine<void>();
  }

  context_type context;
  int i;
  corio::coroutine<void> coro;
}; // class coroutine_dtor

} // namespace *unnamed*

TEST_F(coroutine_dtor, get_executor)
{
  EXPECT_THROW(coro.get_executor();, corio::invalid_coroutine_error);
}

TEST_F(coroutine_dtor, valid)
{
  EXPECT_FALSE(coro.valid());
}

TEST_F(coroutine_dtor, resume)
{
  EXPECT_THROW(coro.resume();, corio::invalid_coroutine_error);
}

TEST_F(coroutine_dtor, done)
{
  EXPECT_THROW(coro.done();, corio::invalid_coroutine_error);
}

namespace{

class coroutine_done_dtor
  : public ::testing::Test
{
protected:
  using context_type = boost::asio::io_context;

  coroutine_done_dtor()
    : context(),
      i(),
      coro(f(context.get_executor(), i))
  {
    coro.resume();
    coro = corio::coroutine<void>();
  }

  context_type context;
  int i;
  corio::coroutine<void> coro;
}; // class coroutine_done_dtor

} // namespace *unnamed*

TEST_F(coroutine_done_dtor, get_executor)
{
  EXPECT_THROW(coro.get_executor();, corio::invalid_coroutine_error);
}

TEST_F(coroutine_done_dtor, valid)
{
  EXPECT_FALSE(coro.valid());
}

TEST_F(coroutine_done_dtor, resume)
{
  EXPECT_THROW(coro.resume();, corio::invalid_coroutine_error);
}

TEST_F(coroutine_done_dtor, done)
{
  EXPECT_THROW(coro.done();, corio::invalid_coroutine_error);
}
