#include <corio/thread_unsafe/coroutine.hpp>

#include <corio/thread_unsafe/future.hpp>
#include <corio/core/error.hpp>
#include <gtest/gtest.h>
#include <boost/asio/io_service.hpp>
#include <memory>


namespace corio
{ using namespace thread_unsafe; }

namespace{

class coroutine_void_0
  : public ::testing::Test
{
protected:
  static corio::coroutine<void> f(bool &flag)
  {
    flag = true;
    co_return;
  }

  coroutine_void_0()
    : p_context(std::make_unique<boost::asio::io_context>()),
      flag(),
      future(corio::post(*p_context, f, flag)),
      status()
  {}

  std::unique_ptr<boost::asio::io_context> p_context;
  bool flag;
  corio::future<void> future;
  std::future_status status;
}; // class coroutine_void_0

} // namespace *unnamed*

TEST_F(coroutine_void_0, test)
{
  EXPECT_FALSE(flag);
}

TEST_F(coroutine_void_0, test_r)
{
  p_context->run();
  EXPECT_TRUE(flag);
}

TEST_F(coroutine_void_0, test_s)
{
  p_context.reset();
  EXPECT_FALSE(flag);
}

TEST_F(coroutine_void_0, test_g)
{
  status = std::future_status::ready;
  bool ready = false;
  future.async_get([&ready](corio::expected<void> result) -> void{ result.get(); ready = true; });
  EXPECT_FALSE(flag);
  EXPECT_FALSE(ready);
}

TEST_F(coroutine_void_0, test_rs)
{
  p_context->run();
  p_context.reset();
  EXPECT_TRUE(flag);
}

TEST_F(coroutine_void_0, test_rg)
{
  p_context->run();
  bool ready = false;
  future.async_get([&ready](corio::expected<void> result) -> void{ result.get(); ready = true; });
  EXPECT_TRUE(flag);
  EXPECT_FALSE(ready);
}

TEST_F(coroutine_void_0, test_gr)
{
  bool ready = false;
  future.async_get([&ready](corio::expected<void> result) -> void{ result.get(); ready = true; });
  p_context->run();
  EXPECT_TRUE(flag);
  EXPECT_TRUE(ready);
}

TEST_F(coroutine_void_0, test_gs)
{
  bool ready = false;
  future.async_get([&ready](corio::expected<void> result) -> void{ result.get(); ready = true; });
  p_context.reset();
  EXPECT_FALSE(flag);
  EXPECT_FALSE(ready);
}

TEST_F(coroutine_void_0, test_rgs)
{
  p_context->run();
  bool ready = false;
  future.async_get([&ready](corio::expected<void> result) -> void{ result.get(); ready = true; });
  p_context.reset();
  EXPECT_TRUE(flag);
  EXPECT_FALSE(ready);
}

TEST_F(coroutine_void_0, test_grs)
{
  bool ready = true;
  future.async_get([&ready](corio::expected<void> result) -> void{ result.get(); ready = true; });
  p_context->run();
  p_context.reset();
  EXPECT_TRUE(flag);
  EXPECT_TRUE(ready);
}

namespace{

class coroutine_void_1
  : public ::testing::Test
{
protected:
  static corio::coroutine<void> f(bool &flag0, bool &flag1)
  {
    flag0 = true;
    co_await std::experimental::suspend_always{};
    flag1 = true;
    co_return;
  }

  coroutine_void_1()
    : p_context(std::make_unique<boost::asio::io_context>()),
      flag0(),
      flag1(),
      future(corio::post(*p_context, f, flag0, flag1))
  {}

  std::unique_ptr<boost::asio::io_context> p_context;
  bool flag0;
  bool flag1;
  corio::future<void> future;
}; // class coroutine_void_1

} // namespace *unnamed*

TEST_F(coroutine_void_1, test)
{
  EXPECT_FALSE(flag0);
  EXPECT_FALSE(flag1);
}

TEST_F(coroutine_void_1, test_r)
{
  p_context->run();
  EXPECT_TRUE(flag0);
  EXPECT_FALSE(flag1);
}

TEST_F(coroutine_void_1, test_s)
{
  p_context.reset();
  EXPECT_FALSE(flag0);
  EXPECT_FALSE(flag1);
}

TEST_F(coroutine_void_1, test_rs)
{
  p_context->run();
  p_context.reset();
  EXPECT_TRUE(flag0);
  EXPECT_FALSE(flag1);
}

#if 0

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

#endif
