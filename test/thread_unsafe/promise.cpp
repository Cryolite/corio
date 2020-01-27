#include <corio/thread_unsafe/promise.hpp>

#include <corio/thread_unsafe/future.hpp>
#include <gtest/gtest.h>
#include <boost/asio/io_context.hpp>


// Fixture class names follow the convention
// `promise(_value|_move|_ref|_void)(_[efvx]+)?`, where
//
//   - `promise_value_*` stands for tests on `promise<int>`,
//   - `promise_move_*` stands for tests on `promise<std::unique_ptr<int> >`,
//   - `promise_ref_*` stands for tests on `promise<int &>`,
//   - `promise_void_*` stands for tests on `promise<void>`,
//   - `e` suffix indicates that an executor is already associated,
//   - `f` suffix indicates that a future is already retrieved,
//   - `v` suffix indicates that the promise has been already satisfied with a
//     value, and
//   - `x` suffix indicates that the promise has been already satisfied with an
//     exception.

namespace corio
{ using namespace thread_unsafe; }

namespace{

class promise_value
  : public ::testing::Test
{
protected:
  promise_value()
    : context(),
      promise()
  {}

  boost::asio::io_context context;
  corio::promise<int> promise;
}; // class promise_value

} // namespace *unnamed*

TEST_F(promise_value, has_executor)
{
  EXPECT_FALSE(promise.has_executor());
}

TEST_F(promise_value, set_executor)
{
  promise.set_executor(context.get_executor());
  EXPECT_TRUE(promise.has_executor());
  EXPECT_EQ(promise.get_executor(), context.get_executor());
}

TEST_F(promise_value, get_executor)
{
  EXPECT_THROW(promise.get_executor();, corio::no_executor_error);
}

TEST_F(promise_value, get_future)
{
  auto future = promise.get_future();
  EXPECT_FALSE(future.has_executor());
  EXPECT_TRUE(future.valid());
}

TEST_F(promise_value, set_value)
{
  promise.set_value(42);
}

TEST_F(promise_value, set_exception)
{
  auto p = std::make_exception_ptr(std::runtime_error("Hello, world!"));
  promise.set_exception(p);
}

namespace{

class promise_value_e
  : public ::testing::Test
{
protected:
  promise_value_e()
    : context(),
      promise(context)
  {}

  boost::asio::io_context context;
  corio::promise<int> promise;
}; // class promise_value_e

} // namespace *unnamed*

TEST_F(promise_value_e, has_executor)
{
  EXPECT_TRUE(promise.has_executor());
}

TEST_F(promise_value_e, set_executor)
{
  EXPECT_THROW(promise.set_executor(context.get_executor());,
               corio::executor_already_assigned_error);
}

TEST_F(promise_value_e, get_executor)
{
  EXPECT_EQ(promise.get_executor(), context.get_executor());
}

TEST_F(promise_value_e, get_future)
{
  auto future = promise.get_future();
  EXPECT_TRUE(future.has_executor());
  EXPECT_EQ(future.get_executor(), context.get_executor());
  EXPECT_TRUE(future.valid());
}

TEST_F(promise_value_e, set_value)
{
  promise.set_value(42);
}

TEST_F(promise_value_e, set_exception)
{
  auto p = std::make_exception_ptr(std::runtime_error("Hello, world!"));
  promise.set_exception(std::move(p));
}

namespace{

class promise_value_f
  : public ::testing::Test
{
protected:
  promise_value_f()
    : context(),
      promise(),
      future(promise.get_future())
  {}

  boost::asio::io_context context;
  corio::promise<int> promise;
  corio::future<int> future;
}; // class promise_value_f

} // namespace *unnamed*

TEST_F(promise_value_f, has_executor)
{
  EXPECT_FALSE(promise.has_executor());
  EXPECT_FALSE(future.has_executor());
}

TEST_F(promise_value_f, set_executor)
{
  promise.set_executor(context.get_executor());
  EXPECT_TRUE(promise.has_executor());
  EXPECT_EQ(promise.get_executor(), context.get_executor());
  EXPECT_TRUE(future.has_executor());
  EXPECT_EQ(future.get_executor(), context.get_executor());
}

TEST_F(promise_value_f, get_executor)
{
  EXPECT_THROW(promise.get_executor();, corio::no_executor_error);
}

TEST_F(promise_value_f, get_future)
{
  EXPECT_THROW(promise.get_future();, corio::future_already_retrieved_error);
}

TEST_F(promise_value_f, set_value)
{
  promise.set_value(42);
}

TEST_F(promise_value_f, set_exception)
{
  auto p = std::make_exception_ptr(std::runtime_error("Hello, world!"));
  promise.set_exception(p);
}

namespace{

class promise_value_v
  : public ::testing::Test
{
protected:
  promise_value_v()
    : context(),
      promise()
  {
    promise.set_value(42);
  }

  boost::asio::io_context context;
  corio::promise<int> promise;
}; // class promise_value_v

} // namespace *unnamed*

TEST_F(promise_value_v, has_executor)
{
  EXPECT_FALSE(promise.has_executor());
}

TEST_F(promise_value_v, set_executor)
{
  promise.set_executor(context.get_executor());
  EXPECT_TRUE(promise.has_executor());
  EXPECT_EQ(promise.get_executor(), context.get_executor());
}

TEST_F(promise_value_v, get_executor)
{
  EXPECT_THROW(promise.get_executor();, corio::no_executor_error);
}

TEST_F(promise_value_v, get_future)
{
  auto future = promise.get_future();
  EXPECT_FALSE(future.has_executor());
  EXPECT_TRUE(future.valid());
}

TEST_F(promise_value_v, set_value)
{
  EXPECT_THROW(promise.set_value(42);, corio::promise_already_satisfied_error);
}

TEST_F(promise_value_v, set_exception)
{
  auto p = std::make_exception_ptr(std::runtime_error("Hello, world!"));
  EXPECT_THROW(promise.set_exception(p);, corio::promise_already_satisfied_error);
}

namespace{

class promise_value_x
  : public ::testing::Test
{
protected:
  promise_value_x()
    : context(),
      promise()
  {
    auto p = std::make_exception_ptr(std::runtime_error("Hello, world!"));
    promise.set_exception(p);
  }

  boost::asio::io_context context;
  corio::promise<int> promise;
}; // class promise_value_x

} // namespace *unnamed*

TEST_F(promise_value_x, has_executor)
{
  EXPECT_FALSE(promise.has_executor());
}

TEST_F(promise_value_x, set_executor)
{
  promise.set_executor(context.get_executor());
  EXPECT_TRUE(promise.has_executor());
  EXPECT_EQ(promise.get_executor(), context.get_executor());
}

TEST_F(promise_value_x, get_executor)
{
  EXPECT_THROW(promise.get_executor();, corio::no_executor_error);
}

TEST_F(promise_value_x, get_future)
{
  auto future = promise.get_future();
  EXPECT_FALSE(future.has_executor());
  EXPECT_TRUE(future.valid());
}

TEST_F(promise_value_x, set_value)
{
  EXPECT_THROW(promise.set_value(42);, corio::promise_already_satisfied_error);
}

TEST_F(promise_value_x, set_exception)
{
  auto p = std::make_exception_ptr(std::runtime_error("Hello, world!"));
  EXPECT_THROW(promise.set_exception(p);, corio::promise_already_satisfied_error);
}

namespace{

class promise_value_ef
  : public ::testing::Test
{
protected:
  promise_value_ef()
    : context(),
      promise(context),
      future(promise.get_future())
  {}

  boost::asio::io_context context;
  corio::promise<int> promise;
  corio::future<int> future;
}; // class promise_value_ef

} // namespace *unnamed*

TEST_F(promise_value_ef, has_executor)
{
  EXPECT_TRUE(promise.has_executor());
}

TEST_F(promise_value_ef, set_executor)
{
  EXPECT_THROW(promise.set_executor(context.get_executor());,
               corio::executor_already_assigned_error);
}

TEST_F(promise_value_ef, get_executor)
{
  EXPECT_EQ(promise.get_executor(), context.get_executor());
}

TEST_F(promise_value_ef, get_future)
{
  EXPECT_THROW(promise.get_future();, corio::future_already_retrieved_error);
}

TEST_F(promise_value_ef, set_value)
{
  promise.set_value(42);
}

TEST_F(promise_value_ef, set_exception)
{
  auto p = std::make_exception_ptr(std::runtime_error("Hello, world!"));
  promise.set_exception(p);
}

namespace{

class promise_value_ev
  : public ::testing::Test
{
protected:
  promise_value_ev()
    : context(),
      promise(context)
  {
    promise.set_value(42);
  }

  boost::asio::io_context context;
  corio::promise<int> promise;
}; // class promise_value_ev

} // namespace *unnamed*

TEST_F(promise_value_ev, has_executor)
{
  EXPECT_TRUE(promise.has_executor());
}

TEST_F(promise_value_ev, set_executor)
{
  EXPECT_THROW(promise.set_executor(context.get_executor());, corio::executor_already_assigned_error);
}

TEST_F(promise_value_ev, get_executor)
{
  EXPECT_EQ(promise.get_executor(), context.get_executor());
}

TEST_F(promise_value_ev, get_future)
{
  auto future = promise.get_future();
  EXPECT_TRUE(future.has_executor());
  EXPECT_EQ(future.get_executor(), context.get_executor());
}

TEST_F(promise_value_ev, set_value)
{
  EXPECT_THROW(promise.set_value(42);, corio::promise_already_satisfied_error);
}

TEST_F(promise_value_ev, set_exception)
{
  auto p = std::make_exception_ptr(std::runtime_error("Hello, world!"));
  EXPECT_THROW(promise.set_exception(p);, corio::promise_already_satisfied_error);
}

namespace{

class promise_value_ex
  : public ::testing::Test
{
protected:
  promise_value_ex()
    : context(),
      promise(context)
  {
    auto p = std::make_exception_ptr(std::runtime_error("Hello, world!"));
    promise.set_exception(p);
  }

  boost::asio::io_context context;
  corio::promise<int> promise;
}; // class promise_value_ex

} // namespace *unnamed*

TEST_F(promise_value_ex, has_executor)
{
  EXPECT_TRUE(promise.has_executor());
}

TEST_F(promise_value_ex, set_executor)
{
  EXPECT_THROW(promise.set_executor(context.get_executor());,
               corio::executor_already_assigned_error);
}

TEST_F(promise_value_ex, get_executor)
{
  EXPECT_EQ(promise.get_executor(), context.get_executor());
}

TEST_F(promise_value_ex, get_future)
{
  auto future = promise.get_future();
  EXPECT_TRUE(future.has_executor());
  EXPECT_EQ(future.get_executor(), context.get_executor());
}

TEST_F(promise_value_ex, set_value)
{
  EXPECT_THROW(promise.set_value(42);, corio::promise_already_satisfied_error);
}

TEST_F(promise_value_ex, set_exception)
{
  auto p = std::make_exception_ptr(std::runtime_error("Hello, world!"));
  EXPECT_THROW(promise.set_exception(p);, corio::promise_already_satisfied_error);
}

namespace{

class promise_value_fv
  : public ::testing::Test
{
protected:
  promise_value_fv()
    : context(),
      promise(),
      future(promise.get_future())
  {
    promise.set_value(42);
  }

  boost::asio::io_context context;
  corio::promise<int> promise;
  corio::future<int> future;
}; // class promise_value_fv

} // namespace *unnamed*

TEST_F(promise_value_fv, has_executor)
{
  EXPECT_FALSE(promise.has_executor());
}

TEST_F(promise_value_fv, set_executor)
{
  promise.set_executor(context.get_executor());
  EXPECT_TRUE(promise.has_executor());
  EXPECT_EQ(promise.get_executor(), context.get_executor());
}

TEST_F(promise_value_fv, get_executor)
{
  EXPECT_THROW(promise.get_executor();, corio::no_executor_error);
}

TEST_F(promise_value_fv, get_future)
{
  EXPECT_THROW(promise.get_future();, corio::future_already_retrieved_error);
}

TEST_F(promise_value_fv, set_value)
{
  EXPECT_THROW(promise.set_value(42);, corio::promise_already_satisfied_error);
}

TEST_F(promise_value_fv, set_exception)
{
  auto p = std::make_exception_ptr(std::runtime_error("Hello, world!"));
  EXPECT_THROW(promise.set_exception(p);, corio::promise_already_satisfied_error);
}

namespace{

class promise_value_fx
  : public ::testing::Test
{
protected:
  promise_value_fx()
    : context(),
      promise(),
      future(promise.get_future())
  {
    auto p = std::make_exception_ptr(std::runtime_error("Hello, world!"));
    promise.set_exception(p);
  }

  boost::asio::io_context context;
  corio::promise<int> promise;
  corio::future<int> future;
}; // class promise_value_fx

} // namespace *unnamed*

TEST_F(promise_value_fx, has_executor)
{
  EXPECT_FALSE(promise.has_executor());
}

TEST_F(promise_value_fx, set_executor)
{
  promise.set_executor(context.get_executor());
  EXPECT_TRUE(promise.has_executor());
  EXPECT_EQ(promise.get_executor(), context.get_executor());
}

TEST_F(promise_value_fx, get_executor)
{
  EXPECT_THROW(promise.get_executor();, corio::no_executor_error);
}

TEST_F(promise_value_fx, get_future)
{
  EXPECT_THROW(promise.get_future();, corio::future_already_retrieved_error);
}

TEST_F(promise_value_fx, set_value)
{
  EXPECT_THROW(promise.set_value(42);, corio::promise_already_satisfied_error);
}

TEST_F(promise_value_fx, set_exception)
{
  auto p = std::make_exception_ptr(std::runtime_error("Hello, world!"));
  EXPECT_THROW(promise.set_exception(p);, corio::promise_already_satisfied_error);
}

namespace{

class promise_value_efv
  : public ::testing::Test
{
protected:
  promise_value_efv()
    : context(),
      promise(context),
      future(promise.get_future())
  {
    promise.set_value(42);
  }

  boost::asio::io_context context;
  corio::promise<int> promise;
  corio::future<int> future;
}; // class promise_value_efv

} // namespace *unnamed*

TEST_F(promise_value_efv, has_executor)
{
  EXPECT_TRUE(promise.has_executor());
}

TEST_F(promise_value_efv, set_executor)
{
  EXPECT_THROW(promise.set_executor(context.get_executor());, corio::executor_already_assigned_error);
}

TEST_F(promise_value_efv, get_executor)
{
  EXPECT_EQ(promise.get_executor(), context.get_executor());
}

TEST_F(promise_value_efv, get_future)
{
  EXPECT_THROW(promise.get_future();, corio::future_already_retrieved_error);
}

TEST_F(promise_value_efv, set_value)
{
  EXPECT_THROW(promise.set_value(42);, corio::promise_already_satisfied_error);
}

TEST_F(promise_value_efv, set_exception)
{
  auto p = std::make_exception_ptr(std::runtime_error("Hello, world!"));
  EXPECT_THROW(promise.set_exception(p);, corio::promise_already_satisfied_error);
}

namespace{

class promise_value_efx
  : public ::testing::Test
{
protected:
  promise_value_efx()
    : context(),
      promise(context),
      future(promise.get_future())
  {
    auto p = std::make_exception_ptr("Hello, world!");
    promise.set_exception(p);
  }

  boost::asio::io_context context;
  corio::promise<int> promise;
  corio::future<int> future;
}; // class promise_value_efx

} // namespace *unnamed*

TEST_F(promise_value_efx, has_executor)
{
  EXPECT_TRUE(promise.has_executor());
}

TEST_F(promise_value_efx, set_executor)
{
  EXPECT_THROW(promise.set_executor(context.get_executor());, corio::executor_already_assigned_error);
}

TEST_F(promise_value_efx, get_executor)
{
  EXPECT_EQ(promise.get_executor(), context.get_executor());
}

TEST_F(promise_value_efx, get_future)
{
  EXPECT_THROW(promise.get_future();, corio::future_already_retrieved_error);
}

TEST_F(promise_value_efx, set_value)
{
  EXPECT_THROW(promise.set_value(42);, corio::promise_already_satisfied_error);
}

TEST_F(promise_value_efx, set_exception)
{
  auto p = std::make_exception_ptr(std::runtime_error("Hello, world!"));
  EXPECT_THROW(promise.set_exception(p);, corio::promise_already_satisfied_error);
}
