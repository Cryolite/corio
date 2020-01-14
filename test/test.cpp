#include <corio/thread_unsafe/coroutine.hpp>
#include <corio/thread_unsafe/promise.hpp>

#include <gtest/gtest.h>
#include <experimental/coroutine>


namespace corio
{ using namespace thread_unsafe; }

corio::coroutine<void> f()
{
  co_return;
}

TEST(test_corio, coroutine)
{
  corio::coroutine<void> coro = f();
  ASSERT_FALSE(coro.done());
  coro();
  ASSERT_TRUE(coro.done());
}
