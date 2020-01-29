#include <corio/thread_unsafe/condition_variable.hpp>

#include <gtest/gtest.h>
#include <boost/asio/io_context.hpp>
#include <condition_variable>
#include <chrono>
#include <cstddef>


namespace corio{ using namespace thread_unsafe; }

namespace{

class condition_variable
  : public ::testing::Test
{
protected:
  condition_variable()
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
}; // class condition_variable

} // namespace *unnamed*

TEST_F(condition_variable, run)
{
  auto count = context.run();
  EXPECT_EQ(count, 0u);
}

TEST_F(condition_variable, notify_one)
{
  cv.notify_one();
  auto count = context.run();
  EXPECT_EQ(count, 0u);
}

TEST_F(condition_variable, notify_all)
{
  cv.notify_all();
  auto count = context.run();
  EXPECT_EQ(count, 0u);
}

TEST_F(condition_variable, async_wait)
{
  cv.async_wait([]() -> void{});
  auto count = context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(count, 0u);
}

TEST_F(condition_variable, async_wait_p0)
{
  cv.async_wait([this]() -> bool{ return i >= 0; }, [this]() -> void{ notified += 1; });
  context.run();
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable, async_wait_p1)
{
  cv.async_wait([this]() -> bool{ return i >= 1; }, [this]() -> void{ notified += 1; });
  auto count = context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(notified, 0u);
  EXPECT_EQ(count, 0u);
}

TEST_F(condition_variable, async_wait_until)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                      [this](std::cv_status stat) -> void{ status = stat; notified += 1; });
  context.run();
  EXPECT_EQ(status, std::cv_status::timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable, async_wait_until_timeout)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this](std::cv_status stat) -> void{ status = stat; notified += 1; });
  context.run();
  EXPECT_EQ(status, std::cv_status::timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable, async_wait_until_p0)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                      [this]() -> bool{ return i >= 0; },
                      [this](std::cv_status stat) -> void{ status = stat; notified += 1; });
  context.run();
  EXPECT_EQ(status, std::cv_status::no_timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable, async_wait_until_p1)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                      [this]() -> bool{ return i >= 1; },
                      [this](std::cv_status stat) -> void{ status = stat; notified += 1; });
  context.run();
  EXPECT_EQ(status, std::cv_status::timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable, async_wait_until_timeout_p0)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this]() -> bool{ return i >= 0; },
                      [this](std::cv_status stat) -> void{ status = stat; notified += 1; });
  context.run();
  EXPECT_EQ(status, std::cv_status::no_timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable, async_wait_until_timeout_p1)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this]() -> bool{ return i >= 1; },
                      [this](std::cv_status stat) -> void{ status = stat; notified += 1; });
  context.run();
  EXPECT_EQ(status, std::cv_status::timeout);
  EXPECT_EQ(notified, 1u);
}

//=============================================================================
//=============================================================================
//=============================================================================

namespace{

class condition_variable_w
  : public ::testing::Test
{
protected:
  condition_variable_w()
    : context(),
      cv(context),
      i(),
      status(),
      notified()
  {
    cv.async_wait([this]() -> void{ notified += 1; });
  }

  using context_type = boost::asio::io_context;
  context_type context;
  corio::condition_variable cv;
  int i;
  std::cv_status status;
  std::size_t notified;
}; // class condition_variable_w

} // namespace *unnamed*

TEST_F(condition_variable_w, notify_one)
{
  cv.notify_one();
  context.run();
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_w, notify_all)
{
  cv.notify_all();
  context.run();
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_w, async_wait)
{
  cv.async_wait([this]() -> void{ notified += 2; });
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(notified, 0u);
}

TEST_F(condition_variable_w, async_wait_p0)
{
  cv.async_wait([this]() -> bool{ return i >= 0; }, [this]() -> void{ notified += 2; });
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(notified, 2u);
}

TEST_F(condition_variable_w, async_wait_p1)
{
  cv.async_wait([this]() -> bool{ return i >= 1; }, [this]() -> void{ notified += 2; });
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(notified, 0u);
}

TEST_F(condition_variable_w, async_wait_until)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                      [this](std::cv_status s) -> void{ status = s; notified += 2; });
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(status, std::cv_status::timeout);
  EXPECT_EQ(notified, 2u);
}

TEST_F(condition_variable_w, async_wait_until_p0)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                      [this]() -> bool{ return i >= 0; },
                      [this](std::cv_status s) -> void{ status = s; notified += 2; });
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(status, std::cv_status::no_timeout);
  EXPECT_EQ(notified, 2u);
}

TEST_F(condition_variable_w, async_wait_until_p1)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                      [this]() -> bool{ return i >= 1; },
                      [this](std::cv_status s) -> void{ status = s; notified += 2; });
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(status, std::cv_status::timeout);
  EXPECT_EQ(notified, 2u);
}

TEST_F(condition_variable_w, async_wait_until_timeout)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this](std::cv_status s) -> void{ status = s; notified += 2; });
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(status, std::cv_status::timeout);
  EXPECT_EQ(notified, 2u);
}

TEST_F(condition_variable_w, async_wait_until_timeout_p0)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this]() -> bool{ return i >= 0; },
                      [this](std::cv_status s) -> void{ status = s; notified += 2; });
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(status, std::cv_status::no_timeout);
  EXPECT_EQ(notified, 2u);
}

TEST_F(condition_variable_w, async_wait_until_timeout_p1)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this]() -> bool{ return i >= 1; },
                      [this](std::cv_status s) -> void{ status = s; notified += 2; });
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(status, std::cv_status::timeout);
  EXPECT_EQ(notified, 2u);
}

//=============================================================================
//=============================================================================
//=============================================================================

namespace{

class condition_variable_wp0
  : public ::testing::Test
{
protected:
  condition_variable_wp0()
    : context(),
      cv(context),
      i(),
      status(std::cv_status::no_timeout),
      notified()
  {
    cv.async_wait([this]() -> bool{ return i >= 0; }, [this]() -> void{ notified += 1; });
  }

  using context_type = boost::asio::io_context;
  context_type context;
  corio::condition_variable cv;
  int i;
  std::cv_status status;
  std::size_t notified;
}; // class condition_variable_wp0

} // namespace *unnamed*

TEST_F(condition_variable_wp0, notify_one)
{
  cv.notify_one();
  context.run();
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_wp0, notify_all)
{
  cv.notify_all();
  context.run();
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_wp0, async_wait)
{
  cv.async_wait([this]() -> void{ notified += 2; });
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_wp0, async_wait_p0)
{
  cv.async_wait([this]() -> bool{ return i >= 0; }, [this]() -> void{ notified += 2; });
  context.run();
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wp0, async_wait_p1)
{
  cv.async_wait([this]() -> bool{ return i >= 1; }, [this]() -> void{ notified += 2; });
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_wp0, async_wait_until)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(10),
                      [this](std::cv_status s) -> void{ status = s; notified += 2;});
  context.run();
  EXPECT_EQ(status, std::cv_status::timeout);
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wp0, async_wait_until_p0)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(10),
                      [this]() -> bool { return i >= 0; },
                      [this](std::cv_status s) -> void{ status = s; notified += 2;});
  context.run();
  EXPECT_EQ(status, std::cv_status::no_timeout);
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wp0, async_wait_until_p1)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(10),
                      [this]() -> bool { return i >= 1; },
                      [this](std::cv_status s) -> void{ status = s; notified += 2;});
  context.run();
  EXPECT_EQ(status, std::cv_status::timeout);
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wp0, async_wait_until_timeout)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this](std::cv_status s) -> void{ status = s; notified += 2;});
  context.run();
  EXPECT_EQ(status, std::cv_status::timeout);
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wp0, async_wait_until_timeout_p0)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this]() -> bool { return i >= 0; },
                      [this](std::cv_status s) -> void{ status = s; notified += 2;});
  context.run();
  EXPECT_EQ(status, std::cv_status::no_timeout);
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wp0, async_wait_until_timeout_p1)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(10),
                      [this]() -> bool { return i >= 1; },
                      [this](std::cv_status s) -> void{ status = s; notified += 2;});
  context.run();
  EXPECT_EQ(status, std::cv_status::timeout);
  EXPECT_EQ(notified, 3u);
}

//=============================================================================
//=============================================================================
//=============================================================================

namespace{

class condition_variable_wp1
  : public ::testing::Test
{
protected:
  condition_variable_wp1()
    : context(),
      cv(context),
      i(),
      status(),
      notified()
  {
    cv.async_wait([this]() -> bool{ return i >= 1; }, [this]() -> void{ notified += 1; });
  }

  using context_type = boost::asio::io_context;
  context_type context;
  corio::condition_variable cv;
  int i;
  std::cv_status status;
  std::size_t notified;
}; // class condition_variable_wp1

} // namespace *unnamed*

TEST_F(condition_variable_wp1, run)
{
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(notified, 0u);
}

TEST_F(condition_variable_wp1, notify_one)
{
  cv.notify_one();
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(notified, 0u);
}

TEST_F(condition_variable_wp1, notify_all)
{
  cv.notify_all();
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(notified, 0u);
}

TEST_F(condition_variable_wp1, async_wait)
{
  cv.async_wait([this]() -> void{ notified += 2; });
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(notified, 0u);
}

TEST_F(condition_variable_wp1, async_wait_p0)
{
  cv.async_wait([this]() -> bool{ return i >= 0; }, [this]() -> void{ notified += 2; });
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(notified, 2u);
}

TEST_F(condition_variable_wp1, async_wait_p1)
{
  cv.async_wait([this]() -> bool{ return i >= 1; }, [this]() -> void{ notified += 2; });
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(notified, 0u);
}

TEST_F(condition_variable_wp1, async_wait_until)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                      [this](std::cv_status s) -> void{ status = s; notified += 2;});
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(status, std::cv_status::timeout);
  EXPECT_EQ(notified, 2u);
}

TEST_F(condition_variable_wp1, async_wait_until_p0)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                      [this]() -> bool{ return i >= 0; },
                      [this](std::cv_status s) -> void{ status = s; notified += 2; });
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(status, std::cv_status::no_timeout);
  EXPECT_EQ(notified, 2u);
}

TEST_F(condition_variable_wp1, async_wait_until_p1)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                      [this]() -> bool{ return i >= 1; },
                      [this](std::cv_status s) -> void{ status = s; notified += 2; });
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(status, std::cv_status::timeout);
  EXPECT_EQ(notified, 2u);
}

TEST_F(condition_variable_wp1, async_wait_until_timeout)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this](std::cv_status s) -> void{ status = s; notified += 2;});
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(status, std::cv_status::timeout);
  EXPECT_EQ(notified, 2u);
}

TEST_F(condition_variable_wp1, async_wait_until_timeout_p0)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this]() -> bool{ return i >= 0; },
                      [this](std::cv_status s) -> void{ status = s; notified += 2; });
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(status, std::cv_status::no_timeout);
  EXPECT_EQ(notified, 2u);
}

TEST_F(condition_variable_wp1, async_wait_until_timeout_p1)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this]() -> bool{ return i >= 1; },
                      [this](std::cv_status s) -> void{ status = s; notified += 2; });
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(status, std::cv_status::timeout);
  EXPECT_EQ(notified, 2u);
}

//=============================================================================
//=============================================================================
//=============================================================================

namespace{

class condition_variable_wu
  : public ::testing::Test
{
protected:
  condition_variable_wu()
    : context(),
      cv(context),
      i(),
      status0(),
      status1(),
      notified()
  {
    cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                        [this](std::cv_status s) -> void{ status0 = s; notified += 1; });
  }

  using context_type = boost::asio::io_context;
  context_type context;
  corio::condition_variable cv;
  int i;
  std::cv_status status0;
  std::cv_status status1;
  std::size_t notified;
}; // class condition_variable_wu

} // namespace *unnamed*

TEST_F(condition_variable_wu, run)
{
  context.run();
  EXPECT_EQ(status0, std::cv_status::timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_wu, notify_one)
{
  cv.notify_one();
  context.run();
  EXPECT_EQ(status0, std::cv_status::no_timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_wu, notify_all)
{
  cv.notify_all();
  context.run();
  EXPECT_EQ(status0, std::cv_status::no_timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_wu, async_wait)
{
  cv.async_wait([this]() -> void{ notified += 2; });
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(status0, std::cv_status::timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_wu, async_wait_p0)
{
  cv.async_wait([this]() -> bool{ return i >= 0; }, [this]() -> void{ notified += 2; });
  context.run();
  EXPECT_EQ(status0, std::cv_status::timeout);
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wu, async_wait_p1)
{
  cv.async_wait([this]() -> bool{ return i >= 1; }, [this]() -> void{ notified += 2; });
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(status0, std::cv_status::timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_wu, async_wait_until)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                      [this](std::cv_status s) -> void{ status1 = s; notified += 2; });
  context.run();
  EXPECT_EQ(status0, std::cv_status::timeout);
  EXPECT_EQ(status1, std::cv_status::timeout);
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wu, async_wait_until_p0)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                      [this]() -> bool{ return i >= 0; },
                      [this](std::cv_status s) -> void{ status1 = s; notified += 2; });
  context.run();
  EXPECT_EQ(status0, std::cv_status::timeout);
  EXPECT_EQ(status1, std::cv_status::no_timeout);
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wu, async_wait_until_p1)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                      [this]() -> bool{ return i >= 1; },
                      [this](std::cv_status s) -> void{ status1 = s; notified += 2; });
  context.run();
  EXPECT_EQ(status0, std::cv_status::timeout);
  EXPECT_EQ(status1, std::cv_status::timeout);
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wu, async_wait_until_timeout)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this](std::cv_status s) -> void{ status1 = s; notified += 2; });
  context.run();
  EXPECT_EQ(status0, std::cv_status::timeout);
  EXPECT_EQ(status1, std::cv_status::timeout);
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wu, async_wait_until_timeout_p0)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this]() -> bool{ return i >= 0; },
                      [this](std::cv_status s) -> void{ status1 = s; notified += 2; });
  context.run();
  EXPECT_EQ(status0, std::cv_status::timeout);
  EXPECT_EQ(status1, std::cv_status::no_timeout);
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wu, async_wait_until_timeout_p1)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this]() -> bool{ return i >= 1; },
                      [this](std::cv_status s) -> void{ status1 = s; notified += 2; });
  context.run();
  EXPECT_EQ(status0, std::cv_status::timeout);
  EXPECT_EQ(status1, std::cv_status::timeout);
  EXPECT_EQ(notified, 3u);
}

//=============================================================================
//=============================================================================
//=============================================================================

namespace{

class condition_variable_wup0
  : public ::testing::Test
{
protected:
  condition_variable_wup0()
    : context(),
      cv(context),
      i(),
      status0(),
      status1(),
      notified()
  {
    cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                        [this]() -> bool { return i >= 0; },
                        [this](std::cv_status s) -> void{ status0 = s; notified += 1; });
  }

  using context_type = boost::asio::io_context;
  context_type context;
  corio::condition_variable cv;
  int i;
  std::cv_status status0;
  std::cv_status status1;
  std::size_t notified;
}; // class condition_variable_wup0

} // namespace *unnamed*

TEST_F(condition_variable_wup0, run)
{
  context.run();
  EXPECT_EQ(status0, std::cv_status::no_timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_wup0, notify_one)
{
  cv.notify_one();
  context.run();
  EXPECT_EQ(status0, std::cv_status::no_timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_wup0, notify_all)
{
  cv.notify_all();
  context.run();
  EXPECT_EQ(status0, std::cv_status::no_timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_wup0, async_wait)
{
  cv.async_wait([this]() -> void{ notified += 2; });
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(status0, std::cv_status::no_timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_wup0, async_wait_p0)
{
  cv.async_wait([this]() -> bool{ return i >= 0; }, [this]() -> void{ notified += 2; });
  context.run();
  EXPECT_EQ(status0, std::cv_status::no_timeout);
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wup0, async_wait_p1)
{
  cv.async_wait([this]() -> bool{ return i >= 1; }, [this]() -> void{ notified += 2; });
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(status0, std::cv_status::no_timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_wup0, async_wait_until)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                      [this](std::cv_status s) -> void{ status1 = s; notified += 2; });
  context.run();
  EXPECT_EQ(status0, std::cv_status::no_timeout);
  EXPECT_EQ(status1, std::cv_status::timeout);
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wup0, async_wait_until_p0)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                      [this]() -> bool{ return i >= 0; },
                      [this](std::cv_status s) -> void{ status1 = s; notified += 2; });
  context.run();
  EXPECT_EQ(status0, std::cv_status::no_timeout);
  EXPECT_EQ(status1, std::cv_status::no_timeout);
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wup0, async_wait_until_p1)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                      [this]() -> bool{ return i >= 1; },
                      [this](std::cv_status s) -> void{ status1 = s; notified += 2; });
  context.run();
  EXPECT_EQ(status0, std::cv_status::no_timeout);
  EXPECT_EQ(status1, std::cv_status::timeout);
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wup0, async_wait_until_timeout)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this](std::cv_status s) -> void{ status1 = s; notified += 2; });
  context.run();
  EXPECT_EQ(status0, std::cv_status::no_timeout);
  EXPECT_EQ(status1, std::cv_status::timeout);
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wup0, async_wait_until_timeout_p0)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this]() -> bool{ return i >= 0; },
                      [this](std::cv_status s) -> void{ status1 = s; notified += 2; });
  context.run();
  EXPECT_EQ(status0, std::cv_status::no_timeout);
  EXPECT_EQ(status1, std::cv_status::no_timeout);
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wup0, async_wait_until_timeout_p1)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this]() -> bool{ return i >= 1; },
                      [this](std::cv_status s) -> void{ status1 = s; notified += 2; });
  context.run();
  EXPECT_EQ(status0, std::cv_status::no_timeout);
  EXPECT_EQ(status1, std::cv_status::timeout);
  EXPECT_EQ(notified, 3u);
}

//=============================================================================
//=============================================================================
//=============================================================================

namespace{

class condition_variable_wup1
  : public ::testing::Test
{
protected:
  condition_variable_wup1()
    : context(),
      cv(context),
      i(),
      status0(),
      status1(),
      notified()
  {
    cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                        [this]() -> bool { return i >= 1; },
                        [this](std::cv_status s) -> void{ status0 = s; notified += 1; });
  }

  using context_type = boost::asio::io_context;
  context_type context;
  corio::condition_variable cv;
  int i;
  std::cv_status status0;
  std::cv_status status1;
  std::size_t notified;
}; // class condition_variable_wup1

} // namespace *unnamed*

TEST_F(condition_variable_wup1, run)
{
  context.run();
  EXPECT_EQ(status0, std::cv_status::timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_wup1, notify_one)
{
  cv.notify_one();
  context.run();
  EXPECT_EQ(status0, std::cv_status::timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_wup1, notify_all)
{
  cv.notify_all();
  context.run();
  EXPECT_EQ(status0, std::cv_status::timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_wup1, async_wait)
{
  cv.async_wait([this]() -> void{ notified += 2; });
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(status0, std::cv_status::timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_wup1, async_wait_p0)
{
  cv.async_wait([this]() -> bool{ return i >= 0; }, [this]() -> void{ notified += 2; });
  context.run();
  EXPECT_EQ(status0, std::cv_status::timeout);
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wup1, async_wait_p1)
{
  cv.async_wait([this]() -> bool{ return i >= 1; }, [this]() -> void{ notified += 2; });
  context.run_for(std::chrono::milliseconds(10));
  EXPECT_EQ(status0, std::cv_status::timeout);
  EXPECT_EQ(notified, 1u);
}

TEST_F(condition_variable_wup1, async_wait_until)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                      [this](std::cv_status s) -> void{ status1 = s; notified += 2; });
  context.run();
  EXPECT_EQ(status0, std::cv_status::timeout);
  EXPECT_EQ(status1, std::cv_status::timeout);
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wup1, async_wait_until_p0)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                      [this]() -> bool{ return i >= 0; },
                      [this](std::cv_status s) -> void{ status1 = s; notified += 2; });
  context.run();
  EXPECT_EQ(status0, std::cv_status::timeout);
  EXPECT_EQ(status1, std::cv_status::no_timeout);
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wup1, async_wait_until_p1)
{
  cv.async_wait_until(std::chrono::system_clock::now() + std::chrono::milliseconds(1),
                      [this]() -> bool{ return i >= 1; },
                      [this](std::cv_status s) -> void{ status1 = s; notified += 2; });
  context.run();
  EXPECT_EQ(status0, std::cv_status::timeout);
  EXPECT_EQ(status1, std::cv_status::timeout);
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wup1, async_wait_until_timeout)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this](std::cv_status s) -> void{ status1 = s; notified += 2; });
  context.run();
  EXPECT_EQ(status0, std::cv_status::timeout);
  EXPECT_EQ(status1, std::cv_status::timeout);
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wup1, async_wait_until_timeout_p0)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this]() -> bool{ return i >= 0; },
                      [this](std::cv_status s) -> void{ status1 = s; notified += 2; });
  context.run();
  EXPECT_EQ(status0, std::cv_status::timeout);
  EXPECT_EQ(status1, std::cv_status::no_timeout);
  EXPECT_EQ(notified, 3u);
}

TEST_F(condition_variable_wup1, async_wait_until_timeout_p1)
{
  cv.async_wait_until(std::chrono::system_clock::now(),
                      [this]() -> bool{ return i >= 1; },
                      [this](std::cv_status s) -> void{ status1 = s; notified += 2; });
  context.run();
  EXPECT_EQ(status0, std::cv_status::timeout);
  EXPECT_EQ(status1, std::cv_status::timeout);
  EXPECT_EQ(notified, 3u);
}
