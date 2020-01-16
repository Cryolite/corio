#include <corio/thread_unsafe/promise.hpp>

#include <gtest/gtest.h>
#include <boost/asio/io_context.hpp>


namespace corio
{ using namespace thread_unsafe; }

TEST(promise, normal_constructor)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using promise = corio::thread_unsafe::basic_promise<int, context_type>;
  promise p(ctx);
}

TEST(promise, normal_get_future)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using promise = corio::thread_unsafe::basic_promise<int, context_type>;
  promise p(ctx);
  using future = corio::basic_future<int, context_type>;
  [[maybe_unused]] future fut = p.get_future();
}

TEST(promise, normal_set_value)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using promise = corio::thread_unsafe::basic_promise<int, context_type>;
  promise p(ctx);
  p.set_value(42);
}

TEST(promise, void_constructor)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using promise = corio::thread_unsafe::basic_promise<void, context_type>;
  promise p(ctx);
}

TEST(promise, void_get_future)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using promise = corio::thread_unsafe::basic_promise<void, context_type>;
  promise p(ctx);
  using future = corio::basic_future<void, context_type>;
  [[maybe_unused]] future fut = p.get_future();
}

TEST(promise, void_set_value)
{
  using context_type = boost::asio::io_context;
  context_type ctx;
  using promise = corio::thread_unsafe::basic_promise<void, context_type>;
  promise p(ctx);
  p.set_value();
}
