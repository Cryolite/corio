#include <corio/thread_unsafe/coroutine.hpp>

#include <gtest/gtest.h>
#include <experimental/coroutine>


namespace corio
{ using namespace thread_unsafe; }

namespace{

int i = 0;

corio::coroutine<void> void_coro()
{
  ++i;
  co_return;
}

} // namespace *unnamed*

TEST(coroutine, void_co_return)
{
  corio::coroutine<void> coro = void_coro();
  EXPECT_EQ(i, 0);
  ASSERT_FALSE(coro.done());
  coro.resume();
  EXPECT_EQ(i, 1);
  EXPECT_TRUE(coro.done());
}

corio::coroutine<void> void_flow_off()
{
  co_await std::experimental::suspend_always{};
}

TEST(coroutine, void_flow_off)
{
  corio::coroutine<void> coro = void_flow_off();
  ASSERT_FALSE(coro.done());
  coro.resume();
  ASSERT_FALSE(coro.done());
  coro.resume();
  EXPECT_TRUE(coro.done());
}
