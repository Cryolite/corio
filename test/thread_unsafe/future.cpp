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

class future_value_f
  : public ::testing::Test
{
protected:
  future_value_f()
    : context(),
      promise(context),
      future(promise.get_future()),
      status()
  {}

  boost::asio::io_context context;
  corio::promise<int> promise;
  corio::future<int> future;
  std::future_status status;
}; // class future_value_f

} // namespace *unnamed*

TEST_F(future_value_f, get_executor)
{
  EXPECT_EQ(future.get_executor(), context.get_executor());
}

TEST_F(future_value_f, valid)
{
  EXPECT_TRUE(future.valid());
}

TEST_F(future_value_f, async_get)
{
  int result = 0;
  future.async_get([&result](auto v) -> void{ result = corio::get(v); });
  auto count = context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(count, 0u);
}

TEST_F(future_value_f, async_wait)
{
  future.async_wait([]() -> void{});
  auto count = context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(count, 0u);
}

TEST_F(future_value_f, async_wait_for)
{
  future.async_wait_for(std::chrono::milliseconds(1),
                        [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::timeout);
}

TEST_F(future_value_f, async_wait_until)
{
  future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                          [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::timeout);
}

namespace{

class future_value_v
  : public ::testing::Test
{
protected:
  future_value_v()
    : context(),
      promise(context),
      future(),
      status()
  {
    promise.set_value(42);
  }

  boost::asio::io_context context;
  corio::promise<int> promise;
  corio::future<int> future;
  std::future_status status;
}; // class future_value_v

} // namespace *unnamed*

TEST_F(future_value_v, get_executor)
{
  EXPECT_THROW(future.get_executor();, corio::no_future_state_error);
}

TEST_F(future_value_v, valid)
{
  EXPECT_FALSE(future.valid());
}

TEST_F(future_value_v, async_get)
{
  int result = 0;
  EXPECT_THROW(future.async_get([&result](auto v) -> void{ result = corio::get(v); });,
               corio::no_future_state_error);
}

TEST_F(future_value_v, async_wait)
{
  bool ready = false;
  EXPECT_THROW(future.async_wait([&ready]() -> void{ ready = true; });, corio::no_future_state_error);
}

TEST_F(future_value_v, async_wait_for)
{
  EXPECT_THROW(future.async_wait_for(std::chrono::milliseconds(1),
                                     [this](std::future_status s) -> void{ status = s; });,
               corio::no_future_state_error);
}

TEST_F(future_value_v, async_wait_until)
{
  EXPECT_THROW(future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                                       [this](std::future_status s) -> void{ status = s; });,
               corio::no_future_state_error);
}

namespace{

class future_value_x
  : public ::testing::Test
{
protected:
  future_value_x()
    : context(),
      promise(context),
      future(),
      status()
  {
    auto p = std::make_exception_ptr("Hello, world!");
    promise.set_exception(std::move(p));
  }

  boost::asio::io_context context;
  corio::promise<int> promise;
  corio::future<int> future;
  std::future_status status;
}; // class future_value_x

} // namespace *unnamed*

TEST_F(future_value_x, get_executor)
{
  EXPECT_THROW(future.get_executor();, corio::no_future_state_error);
}

TEST_F(future_value_x, valid)
{
  EXPECT_FALSE(future.valid());
}

TEST_F(future_value_x, async_get)
{
  int result = 0;
  EXPECT_THROW(future.async_get([&result](auto v) -> void{ result = corio::get(v); });,
               corio::no_future_state_error);
}

TEST_F(future_value_x, async_wait)
{
  bool ready = false;
  EXPECT_THROW(future.async_wait([&ready]() -> void{ ready = true; });, corio::no_future_state_error);
}

TEST_F(future_value_x, async_wait_for)
{
  EXPECT_THROW(future.async_wait_for(std::chrono::milliseconds(1),
                                     [this](std::future_status s) -> void{ status = s; });,
               corio::no_future_state_error);
}

TEST_F(future_value_x, async_wait_until)
{
  EXPECT_THROW(future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                                       [this](std::future_status s) -> void{ status = s; });,
               corio::no_future_state_error);
}

namespace{

class future_value_fv
  : public ::testing::Test
{
protected:
  future_value_fv()
    : context(),
      promise(context),
      future(promise.get_future()),
      status()
  {
    promise.set_value(42);
  }

  boost::asio::io_context context;
  corio::promise<int> promise;
  corio::future<int> future;
  std::future_status status;
}; // class future_value_fv

} // namespace *unnamed*

TEST_F(future_value_fv, get_executor)
{
  EXPECT_EQ(future.get_executor(), context.get_executor());
}

TEST_F(future_value_fv, valid)
{
  EXPECT_TRUE(future.valid());
}

TEST_F(future_value_fv, async_get)
{
  int result = 0;
  future.async_get([&result](auto v) -> void{ result = corio::get(v); });
  context.run();
  EXPECT_EQ(result, 42);
}

TEST_F(future_value_fv, async_wait)
{
  bool ready = false;
  future.async_wait([&ready]() -> void{ ready = true; });
  context.run();
  EXPECT_TRUE(ready);
}

TEST_F(future_value_fv, async_wait_for)
{
  future.async_wait_for(std::chrono::milliseconds(1),
                        [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

TEST_F(future_value_fv, async_wait_until)
{
  future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                          [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

namespace{

class future_value_fx
  : public ::testing::Test
{
protected:
  future_value_fx()
    : context(),
      promise(context),
      future(promise.get_future()),
      status()
  {
    auto p = std::make_exception_ptr(std::runtime_error("Hello, world!"));
    promise.set_exception(std::move(p));
  }

  boost::asio::io_context context;
  corio::promise<int> promise;
  corio::future<int> future;
  std::future_status status;
}; // class future_value_fx

} // namespace *unnamed*

TEST_F(future_value_fx, get_executor)
{
  EXPECT_EQ(future.get_executor(), context.get_executor());
}

TEST_F(future_value_fx, valid)
{
  EXPECT_TRUE(future.valid());
}

TEST_F(future_value_fx, async_get)
{
  int result = 0;
  future.async_get([&result](auto v) -> void{ result = corio::get(v); });
  std::string s;
  try{
    context.run();
  }
  catch (std::runtime_error const &e) {
    s = e.what();
  }
  EXPECT_EQ(result, 0);
  EXPECT_EQ(s, "Hello, world!");
}

TEST_F(future_value_fx, async_wait)
{
  bool ready = false;
  future.async_wait([&ready]() -> void{ ready = true; });
  context.run();
  EXPECT_TRUE(ready);
}

TEST_F(future_value_fx, async_wait_for)
{
  future.async_wait_for(std::chrono::milliseconds(1),
                        [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

TEST_F(future_value_fx, async_wait_until)
{
  future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                          [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

namespace{

class future_value_vf
  : public ::testing::Test
{
protected:
  future_value_vf()
    : context(),
      promise(context),
      future(),
      status()
  {
    promise.set_value(42);
    future = promise.get_future();
  }

  boost::asio::io_context context;
  corio::promise<int> promise;
  corio::future<int> future;
  std::future_status status;
}; // class future_value_vf

} // namespace *unnamed*

TEST_F(future_value_vf, get_executor)
{
  EXPECT_EQ(future.get_executor(), context.get_executor());
}

TEST_F(future_value_vf, valid)
{
  EXPECT_TRUE(future.valid());
}

TEST_F(future_value_vf, async_get)
{
  int result = 0;
  future.async_get([&result](auto v) -> void{ result = corio::get(v); });
  context.run();
  EXPECT_EQ(result, 42);
}

TEST_F(future_value_vf, async_wait)
{
  bool ready = false;
  future.async_wait([&ready]() -> void{ ready = true; });
  context.run();
  EXPECT_TRUE(ready);
}

TEST_F(future_value_vf, async_wait_for)
{
  future.async_wait_for(std::chrono::milliseconds(1),
                        [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

TEST_F(future_value_vf, async_wait_until)
{
  future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                          [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

namespace{

class future_value_xf
  : public ::testing::Test
{
protected:
  future_value_xf()
    : context(),
      promise(context),
      future(),
      status()
  {
    auto p = std::make_exception_ptr(std::runtime_error("Hello, world!"));
    promise.set_exception(std::move(p));
    future = promise.get_future();
  }

  boost::asio::io_context context;
  corio::promise<int> promise;
  corio::future<int> future;
  std::future_status status;
}; // class future_value_xf

} // namespace *unnamed*

TEST_F(future_value_xf, get_executor)
{
  EXPECT_EQ(future.get_executor(), context.get_executor());
}

TEST_F(future_value_xf, valid)
{
  EXPECT_TRUE(future.valid());
}

TEST_F(future_value_xf, async_get)
{
  int result = 0;
  future.async_get([&result](auto v) -> void{ result = corio::get(v); });
  std::string s;
  try{
    context.run();
  }
  catch (std::runtime_error const &e) {
    s = e.what();
  }
  EXPECT_EQ(result, 0);
  EXPECT_EQ(s, "Hello, world!");
}

TEST_F(future_value_xf, async_wait)
{
  bool ready = false;
  future.async_wait([&ready]() -> void{ ready = true; });
  context.run();
  EXPECT_TRUE(ready);
}

TEST_F(future_value_xf, async_wait_for)
{
  future.async_wait_for(std::chrono::milliseconds(1),
                        [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

TEST_F(future_value_xf, async_wait_until)
{
  future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                          [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

namespace{

class future_move_f
  : public ::testing::Test
{
protected:
  future_move_f()
    : context(),
      promise(context),
      future(promise.get_future()),
      status()
  {}

  boost::asio::io_context context;
  corio::promise<std::unique_ptr<int> > promise;
  corio::future<std::unique_ptr<int> > future;
  std::future_status status;
}; // class future_move_f

} // namespace *unnamed*

TEST_F(future_move_f, get_executor)
{
  EXPECT_EQ(future.get_executor(), context.get_executor());
}

TEST_F(future_move_f, valid)
{
  EXPECT_TRUE(future.valid());
}

TEST_F(future_move_f, async_get)
{
  std::unique_ptr<int> result;
  future.async_get([&result](auto v) -> void{ result = corio::get(std::move(v)); });
  auto count = context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(count, 0u);
}

TEST_F(future_move_f, async_wait)
{
  future.async_wait([]() -> void{});
  auto count = context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(count, 0u);
}

TEST_F(future_move_f, async_wait_for)
{
  future.async_wait_for(std::chrono::milliseconds(1),
                        [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::timeout);
}

TEST_F(future_move_f, async_wait_until)
{
  future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                          [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::timeout);
}

namespace{

class future_move_v
  : public ::testing::Test
{
protected:
  future_move_v()
    : context(),
      promise(context),
      future(),
      status()
  {
    auto p = std::make_unique<int>(42);
    promise.set_value(std::move(p));
  }

  boost::asio::io_context context;
  corio::promise<std::unique_ptr<int> > promise;
  corio::future<std::unique_ptr<int> > future;
  std::future_status status;
}; // class future_move_v

} // namespace *unnamed*

TEST_F(future_move_v, get_executor)
{
  EXPECT_THROW(future.get_executor();, corio::no_future_state_error);
}

TEST_F(future_move_v, valid)
{
  EXPECT_FALSE(future.valid());
}

TEST_F(future_move_v, async_get)
{
  std::unique_ptr<int> result;
  EXPECT_THROW(future.async_get([&result](auto v) -> void{ result = corio::get(std::move(v)); });,
               corio::no_future_state_error);
}

TEST_F(future_move_v, async_wait)
{
  EXPECT_THROW(future.async_wait([]() -> void{});, corio::no_future_state_error);
}

TEST_F(future_move_v, async_wait_for)
{
  EXPECT_THROW(future.async_wait_for(std::chrono::milliseconds(1),
                                     [this](std::future_status s) -> void{ status = s; });,
               corio::no_future_state_error);
}

TEST_F(future_move_v, async_wait_until)
{
  EXPECT_THROW(future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                                       [this](std::future_status s) -> void{ status = s; });,
               corio::no_future_state_error);
}

namespace{

class future_move_x
  : public ::testing::Test
{
protected:
  future_move_x()
    : context(),
      promise(context),
      future(),
      status()
  {
    auto p = std::make_exception_ptr("Hello, world!");
    promise.set_exception(std::move(p));
  }

  boost::asio::io_context context;
  corio::promise<std::unique_ptr<int> > promise;
  corio::future<std::unique_ptr<int> > future;
  std::future_status status;
}; // class future_move_x

} // namespace *unnamed*

TEST_F(future_move_x, get_executor)
{
  EXPECT_THROW(future.get_executor();, corio::no_future_state_error);
}

TEST_F(future_move_x, valid)
{
  EXPECT_FALSE(future.valid());
}

TEST_F(future_move_x, async_get)
{
  std::unique_ptr<int> result;
  EXPECT_THROW(future.async_get([&result](auto v) -> void{ result = corio::get(std::move(v)); });,
               corio::no_future_state_error);
}

TEST_F(future_move_x, async_wait)
{
  EXPECT_THROW(future.async_wait([]() -> void{});, corio::no_future_state_error);
}

TEST_F(future_move_x, async_wait_for)
{
  EXPECT_THROW(future.async_wait_for(std::chrono::milliseconds(1),
                                     [this](std::future_status s) -> void{ status = s; });,
               corio::no_future_state_error);
}

TEST_F(future_move_x, async_wait_until)
{
  EXPECT_THROW(future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                                       [this](std::future_status s) -> void{ status = s; });,
               corio::no_future_state_error);
}

namespace{

class future_move_fv
  : public ::testing::Test
{
protected:
  future_move_fv()
    : context(),
      promise(context),
      future(promise.get_future()),
      status()
  {
    auto p = std::make_unique<int>(42);
    promise.set_value(std::move(p));
  }

  boost::asio::io_context context;
  corio::promise<std::unique_ptr<int> > promise;
  corio::future<std::unique_ptr<int> > future;
  std::future_status status;
}; // class future_move_fv

} // namespace *unnamed*

TEST_F(future_move_fv, get_executor)
{
  EXPECT_EQ(future.get_executor(), context.get_executor());
}

TEST_F(future_move_fv, valid)
{
  EXPECT_TRUE(future.valid());
}

TEST_F(future_move_fv, async_get)
{
  std::unique_ptr<int> result;
  future.async_get([&result](auto v) -> void{ result = corio::get(std::move(v)); });
  context.run();
  ASSERT_TRUE(!!result);
  EXPECT_EQ(*result, 42);
}

TEST_F(future_move_fv, async_wait)
{
  bool ready = false;
  future.async_wait([&ready]() -> void{ ready = true; });
  context.run();
  EXPECT_TRUE(ready);
}

TEST_F(future_move_fv, async_wait_for)
{
  future.async_wait_for(std::chrono::milliseconds(1),
                        [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

TEST_F(future_move_fv, async_wait_until)
{
  future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                          [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

namespace{

class future_move_fx
  : public ::testing::Test
{
protected:
  future_move_fx()
    : context(),
      promise(context),
      future(promise.get_future()),
      status()
  {
    auto p = std::make_exception_ptr(std::runtime_error("Hello, world!"));
    promise.set_exception(std::move(p));
  }

  boost::asio::io_context context;
  corio::promise<std::unique_ptr<int> > promise;
  corio::future<std::unique_ptr<int> > future;
  std::future_status status;
}; // class future_move_fx

} // namespace *unnamed*

TEST_F(future_move_fx, get_executor)
{
  EXPECT_EQ(future.get_executor(), context.get_executor());
}

TEST_F(future_move_fx, valid)
{
  EXPECT_TRUE(future.valid());
}

TEST_F(future_move_fx, async_get)
{
  std::unique_ptr<int> result;
  future.async_get([&result](auto v) -> void{ result = corio::get(std::move(v)); });
  std::string s;
  try{
    context.run();
  }
  catch (std::runtime_error const &e) {
    s = e.what();
  }
  EXPECT_TRUE(!result);
  EXPECT_EQ(s, "Hello, world!");
}

TEST_F(future_move_fx, async_wait)
{
  bool ready = false;
  future.async_wait([&ready]() -> void{ ready = true; });
  context.run();
  EXPECT_TRUE(ready);
}

TEST_F(future_move_fx, async_wait_for)
{
  future.async_wait_for(std::chrono::milliseconds(1),
                        [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

TEST_F(future_move_fx, async_wait_until)
{
  future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                          [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

namespace{

class future_move_vf
  : public ::testing::Test
{
protected:
  future_move_vf()
    : context(),
      promise(context),
      future(),
      status()
  {
    auto p = std::make_unique<int>(42);
    promise.set_value(std::move(p));
    future = promise.get_future();
  }

  boost::asio::io_context context;
  corio::promise<std::unique_ptr<int> > promise;
  corio::future<std::unique_ptr<int> > future;
  std::future_status status;
}; // class future_move_vf

} // namespace *unnamed*

TEST_F(future_move_vf, get_executor)
{
  EXPECT_EQ(future.get_executor(), context.get_executor());
}

TEST_F(future_move_vf, valid)
{
  EXPECT_TRUE(future.valid());
}

TEST_F(future_move_vf, async_get)
{
  std::unique_ptr<int> result;
  future.async_get([&result](auto v) -> void{ result = corio::get(std::move(v)); });
  context.run();
  ASSERT_FALSE(!result);
  EXPECT_EQ(*result, 42);
}

TEST_F(future_move_vf, async_wait)
{
  bool ready = false;
  future.async_wait([&ready]() -> void{ ready = true; });
  context.run();
  EXPECT_TRUE(ready);
}

TEST_F(future_move_vf, async_wait_for)
{
  future.async_wait_for(std::chrono::milliseconds(1),
                        [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

TEST_F(future_move_vf, async_wait_until)
{
  future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                          [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

namespace{

class future_move_xf
  : public ::testing::Test
{
protected:
  future_move_xf()
    : context(),
      promise(context),
      future(),
      status()
  {
    auto p = std::make_exception_ptr(std::runtime_error("Hello, world!"));
    promise.set_exception(std::move(p));
    future = promise.get_future();
  }

  boost::asio::io_context context;
  corio::promise<std::unique_ptr<int> > promise;
  corio::future<std::unique_ptr<int> > future;
  std::future_status status;
}; // class future_move_xf

} // namespace *unnamed*

TEST_F(future_move_xf, get_executor)
{
  EXPECT_EQ(future.get_executor(), context.get_executor());
}

TEST_F(future_move_xf, valid)
{
  EXPECT_TRUE(future.valid());
}

TEST_F(future_move_xf, async_get)
{
  std::unique_ptr<int> result;
  future.async_get([&result](auto v) -> void{ result = corio::get(std::move(v)); });
  std::string s;
  try{
    context.run();
  }
  catch (std::runtime_error const &e) {
    s = e.what();
  }
  EXPECT_TRUE(!result);
  EXPECT_EQ(s, "Hello, world!");
}

TEST_F(future_move_xf, async_wait)
{
  bool ready = false;
  future.async_wait([&ready]() -> void{ ready = true; });
  context.run();
  EXPECT_TRUE(ready);
}

TEST_F(future_move_xf, async_wait_for)
{
  future.async_wait_for(std::chrono::milliseconds(1),
                        [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

TEST_F(future_move_xf, async_wait_until)
{
  future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                          [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

namespace{

class future_ref_f
  : public ::testing::Test
{
protected:
  future_ref_f()
    : context(),
      promise(context),
      i(),
      future(promise.get_future()),
      status()
  {}

  boost::asio::io_context context;
  corio::promise<int &> promise;
  int i;
  corio::future<int &> future;
  std::future_status status;
}; // class future_ref_f

} // namespace *unnamed*

TEST_F(future_ref_f, get_executor)
{
  EXPECT_EQ(future.get_executor(), context.get_executor());
}

TEST_F(future_ref_f, valid)
{
  EXPECT_TRUE(future.valid());
}

TEST_F(future_ref_f, async_get)
{
  int *p = nullptr;
  future.async_get([&p](auto v) -> void{ p = &corio::get(v); });
  auto count = context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(count, 0u);
}

TEST_F(future_ref_f, async_wait)
{
  future.async_wait([]() -> void{});
  auto count = context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(count, 0u);
}

TEST_F(future_ref_f, async_wait_for)
{
  future.async_wait_for(std::chrono::milliseconds(1),
                        [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::timeout);
}

TEST_F(future_ref_f, async_wait_until)
{
  future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                          [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::timeout);
}

namespace{

class future_ref_v
  : public ::testing::Test
{
protected:
  future_ref_v()
    : context(),
      promise(context),
      i(),
      future(),
      status()
  {
    promise.set_value(i);
  }

  boost::asio::io_context context;
  corio::promise<int &> promise;
  int i;
  corio::future<int &> future;
  std::future_status status;
}; // class future_ref_v

} // namespace *unnamed*

TEST_F(future_ref_v, get_executor)
{
  EXPECT_THROW(future.get_executor();, corio::no_future_state_error);
}

TEST_F(future_ref_v, valid)
{
  EXPECT_FALSE(future.valid());
}

TEST_F(future_ref_v, async_get)
{
  int *p = nullptr;
  EXPECT_THROW(future.async_get([&p](auto v) -> void{ p = &corio::get(v); });,
               corio::no_future_state_error);
}

TEST_F(future_ref_v, async_wait)
{
  EXPECT_THROW(future.async_wait([]() -> void{});, corio::no_future_state_error);
}

TEST_F(future_ref_v, async_wait_for)
{
  EXPECT_THROW(future.async_wait_for(std::chrono::milliseconds(1),
                                     [this](std::future_status s) -> void{ status = s; });,
               corio::no_future_state_error);
}

TEST_F(future_ref_v, async_wait_until)
{
  EXPECT_THROW(future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                                       [this](std::future_status s) -> void{ status = s; });,
               corio::no_future_state_error);
}

namespace{

class future_ref_x
  : public ::testing::Test
{
protected:
  future_ref_x()
    : context(),
      promise(context),
      i(),
      future(),
      status()
  {
    auto p = std::make_exception_ptr("Hello, world!");
    promise.set_exception(std::move(p));
  }

  boost::asio::io_context context;
  corio::promise<int &> promise;
  int i;
  corio::future<int &> future;
  std::future_status status;
}; // class future_ref_x

} // namespace *unnamed*

TEST_F(future_ref_x, get_executor)
{
  EXPECT_THROW(future.get_executor();, corio::no_future_state_error);
}

TEST_F(future_ref_x, valid)
{
  EXPECT_FALSE(future.valid());
}

TEST_F(future_ref_x, async_get)
{
  int *p = nullptr;
  EXPECT_THROW(future.async_get([&p](auto v) -> void{ p = &corio::get(v); });,
               corio::no_future_state_error);
}

TEST_F(future_ref_x, async_wait)
{
  EXPECT_THROW(future.async_wait([]() -> void{});, corio::no_future_state_error);
}

TEST_F(future_ref_x, async_wait_for)
{
  EXPECT_THROW(future.async_wait_for(std::chrono::milliseconds(1),
                                     [this](std::future_status s) -> void{ status = s; });,
               corio::no_future_state_error);
}

TEST_F(future_ref_x, async_wait_until)
{
  EXPECT_THROW(future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                                       [this](std::future_status s) -> void{ status = s; });,
               corio::no_future_state_error);
}

namespace{

class future_ref_fv
  : public ::testing::Test
{
protected:
  future_ref_fv()
    : context(),
      promise(context),
      i(),
      future(promise.get_future()),
      status()
  {
    promise.set_value(i);
  }

  boost::asio::io_context context;
  corio::promise<int &> promise;
  int i;
  corio::future<int &> future;
  std::future_status status;
}; // class future_ref_fv

} // namespace *unnamed*

TEST_F(future_ref_fv, get_executor)
{
  EXPECT_EQ(future.get_executor(), context.get_executor());
}

TEST_F(future_ref_fv, valid)
{
  EXPECT_TRUE(future.valid());
}

TEST_F(future_ref_fv, async_get)
{
  int *p = nullptr;
  future.async_get([&p](auto v) -> void{ p = &corio::get(v); });
  context.run();
  EXPECT_EQ(p, &i);
}

TEST_F(future_ref_fv, async_wait)
{
  bool ready = false;
  future.async_wait([&ready]() -> void{ ready = true; });
  context.run();
  EXPECT_TRUE(ready);
}

TEST_F(future_ref_fv, async_wait_for)
{
  future.async_wait_for(std::chrono::milliseconds(1),
                        [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

TEST_F(future_ref_fv, async_wait_until)
{
  future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                          [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

namespace{

class future_ref_fx
  : public ::testing::Test
{
protected:
  future_ref_fx()
    : context(),
      promise(context),
      i(),
      future(promise.get_future()),
      status()
  {
    auto p = std::make_exception_ptr(std::runtime_error("Hello, world!"));
    promise.set_exception(std::move(p));
  }

  boost::asio::io_context context;
  corio::promise<int &> promise;
  int i;
  corio::future<int &> future;
  std::future_status status;
}; // class future_ref_fx

} // namespace *unnamed*

TEST_F(future_ref_fx, get_executor)
{
  EXPECT_EQ(future.get_executor(), context.get_executor());
}

TEST_F(future_ref_fx, valid)
{
  EXPECT_TRUE(future.valid());
}

TEST_F(future_ref_fx, async_get)
{
  int *p = nullptr;
  future.async_get([&p](auto v) -> void{ p = &corio::get(v); });
  std::string s;
  try{
    context.run();
  }
  catch (std::runtime_error const &e) {
    s = e.what();
  }
  EXPECT_TRUE(!p);
  EXPECT_EQ(s, "Hello, world!");
}

TEST_F(future_ref_fx, async_wait)
{
  bool ready = false;
  future.async_wait([&ready]() -> void{ ready = true; });
  context.run();
  EXPECT_TRUE(ready);
}

TEST_F(future_ref_fx, async_wait_for)
{
  future.async_wait_for(std::chrono::milliseconds(1),
                        [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

TEST_F(future_ref_fx, async_wait_until)
{
  future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                          [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

namespace{

class future_ref_vf
  : public ::testing::Test
{
protected:
  future_ref_vf()
    : context(),
      promise(context),
      i(),
      future(),
      status()
  {
    promise.set_value(i);
    future = promise.get_future();
  }

  boost::asio::io_context context;
  corio::promise<int &> promise;
  int i;
  corio::future<int &> future;
  std::future_status status;
}; // class future_ref_vf

} // namespace *unnamed*

TEST_F(future_ref_vf, get_executor)
{
  EXPECT_EQ(future.get_executor(), context.get_executor());
}

TEST_F(future_ref_vf, valid)
{
  EXPECT_TRUE(future.valid());
}

TEST_F(future_ref_vf, async_get)
{
  int *p = nullptr;
  future.async_get([&p](auto v) -> void{ p = &corio::get(v); });
  context.run();
  EXPECT_EQ(p, &i);
}

TEST_F(future_ref_vf, async_wait)
{
  bool ready = false;
  future.async_wait([&ready]() -> void{ ready = true; });
  context.run();
  EXPECT_TRUE(ready);
}

TEST_F(future_ref_vf, async_wait_for)
{
  future.async_wait_for(std::chrono::milliseconds(1),
                        [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

TEST_F(future_ref_vf, async_wait_until)
{
  future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                          [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

namespace{

class future_ref_xf
  : public ::testing::Test
{
protected:
  future_ref_xf()
    : context(),
      promise(context),
      i(),
      future(),
      status()
  {
    auto p = std::make_exception_ptr(std::runtime_error("Hello, world!"));
    promise.set_exception(std::move(p));
    future = promise.get_future();
  }

  boost::asio::io_context context;
  corio::promise<int &> promise;
  int i;
  corio::future<int &> future;
  std::future_status status;
}; // class future_ref_xf

} // namespace *unnamed*

TEST_F(future_ref_xf, get_executor)
{
  EXPECT_EQ(future.get_executor(), context.get_executor());
}

TEST_F(future_ref_xf, valid)
{
  EXPECT_TRUE(future.valid());
}

TEST_F(future_ref_xf, async_get)
{
  int *p = nullptr;
  future.async_get([&p](auto v) -> void{ p = &corio::get(v); });
  std::string s;
  try{
    context.run();
  }
  catch (std::runtime_error const &e) {
    s = e.what();
  }
  EXPECT_TRUE(!p);
  EXPECT_EQ(s, "Hello, world!");
}

TEST_F(future_ref_xf, async_wait)
{
  bool ready = false;
  future.async_wait([&ready]() -> void{ ready = true; });
  context.run();
  EXPECT_TRUE(ready);
}

TEST_F(future_ref_xf, async_wait_for)
{
  future.async_wait_for(std::chrono::milliseconds(1),
                        [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

TEST_F(future_ref_xf, async_wait_until)
{
  future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                          [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

namespace{

class future_void_f
  : public ::testing::Test
{
protected:
  future_void_f()
    : context(),
      promise(context),
      future(promise.get_future()),
      status()
  {}

  boost::asio::io_context context;
  corio::promise<void> promise;
  corio::future<void> future;
  std::future_status status;
}; // class future_void_f

} // namespace *unnamed*

TEST_F(future_void_f, get_executor)
{
  EXPECT_EQ(future.get_executor(), context.get_executor());
}

TEST_F(future_void_f, valid)
{
  EXPECT_TRUE(future.valid());
}

TEST_F(future_void_f, async_get)
{
  bool result = false;
  future.async_get([&result](auto v) -> void{ corio::get(v); result = true; });
  auto count = context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(count, 0u);
}

TEST_F(future_void_f, async_wait)
{
  future.async_wait([]() -> void{});
  auto count = context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(count, 0u);
}

TEST_F(future_void_f, async_wait_for)
{
  future.async_wait_for(std::chrono::milliseconds(1),
                        [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::timeout);
}

TEST_F(future_void_f, async_wait_until)
{
  future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                          [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::timeout);
}

namespace{

class future_void_v
  : public ::testing::Test
{
protected:
  future_void_v()
    : context(),
      promise(context),
      future(),
      status()
  {
    promise.set_value();
  }

  boost::asio::io_context context;
  corio::promise<void> promise;
  corio::future<void> future;
  std::future_status status;
}; // class future_void_v

} // namespace *unnamed*

TEST_F(future_void_v, get_executor)
{
  EXPECT_THROW(future.get_executor();, corio::no_future_state_error);
}

TEST_F(future_void_v, valid)
{
  EXPECT_FALSE(future.valid());
}

TEST_F(future_void_v, async_get)
{
  EXPECT_THROW(future.async_get([](auto v) -> void{ corio::get(v); });,
               corio::no_future_state_error);
}

TEST_F(future_void_v, async_wait)
{
  EXPECT_THROW(future.async_wait([]() -> void{});, corio::no_future_state_error);
}

TEST_F(future_void_v, async_wait_for)
{
  EXPECT_THROW(future.async_wait_for(std::chrono::milliseconds(1),
                                     [this](std::future_status s) -> void{ status = s; });,
               corio::no_future_state_error);
}

TEST_F(future_void_v, async_wait_until)
{
  EXPECT_THROW(future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                                       [this](std::future_status s) -> void{ status = s; });,
               corio::no_future_state_error);
}

namespace{

class future_void_x
  : public ::testing::Test
{
protected:
  future_void_x()
    : context(),
      promise(context),
      future(),
      status()
  {
    auto p = std::make_exception_ptr("Hello, world!");
    promise.set_exception(std::move(p));
  }

  boost::asio::io_context context;
  corio::promise<void> promise;
  corio::future<void> future;
  std::future_status status;
}; // class future_void_x

} // namespace *unnamed*

TEST_F(future_void_x, get_executor)
{
  EXPECT_THROW(future.get_executor();, corio::no_future_state_error);
}

TEST_F(future_void_x, valid)
{
  EXPECT_FALSE(future.valid());
}

TEST_F(future_void_x, async_get)
{
  EXPECT_THROW(future.async_get([](auto v) -> void{ corio::get(v); });,
               corio::no_future_state_error);
}

TEST_F(future_void_x, async_wait)
{
  EXPECT_THROW(future.async_wait([]() -> void{});, corio::no_future_state_error);
}

TEST_F(future_void_x, async_wait_for)
{
  EXPECT_THROW(future.async_wait_for(std::chrono::milliseconds(1),
                                     [this](std::future_status s) -> void{ status = s; });,
               corio::no_future_state_error);
}

TEST_F(future_void_x, async_wait_until)
{
  EXPECT_THROW(future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                                       [this](std::future_status s) -> void{ status = s; });,
               corio::no_future_state_error);
}

namespace{

class future_void_fv
  : public ::testing::Test
{
protected:
  future_void_fv()
    : context(),
      promise(context),
      future(promise.get_future()),
      status()
  {
    promise.set_value();
  }

  boost::asio::io_context context;
  corio::promise<void> promise;
  corio::future<void> future;
  std::future_status status;
}; // class future_void_fv

} // namespace *unnamed*

TEST_F(future_void_fv, get_executor)
{
  EXPECT_EQ(future.get_executor(), context.get_executor());
}

TEST_F(future_void_fv, valid)
{
  EXPECT_TRUE(future.valid());
}

TEST_F(future_void_fv, async_get)
{
  bool result = false;
  future.async_get([&result](auto v) -> void{ corio::get(v); result = true; });
  context.run();
  EXPECT_TRUE(result);
}

TEST_F(future_void_fv, async_wait)
{
  bool ready = false;
  future.async_wait([&ready]() -> void{ ready = true; });
  context.run();
  EXPECT_TRUE(ready);
}

TEST_F(future_void_fv, async_wait_for)
{
  future.async_wait_for(std::chrono::milliseconds(1),
                        [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

TEST_F(future_void_fv, async_wait_until)
{
  future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                          [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

namespace{

class future_void_fx
  : public ::testing::Test
{
protected:
  future_void_fx()
    : context(),
      promise(context),
      future(promise.get_future()),
      status()
  {
    auto p = std::make_exception_ptr(std::runtime_error("Hello, world!"));
    promise.set_exception(std::move(p));
  }

  boost::asio::io_context context;
  corio::promise<void> promise;
  corio::future<void> future;
  std::future_status status;
}; // class future_void_fx

} // namespace *unnamed*

TEST_F(future_void_fx, get_executor)
{
  EXPECT_EQ(future.get_executor(), context.get_executor());
}

TEST_F(future_void_fx, valid)
{
  EXPECT_TRUE(future.valid());
}

TEST_F(future_void_fx, async_get)
{
  bool result = false;
  future.async_get([&result](auto v) -> void{ corio::get(v); result = true; });
  std::string s;
  try{
    context.run();
  }
  catch (std::runtime_error const &e) {
    s = e.what();
  }
  EXPECT_FALSE(result);
  EXPECT_EQ(s, "Hello, world!");
}

TEST_F(future_void_fx, async_wait)
{
  bool ready = false;
  future.async_wait([&ready]() -> void{ ready = true; });
  context.run();
  EXPECT_TRUE(ready);
}

TEST_F(future_void_fx, async_wait_for)
{
  future.async_wait_for(std::chrono::milliseconds(1),
                        [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

TEST_F(future_void_fx, async_wait_until)
{
  future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                          [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

namespace{

class future_void_vf
  : public ::testing::Test
{
protected:
  future_void_vf()
    : context(),
      promise(context),
      future(),
      status()
  {
    promise.set_value();
    future = promise.get_future();
  }

  boost::asio::io_context context;
  corio::promise<void> promise;
  corio::future<void> future;
  std::future_status status;
}; // class future_void_vf

} // namespace *unnamed*

TEST_F(future_void_vf, get_executor)
{
  EXPECT_EQ(future.get_executor(), context.get_executor());
}

TEST_F(future_void_vf, valid)
{
  EXPECT_TRUE(future.valid());
}

TEST_F(future_void_vf, async_get)
{
  bool result = false;
  future.async_get([&result](auto v) -> void{ corio::get(v); result = true; });
  context.run();
  EXPECT_TRUE(result);
}

TEST_F(future_void_vf, async_wait)
{
  bool ready = false;
  future.async_wait([&ready]() -> void{ ready = true; });
  context.run();
  EXPECT_TRUE(ready);
}

TEST_F(future_void_vf, async_wait_for)
{
  future.async_wait_for(std::chrono::milliseconds(1),
                        [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

TEST_F(future_void_vf, async_wait_until)
{
  future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                          [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

namespace{

class future_void_xf
  : public ::testing::Test
{
protected:
  future_void_xf()
    : context(),
      promise(context),
      future(),
      status()
  {
    auto p = std::make_exception_ptr(std::runtime_error("Hello, world!"));
    promise.set_exception(std::move(p));
    future = promise.get_future();
  }

  boost::asio::io_context context;
  corio::promise<void> promise;
  corio::future<void> future;
  std::future_status status;
}; // class future_void_xf

} // namespace *unnamed*

TEST_F(future_void_xf, get_executor)
{
  EXPECT_EQ(future.get_executor(), context.get_executor());
}

TEST_F(future_void_xf, valid)
{
  EXPECT_TRUE(future.valid());
}

TEST_F(future_void_xf, async_get)
{
  bool result = false;
  future.async_get([&result](auto v) -> void{ corio::get(v); result = true; });
  std::string s;
  try{
    context.run();
  }
  catch (std::runtime_error const &e) {
    s = e.what();
  }
  EXPECT_FALSE(result);
  EXPECT_EQ(s, "Hello, world!");
}

TEST_F(future_void_xf, async_wait)
{
  bool ready = false;
  future.async_wait([&ready]() -> void{ ready = true; });
  context.run();
  EXPECT_TRUE(ready);
}

TEST_F(future_void_xf, async_wait_for)
{
  future.async_wait_for(std::chrono::milliseconds(1),
                        [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}

TEST_F(future_void_xf, async_wait_until)
{
  future.async_wait_until(std::chrono::steady_clock::now() + std::chrono::milliseconds(1),
                          [this](std::future_status s) -> void{ status = s; });
  context.run();
  EXPECT_EQ(status, std::future_status::ready);
}
