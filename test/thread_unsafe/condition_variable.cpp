#include <corio/thread_unsafe/condition_variable.hpp>

#include <gtest/gtest.h>
#include <boost/asio/io_context.hpp>
#include <condition_variable>
#include <chrono>
#include <cstddef>


namespace corio{ using namespace thread_unsafe; }

namespace{

class condition_variable_empty
  : public ::testing::Test
{
protected:
  condition_variable_empty()
    : context(),
      cv(context),
      i(),
      status(std::cv_status::no_timeout),
      notified()
  {}

  using context_type = boost::asio::io_context;
  context_type context;
  corio::condition_variable cv;
  int i;
  std::cv_status status;
  std::size_t notified;
}; // class condition_variable_empty

} // namespace *unnamed*

TEST_F(condition_variable_empty, run)
{
  auto count = context.run();
  EXPECT_EQ(count, 0u);
}

TEST_F(condition_variable_empty, notify_one)
{
  cv.notify_one();
  auto count = context.run();
  EXPECT_EQ(count, 0u);
}

TEST_F(condition_variable_empty, notify_all)
{
  cv.notify_all();
  auto count = context.run();
  EXPECT_EQ(count, 0u);
}

TEST_F(condition_variable_empty, async_wait)
{
  cv.async_wait([]() -> void{});
  auto count = context.run();
  EXPECT_EQ(count, 0u);
}

TEST_F(condition_variable_empty, async_wait_p0)
{
  cv.async_wait([this]() -> bool{ return i >= 0; }, [this]() -> void{ notified += 1; });
  context.run();
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_empty, async_wait_p1)
{
  cv.async_wait([this]() -> bool{ return i >= 1; }, [this]() -> void{ notified += 1; });
  auto count = context.run();
  EXPECT_EQ(notified, 0u);
  EXPECT_EQ(count, 0u);
}

TEST_F(condition_variable_empty, async_wait_until)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(10),
                      [this](std::cv_status stat) -> void{ status = stat; notified += 1; });
  context.run();
  EXPECT_EQ(status, std::cv_status::timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_empty, async_wait_until_timeout)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this](std::cv_status stat) -> void{ status = stat; notified += 1; });
  context.run();
  EXPECT_EQ(status, std::cv_status::timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_empty, async_wait_until_p0)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(10),
                      [this]() -> bool{ return i >= 0; },
                      [this](std::cv_status stat) -> void{ status = stat; notified += 1; });
  context.run();
  EXPECT_EQ(status, std::cv_status::no_timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_empty, async_wait_until_p1)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(10),
                      [this]() -> bool{ return i >= 1; },
                      [this](std::cv_status stat) -> void{ status = stat; notified += 1; });
  context.run();
  EXPECT_EQ(status, std::cv_status::timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_empty, async_wait_until_timeout_p0)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this]() -> bool{ return i >= 0; },
                      [this](std::cv_status stat) -> void{ status = stat; notified += 1; });
  context.run();
  EXPECT_EQ(status, std::cv_status::no_timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_empty, async_wait_until_timeout_p1)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this]() -> bool{ return i >= 1; },
                      [this](std::cv_status stat) -> void{ status = stat; notified += 1; });
  context.run();
  EXPECT_EQ(status, std::cv_status::timeout);
  EXPECT_EQ(notified, 1u);
}

namespace{

class condition_variable_no
  : public ::testing::Test
{
protected:
  condition_variable_no()
    : context(),
      cv(context)
  {
    cv.notify_one();
  }

  using context_type = boost::asio::io_context;
  context_type context;
  corio::condition_variable cv;
}; // class condition_variable_no

} // namespace *unnamed*

TEST_F(condition_variable_no, run)
{
  auto count = context.run();
  EXPECT_EQ(count, 0u);
}

TEST_F(condition_variable_no, notify_one)
{
  cv.notify_one();
}

TEST_F(condition_variable_no, notify_all)
{
  cv.notify_all();
}

TEST_F(condition_variable_no, async_wait)
{
  cv.async_wait([]() -> void{});
}

TEST_F(condition_variable_no, async_wait_predicate)
{
  cv.async_wait([]() -> bool{ return true; }, []() -> void{});
}

TEST_F(condition_variable_no, async_wait_until)
{
  cv.async_wait_until(std::chrono::system_clock::now(), [](std::cv_status) -> void{});
}

TEST_F(condition_variable_no, async_wait_until_predicate)
{
  cv.async_wait_until(
    std::chrono::system_clock::now(), []() -> bool{ return true; }, [](std::cv_status) -> void{});
}

namespace{

class condition_variable_na
  : public ::testing::Test
{
protected:
  condition_variable_na()
    : context(),
      cv(context)
  {
    cv.notify_all();
  }

  using context_type = boost::asio::io_context;
  context_type context;
  corio::condition_variable cv;
}; // class condition_variable_na

} // namespace *unnamed*

TEST_F(condition_variable_na, run)
{
  auto count = context.run();
  EXPECT_EQ(count, 0u);
}

TEST_F(condition_variable_na, notify_one)
{
  cv.notify_one();
}

TEST_F(condition_variable_na, notify_all)
{
  cv.notify_all();
}

TEST_F(condition_variable_na, async_wait)
{
  cv.async_wait([]() -> void{});
}

TEST_F(condition_variable_na, async_wait_predicate)
{
  cv.async_wait([]() -> bool{ return true; }, []() -> void{});
}

TEST_F(condition_variable_na, async_wait_until)
{
  cv.async_wait_until(std::chrono::system_clock::now(), [](std::cv_status) -> void{});
}

TEST_F(condition_variable_na, async_wait_until_predicate)
{
  cv.async_wait_until(
    std::chrono::system_clock::now(), []() -> bool{ return true; }, [](std::cv_status) -> void{});
}

namespace{

class condition_variable_w
  : public ::testing::Test
{
protected:
  condition_variable_w()
    : context(),
      cv(context),
      num_notified()
  {
    cv.async_wait([this]() -> void{ ++num_notified; });
  }

  using context_type = boost::asio::io_context;
  context_type context;
  corio::condition_variable cv;
  std::size_t num_notified;
}; // class condition_variable_w

} // namespace *unnamed*

TEST_F(condition_variable_w, run)
{
  auto count = context.run();
  EXPECT_EQ(count, 0u);
}

TEST_F(condition_variable_w, notify_one)
{
  cv.notify_one();
  context_type::count_type count = context.run();
  EXPECT_EQ(num_notified, 1u);
  EXPECT_EQ(count, 1u);
}

TEST_F(condition_variable_w, notify_all)
{
  cv.notify_all();
  context_type::count_type count = context.run();
  EXPECT_EQ(num_notified, 1u);
  EXPECT_EQ(count, 1u);
}

TEST_F(condition_variable_w, async_wait)
{
  cv.async_wait([this]() -> void{ ++num_notified; });
}

TEST_F(condition_variable_w, async_wait_predicate)
{
  cv.async_wait([]() -> bool{ return true; }, [this]() -> void{ ++num_notified; });
}

TEST_F(condition_variable_w, async_wait_until)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this](std::cv_status) -> void{ ++num_notified;});
}

TEST_F(condition_variable_w, async_wait_until_predicate)
{
  cv.async_wait_until(std::chrono::system_clock::now(), []() -> bool{ return true;},
                      [this](std::cv_status) -> void{ ++num_notified; });
}

namespace{

class condition_variable_wp0
  : public ::testing::Test
{
protected:
  condition_variable_wp0()
    : context(),
      cv(context),
      i(),
      notified()
  {
    cv.async_wait([this]() -> bool{ return i >= 0; }, [this]() -> void{ notified += 1; });
  }

  using context_type = boost::asio::io_context;
  context_type context;
  corio::condition_variable cv;
  int i;
  std::size_t notified;
}; // class condition_variable_wp0

} // namespace *unnamed*

TEST_F(condition_variable_wp0, run)
{
  auto count = context.run();
  EXPECT_EQ(notified, 1u);
  EXPECT_GE(count, 1u);
}

TEST_F(condition_variable_wp0, notify_one)
{
  ++i;
  cv.notify_one();
  auto count = context.run();
  EXPECT_EQ(notified, 1u);
  EXPECT_GE(count, 1u);
}

TEST_F(condition_variable_wp0, notify_all)
{
  ++i;
  cv.notify_all();
  auto count = context.run();
  EXPECT_EQ(notified, 1u);
  EXPECT_GE(count, 1u);
}

TEST_F(condition_variable_wp0, async_wait)
{
  cv.async_wait([this]() -> void{ notified += 2; });
  auto count = context.run();
  EXPECT_EQ(notified, 1u);
  EXPECT_GE(count, 1u);
}

TEST_F(condition_variable_wp0, async_wait_p0)
{
  cv.async_wait([this]() -> bool{ return i >= 0; }, [this]() -> void{ notified += 2; });
  context.run();
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wp0, async_wait_until)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this](std::cv_status) -> void{ ++notified += 2;});
}

TEST_F(condition_variable_wp0, async_wait_until_predicate)
{
  cv.async_wait_until(std::chrono::system_clock::now(), []() -> bool{ return true;},
                      [this](std::cv_status) -> void{ ++notified += 2; });
}

namespace{

class condition_variable_wp2
  : public ::testing::Test
{
protected:
  condition_variable_wp2()
    : context(),
      cv(context),
      i(),
      num_notified()
  {
    cv.async_wait([this]() -> bool{ return i >= 2; }, [this]() -> void{ ++num_notified; });
  }

  using context_type = boost::asio::io_context;
  context_type context;
  corio::condition_variable cv;
  int i;
  std::size_t num_notified;
}; // class condition_variable_wp2

} // namespace *unnamed*

TEST_F(condition_variable_wp2, run)
{
  auto count = context.run();
  EXPECT_EQ(count, 0u);
}

TEST_F(condition_variable_wp2, notify_one)
{
  ++i;
  cv.notify_one();
  context_type::count_type count = context.run();
  EXPECT_EQ(num_notified, 0u);
  EXPECT_EQ(count, 0u);
}

TEST_F(condition_variable_wp2, notify_all)
{
  ++i;
  cv.notify_all();
  context_type::count_type count = context.run();
  EXPECT_EQ(num_notified, 0u);
  EXPECT_EQ(count, 0u);
}

TEST_F(condition_variable_wp2, async_wait)
{
  cv.async_wait([this]() -> void{ ++num_notified; });
}

TEST_F(condition_variable_wp2, async_wait_predicate)
{
  cv.async_wait([]() -> bool{ return true; }, [this]() -> void{ ++num_notified; });
}

TEST_F(condition_variable_wp2, async_wait_until)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this](std::cv_status) -> void{ ++num_notified;});
}

TEST_F(condition_variable_wp2, async_wait_until_predicate)
{
  cv.async_wait_until(std::chrono::system_clock::now(), []() -> bool{ return true;},
                      [this](std::cv_status) -> void{ ++num_notified; });
}

namespace{

class condition_variable_wu
  : public ::testing::Test
{
protected:
  condition_variable_wu()
    : context(),
      cv(context),
      num_notified()
  {
    cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(100),
                        [this](std::cv_status) -> void{ ++num_notified; });
  }

  using context_type = boost::asio::io_context;
  context_type context;
  corio::condition_variable cv;
  std::size_t num_notified;
}; // class condition_variable_wu

} // namespace *unnamed*

TEST_F(condition_variable_wu, run)
{
  auto count = context.run();
  EXPECT_GT(count, 0u);
}

TEST_F(condition_variable_wu, notify_one)
{
  cv.notify_one();
  context_type::count_type count = context.run();
  EXPECT_EQ(num_notified, 1u);
  EXPECT_GT(count, 1u);
}

TEST_F(condition_variable_wu, notify_all)
{
  cv.notify_all();
  context_type::count_type count = context.run();
  EXPECT_EQ(num_notified, 1u);
  EXPECT_GT(count, 1u);
}

TEST_F(condition_variable_wu, async_wait)
{
  cv.async_wait([this]() -> void{ ++num_notified; });
}

TEST_F(condition_variable_wu, async_wait_predicate)
{
  cv.async_wait([]() -> bool{ return true; }, [this]() -> void{ ++num_notified; });
}

TEST_F(condition_variable_wu, async_wait_until)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this](std::cv_status) -> void{ ++num_notified;});
}

TEST_F(condition_variable_wu, async_wait_until_predicate)
{
  cv.async_wait_until(std::chrono::system_clock::now(), []() -> bool{ return true;},
                      [this](std::cv_status) -> void{ ++num_notified; });
}
