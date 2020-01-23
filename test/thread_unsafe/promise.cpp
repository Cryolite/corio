#include <corio/thread_unsafe/promise.hpp>

#include <corio/thread_unsafe/future.hpp>
#include <gtest/gtest.h>
#include <boost/asio/io_context.hpp>


namespace corio
{ using namespace thread_unsafe; }

TEST(promise_value, constructor)
{
  using context = boost::asio::io_context;
  context ctx;
  using executor = context::executor_type;
  using promise = corio::thread_unsafe::basic_promise<int, executor>;
  promise prom(ctx);
}

TEST(promise_value, get_future)
{
  using context = boost::asio::io_context;
  context ctx;
  using executor = context::executor_type;
  using promise = corio::thread_unsafe::basic_promise<int, executor>;
  promise prom(ctx);

  using future = corio::basic_future<int, executor>;
  [[maybe_unused]] future fut = prom.get_future();
  EXPECT_TRUE(fut.valid());
}

TEST(promise_value, set_value_before_get_future)
{
  using context = boost::asio::io_context;
  context ctx;
  using executor = context::executor_type;
  using promise = corio::thread_unsafe::basic_promise<int, executor>;

  promise prom(ctx);
  prom.set_value(42);

  using future = corio::basic_future<int, executor>;
  [[maybe_unused]] future fut = prom.get_future();
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  int i = 0;
  fut.async_get([&i](auto v) -> void{ i = corio::get(v); });
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(i, 0);
  EXPECT_FALSE(ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(i, 42);
  EXPECT_GE(count, 1u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(promise_value, set_value_after_get_future_before_async_get)
{
  using context = boost::asio::io_context;
  context ctx;
  using executor = context::executor_type;
  using promise = corio::thread_unsafe::basic_promise<int, executor>;

  promise prom(ctx);

  using future = corio::basic_future<int, executor>;
  [[maybe_unused]] future fut = prom.get_future();
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  prom.set_value(42);
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  int i = 0;
  fut.async_get([&i](auto v) -> void{ i = corio::get(v); });
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(i, 0);
  EXPECT_FALSE(ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(i, 42);
  EXPECT_GE(count, 1u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(promise_value, set_value_after_async_get)
{
  using context = boost::asio::io_context;
  context ctx;
  using executor = context::executor_type;
  using promise = corio::thread_unsafe::basic_promise<int, executor>;

  promise prom(ctx);

  using future = corio::basic_future<int, executor>;
  [[maybe_unused]] future fut = prom.get_future();
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  int i = 0;
  fut.async_get([&i](auto v) -> void{ i = corio::get(v); });
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(i, 0);
  EXPECT_FALSE(ctx.stopped());

  prom.set_value(42);
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(i, 42);
  EXPECT_GE(count, 1u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(promise_value, set_exception_before_get_future)
{
  using context = boost::asio::io_context;
  context ctx;
  using executor = context::executor_type;
  using promise = corio::thread_unsafe::basic_promise<int, executor>;

  promise prom(ctx);
  prom.set_exception(std::make_exception_ptr(std::runtime_error("Hello, world!")));

  using future = corio::basic_future<int, executor>;
  [[maybe_unused]] future fut = prom.get_future();
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  char const *p = nullptr;
  fut.async_get(
    [&p](auto v) -> void{
      try {
        corio::get(v);
      }
      catch(std::runtime_error const &e)
      {
        p = e.what();
      }
    });
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(p, nullptr);
  EXPECT_FALSE(ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_TRUE(fut.valid());
  EXPECT_STREQ(p, "Hello, world!");
  EXPECT_GE(count, 1u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(promise_value, set_exception_after_get_future_before_async_get)
{
  using context = boost::asio::io_context;
  context ctx;
  using executor = context::executor_type;
  using promise = corio::thread_unsafe::basic_promise<int, executor>;

  promise prom(ctx);

  using future = corio::basic_future<int, executor>;
  [[maybe_unused]] future fut = prom.get_future();
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  prom.set_exception(std::make_exception_ptr(std::runtime_error("Hello, world!")));
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  char const *p = nullptr;
  fut.async_get(
    [&p](auto v) -> void{
      try {
        corio::get(v);
      }
      catch(std::runtime_error const &e)
      {
        p = e.what();
      }
    });
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(p, nullptr);
  EXPECT_FALSE(ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_TRUE(fut.valid());
  EXPECT_STREQ(p, "Hello, world!");
  EXPECT_GE(count, 1u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(promise_value, set_exception_after_async_get)
{
  using context = boost::asio::io_context;
  context ctx;
  using executor = context::executor_type;
  using promise = corio::thread_unsafe::basic_promise<int, executor>;

  promise prom(ctx);

  using future = corio::basic_future<int, executor>;
  [[maybe_unused]] future fut = prom.get_future();
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  char const *p = nullptr;
  fut.async_get(
    [&p](auto v) -> void{
      try {
        corio::get(v);
      }
      catch(std::runtime_error const &e)
      {
        p = e.what();
      }
    });
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(p, nullptr);
  EXPECT_FALSE(ctx.stopped());

  prom.set_exception(std::make_exception_ptr(std::runtime_error("Hello, world!")));
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_TRUE(fut.valid());
  EXPECT_STREQ(p, "Hello, world!");
  EXPECT_GE(count, 1u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(promise_reference, constructor)
{
  using context = boost::asio::io_context;
  context ctx;
  using executor = context::executor_type;
  using promise = corio::thread_unsafe::basic_promise<int &, executor>;
  promise prom(ctx);
}

TEST(promise_reference, get_future)
{
  using context = boost::asio::io_context;
  context ctx;
  using executor = context::executor_type;
  using promise = corio::thread_unsafe::basic_promise<int &, executor>;
  promise prom(ctx);

  using future = corio::basic_future<int &, executor>;
  [[maybe_unused]] future fut = prom.get_future();
  EXPECT_TRUE(fut.valid());
}

TEST(promise_reference, set_value_before_get_future)
{
  using context = boost::asio::io_context;
  context ctx;
  using executor = context::executor_type;
  using promise = corio::thread_unsafe::basic_promise<int &, executor>;

  int i = 42;

  promise prom(ctx);
  prom.set_value(i);

  using future = corio::basic_future<int &, executor>;
  [[maybe_unused]] future fut = prom.get_future();
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  int *p = nullptr;
  fut.async_get([&p](auto v) -> void{ p = &corio::get(v); });
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(p, nullptr);
  EXPECT_FALSE(ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(p, &i);
  EXPECT_GE(count, 1u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(promise_reference, set_value_after_get_future_before_async_get)
{
  using context = boost::asio::io_context;
  context ctx;
  using executor = context::executor_type;
  using promise = corio::thread_unsafe::basic_promise<int &, executor>;

  promise prom(ctx);

  using future = corio::basic_future<int &, executor>;
  [[maybe_unused]] future fut = prom.get_future();
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  int i = 42;

  prom.set_value(i);
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  int *p = nullptr;
  fut.async_get([&p](auto v) -> void{ p = &corio::get(v); });
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(p, nullptr);
  EXPECT_FALSE(ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(p, &i);
  EXPECT_GE(count, 1u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(promise_reference, set_value_after_async_get)
{
  using context = boost::asio::io_context;
  context ctx;
  using executor = context::executor_type;
  using promise = corio::thread_unsafe::basic_promise<int &, executor>;

  promise prom(ctx);

  using future = corio::basic_future<int &, executor>;
  [[maybe_unused]] future fut = prom.get_future();
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  int *p = nullptr;
  fut.async_get([&p](auto v) -> void{ p = &corio::get(v); });
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(p, nullptr);
  EXPECT_FALSE(ctx.stopped());

  int i = 42;
  prom.set_value(i);
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(p, nullptr);
  EXPECT_FALSE(ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(p, &i);
  EXPECT_GE(count, 1u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(promise_reference, set_exception_before_get_future)
{
  using context = boost::asio::io_context;
  context ctx;
  using executor = context::executor_type;
  using promise = corio::thread_unsafe::basic_promise<int &, executor>;

  promise prom(ctx);
  prom.set_exception(std::make_exception_ptr(std::runtime_error("Hello, world!")));

  using future = corio::basic_future<int &, executor>;
  [[maybe_unused]] future fut = prom.get_future();
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  char const *p = nullptr;
  fut.async_get(
    [&p](auto v) -> void{
      try {
        corio::get(v);
      }
      catch(std::runtime_error const &e)
      {
        p = e.what();
      }
    });
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(p, nullptr);
  EXPECT_FALSE(ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_TRUE(fut.valid());
  EXPECT_STREQ(p, "Hello, world!");
  EXPECT_GE(count, 1u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(promise_reference, set_exception_after_get_future_before_async_get)
{
  using context = boost::asio::io_context;
  context ctx;
  using executor = context::executor_type;
  using promise = corio::thread_unsafe::basic_promise<int &, executor>;

  promise prom(ctx);

  using future = corio::basic_future<int &, executor>;
  [[maybe_unused]] future fut = prom.get_future();
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  prom.set_exception(std::make_exception_ptr(std::runtime_error("Hello, world!")));
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  char const *p = nullptr;
  fut.async_get(
    [&p](auto v) -> void{
      try {
        corio::get(v);
      }
      catch(std::runtime_error const &e)
      {
        p = e.what();
      }
    });
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(p, nullptr);
  EXPECT_FALSE(ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_TRUE(fut.valid());
  EXPECT_STREQ(p, "Hello, world!");
  EXPECT_GE(count, 1u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(promise_reference, set_exception_after_async_get)
{
  using context = boost::asio::io_context;
  context ctx;
  using executor = context::executor_type;
  using promise = corio::thread_unsafe::basic_promise<int &, executor>;

  promise prom(ctx);

  using future = corio::basic_future<int &, executor>;
  [[maybe_unused]] future fut = prom.get_future();
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  char const *p = nullptr;
  fut.async_get(
    [&p](auto v) -> void{
      try {
        corio::get(v);
      }
      catch(std::runtime_error const &e)
      {
        p = e.what();
      }
    });
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(p, nullptr);
  EXPECT_FALSE(ctx.stopped());

  prom.set_exception(std::make_exception_ptr(std::runtime_error("Hello, world!")));
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_TRUE(fut.valid());
  EXPECT_STREQ(p, "Hello, world!");
  EXPECT_GE(count, 1u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(promise_void, constructor)
{
  using context = boost::asio::io_context;
  context ctx;
  using executor = context::executor_type;
  using promise = corio::thread_unsafe::basic_promise<void, executor>;
  promise prom(ctx);
}

TEST(promise_void, get_future)
{
  using context = boost::asio::io_context;
  context ctx;
  using executor = context::executor_type;
  using promise = corio::thread_unsafe::basic_promise<void, executor>;
  promise prom(ctx);

  using future = corio::basic_future<void, executor>;
  [[maybe_unused]] future fut = prom.get_future();
  EXPECT_TRUE(fut.valid());
}

TEST(promise_void, set_value_before_get_future)
{
  using context = boost::asio::io_context;
  context ctx;
  using executor = context::executor_type;
  using promise = corio::thread_unsafe::basic_promise<void, executor>;

  promise prom(ctx);
  prom.set_value();

  using future = corio::basic_future<void, executor>;
  [[maybe_unused]] future fut = prom.get_future();
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  int i = 0;
  fut.async_get([&i](auto v) -> void{ corio::get(v); i = 42; });
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(i, 0);
  EXPECT_FALSE(ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(i, 42);
  EXPECT_GE(count, 1u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(promise_void, set_value_after_get_future_before_async_get)
{
  using context = boost::asio::io_context;
  context ctx;
  using executor = context::executor_type;
  using promise = corio::thread_unsafe::basic_promise<void, executor>;

  promise prom(ctx);

  using future = corio::basic_future<void, executor>;
  [[maybe_unused]] future fut = prom.get_future();
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  prom.set_value();
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  int i = 0;
  fut.async_get([&i](auto v) -> void{ corio::get(v); i = 42; });
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(i, 0);
  EXPECT_FALSE(ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(i, 42);
  EXPECT_GE(count, 1u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(promise_void, set_value_after_async_get)
{
  using context = boost::asio::io_context;
  context ctx;
  using executor = context::executor_type;
  using promise = corio::thread_unsafe::basic_promise<void, executor>;

  promise prom(ctx);

  using future = corio::basic_future<void, executor>;
  [[maybe_unused]] future fut = prom.get_future();
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  int i = 0;
  fut.async_get([&i](auto v) -> void{ corio::get(v); i = 42; });
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(i, 0);
  EXPECT_FALSE(ctx.stopped());

  prom.set_value();
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(i, 0);
  EXPECT_FALSE(ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(i, 42);
  EXPECT_GE(count, 1u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(promise_void, set_exception_before_get_future)
{
  using context = boost::asio::io_context;
  context ctx;
  using executor = context::executor_type;
  using promise = corio::thread_unsafe::basic_promise<void, executor>;

  promise prom(ctx);
  prom.set_exception(std::make_exception_ptr(std::runtime_error("Hello, world!")));

  using future = corio::basic_future<void, executor>;
  [[maybe_unused]] future fut = prom.get_future();
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  char const *p = nullptr;
  fut.async_get(
    [&p](auto v) -> void{
      try {
        corio::get(v);
      }
      catch(std::runtime_error const &e)
      {
        p = e.what();
      }
    });
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(p, nullptr);
  EXPECT_FALSE(ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_TRUE(fut.valid());
  EXPECT_STREQ(p, "Hello, world!");
  EXPECT_GE(count, 1u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(promise_void, set_exception_after_get_future_before_async_get)
{
  using context = boost::asio::io_context;
  context ctx;
  using executor = context::executor_type;
  using promise = corio::thread_unsafe::basic_promise<void, executor>;

  promise prom(ctx);

  using future = corio::basic_future<void, executor>;
  [[maybe_unused]] future fut = prom.get_future();
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  prom.set_exception(std::make_exception_ptr(std::runtime_error("Hello, world!")));
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  char const *p = nullptr;
  fut.async_get(
    [&p](auto v) -> void{
      try {
        corio::get(v);
      }
      catch(std::runtime_error const &e)
      {
        p = e.what();
      }
    });
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(p, nullptr);
  EXPECT_FALSE(ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_TRUE(fut.valid());
  EXPECT_STREQ(p, "Hello, world!");
  EXPECT_GE(count, 1u);
  EXPECT_TRUE(ctx.stopped());
}

TEST(promise_void, set_exception_after_async_get)
{
  using context = boost::asio::io_context;
  context ctx;
  using executor = context::executor_type;
  using promise = corio::thread_unsafe::basic_promise<void, executor>;

  promise prom(ctx);

  using future = corio::basic_future<void, executor>;
  [[maybe_unused]] future fut = prom.get_future();
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  char const *p = nullptr;
  fut.async_get(
    [&p](auto v) -> void{
      try {
        corio::get(v);
      }
      catch(std::runtime_error const &e)
      {
        p = e.what();
      }
    });
  EXPECT_TRUE(fut.valid());
  EXPECT_EQ(p, nullptr);
  EXPECT_FALSE(ctx.stopped());

  prom.set_exception(std::make_exception_ptr(std::runtime_error("Hello, world!")));
  EXPECT_TRUE(fut.valid());
  EXPECT_FALSE(ctx.stopped());

  boost::asio::io_context::count_type count = ctx.run();
  EXPECT_TRUE(fut.valid());
  EXPECT_STREQ(p, "Hello, world!");
  EXPECT_GE(count, 1u);
  EXPECT_TRUE(ctx.stopped());
}
