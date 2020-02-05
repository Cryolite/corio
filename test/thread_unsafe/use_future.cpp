#include <corio/thread_unsafe/coroutine.hpp>

#include <gtest/gtest.h>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/io_context.hpp>
#include <type_traits>


namespace corio
{ using namespace thread_unsafe; }

#if 0

namespace{

corio::coroutine<void> test_post(boost::asio::executor const &, int &i, int &j)
{
  i = 42;
  co_await boost::asio::post(CORIO_USE_FUTURE());
  j = 43;
}

} // namespace *unnamed*

TEST(use_future, post)
{
  int i = 0;
  int j = 0;
  boost::asio::io_context context;
  corio::post(test_post(context.get_executor(), i, j));
  context.run();
  EXPECT_EQ(i, 42);
  EXPECT_EQ(j, 43);
}

namespace{

corio::coroutine<void> test_resolve(boost::asio::executor const &)
{
  using resolver_type = boost::asio::ip::tcp::resolver;
  resolver_type resolver(CORIO_THIS_EXECUTOR());
  resolver_type::results_type results
    = co_await resolver.async_resolve("www.google.com", "http", CORIO_USE_FUTURE());
}

} // namespace *unnamed*

TEST(use_future, test_resolve)
{
  boost::asio::io_context context;
  corio::post(test_resolve(context.get_executor()));
  context.run();
}

#endif
