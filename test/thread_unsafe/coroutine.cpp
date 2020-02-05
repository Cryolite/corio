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
      future(corio::post(*p_context, f, flag))
  {}

  std::unique_ptr<boost::asio::io_context> p_context;
  bool flag;
  corio::future<void> future;
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

TEST_F(coroutine_void_1, test_g)
{
  bool ready = false;
  future.async_get([&ready](corio::expected<void> result) -> void{ result.get(); ready = true; });
  EXPECT_FALSE(flag0);
  EXPECT_FALSE(flag1);
  EXPECT_FALSE(ready);
}

TEST_F(coroutine_void_1, test_rs)
{
  p_context->run();
  p_context.reset();
  EXPECT_TRUE(flag0);
  EXPECT_FALSE(flag1);
}

TEST_F(coroutine_void_1, test_rg)
{
  p_context->run();
  bool ready = false;
  future.async_get([&ready](corio::expected<void> result) -> void{ result.get(); ready = true; });
  EXPECT_TRUE(flag0);
  EXPECT_FALSE(flag1);
  EXPECT_FALSE(ready);
}

TEST_F(coroutine_void_1, test_gr)
{
  bool ready = false;
  future.async_get([&ready](corio::expected<void> result) -> void{ result.get(); ready = true; });
  p_context->run_for(std::chrono::milliseconds(10));
  EXPECT_TRUE(flag0);
  EXPECT_FALSE(flag1);
  EXPECT_FALSE(ready);
}

TEST_F(coroutine_void_1, test_gs)
{
  bool ready = false;
  future.async_get([&ready](corio::expected<void> result) -> void{ result.get(); ready = true; });
  p_context.reset();
  EXPECT_FALSE(flag0);
  EXPECT_FALSE(flag1);
  EXPECT_FALSE(ready);
}

TEST_F(coroutine_void_1, test_rgs)
{
  p_context->run();
  bool ready = false;
  future.async_get([&ready](corio::expected<void> result) -> void{ result.get(); ready = true; });
  p_context.reset();
  EXPECT_TRUE(flag0);
  EXPECT_FALSE(flag1);
  EXPECT_FALSE(ready);
}

TEST_F(coroutine_void_1, test_grs)
{
  bool ready = false;
  future.async_get([&ready](corio::expected<void> result) -> void{ result.get(); ready = true; });
  p_context->run_for(std::chrono::milliseconds(10));
  p_context.reset();
  EXPECT_TRUE(flag0);
  EXPECT_FALSE(flag1);
  EXPECT_FALSE(ready);
}
