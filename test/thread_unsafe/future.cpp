#include <corio/thread_unsafe/future.hpp>

#include <corio/thread_unsafe/promise.hpp>
#include <gtest/gtest.h>
#include <boost/asio/io_context.hpp>


// Fixture class names follow the convention
// `future(_value|_move|_ref|_void)(_[fvx]+)?`, where
//
//   - `future_value_*` stands for tests on `future<int>`,
//   - `future_move_*` stands for tests on `future<std::unique_ptr<int> >`,
//   - `future_ref_*` stands for tests on `future<int &>`,
//   - `future_void_*` stands for tests on `future<void>`,
//   - `f` suffix indicates that a future is already retrieved from the
//     promise,
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
      promise(context),
      future(promise.get_future())
  {}

  boost::asio::io_context context;
  corio::promise<int> promise;
  corio::future<int> future;
}; // class future_value

} // namespace *unnamed*

TEST_F(future_value, get_executor)
{
  EXPECT_EQ(future.get_executor(), context.get_executor());
}

TEST_F(future_value, valid)
{
  EXPECT_TRUE(future.valid());
}

#if 0

TEST_F(future_value, async_get)
{
  int result = 0;
  future.async_get([&result](auto v) -> void{ result = corio::get(v); });
}

TEST_F(future_value, async_wait)
{
  future.async_wait([]() -> void{});
}

#endif
