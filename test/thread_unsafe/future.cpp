#include <corio/thread_unsafe/future.hpp>

#include <corio/thread_unsafe/promise.hpp>
#include <gtest/gtest.h>
#include <boost/asio/io_context.hpp>


// Fixture class names follow the convention
// `future(_value|_move|_ref|_void)(_[efvx]+)?`, where
//
//   - `future_value_*` stands for tests on `future<int>`,
//   - `future_move_*` stands for tests on `future<std::unique_ptr<int> >`,
//   - `future_ref_*` stands for tests on `future<int &>`,
//   - `future_void_*` stands for tests on `future<void>`,
//   - `e` suffix indicates that an executor is already associated to a promise
//     before a future to test is retrieved from that promise,
//   - `v` suffix indicates that a promise has been already satisfied with a
//     value before a future to test is retrieved from that promise, and
//   - `x` suffix indicates that a promise has been already satisfied with an
//     exception before a future to test is retrieved from that promise.

namespace corio
{ using namespace thread_unsafe; }

namespace{

class future_value
  : public ::testing::Test
{
protected:
  future_value()
    : context(),
      promise(),
      future(promise.get_future())
  {}

  using context_type = boost::asio::io_context;
  context_type context;
  corio::promise<int> promise;
  corio::future<int> future;
}; // class future_value

} // namespace *unnamed*

TEST_F(future_value, has_executor)
{
  EXPECT_FALSE(future.has_executor());
}

TEST_F(future_value, set_executor)
{
  future.set_executor(context.get_executor());
  EXPECT_TRUE(future.has_executor());
  EXPECT_EQ(future.get_executor(), context.get_executor());
  EXPECT_TRUE(promise.has_executor());
  EXPECT_EQ(promise.get_executor(), context.get_executor());
}

TEST_F(future_value, get_executor)
{
  EXPECT_THROW(future.get_executor();, corio::no_executor_error);
}

TEST_F(future_value, valid)
{
  EXPECT_TRUE(future.valid());
}

TEST_F(future_value, async_get)
{
  int result = 0;
  future.async_get([&result](auto v) -> void{ result = corio::get(v); });
}

TEST_F(future_value, async_wait)
{
  future.async_wait([]() -> void{});
}
