#include <corio/util/throw.hpp>
#include <corio/util/exception.hpp>
#include <boost/exception/get_error_info.hpp>
#include <boost/exception/exception.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <filesystem>
#include <system_error>
#include <stdexcept>


TEST(throw, basic)
{
  try{
    CORIO_THROW<std::runtime_error>("Hello, world!");
  }
  catch (std::runtime_error const &e) {
    EXPECT_STREQ(e.what(), "Hello, world!");
    {
      char const * const *p = boost::get_error_info<boost::throw_file>(e);
      ASSERT_NE(p, nullptr);
      ASSERT_NE(*p, nullptr);
      EXPECT_THAT(*p, testing::ContainsRegex("corio/test/util/throw\\.cpp$"));
    }
    {
      int const *p = boost::get_error_info<boost::throw_line>(e);
      ASSERT_NE(p, nullptr);
      EXPECT_EQ(*p, 15);
    }
    {
      char const * const *p = boost::get_error_info<boost::throw_function>(e);
      ASSERT_NE(p, nullptr);
      ASSERT_NE(*p, nullptr);
    }
    {
      boost::stacktrace::stacktrace const *p = boost::get_error_info<corio::stacktrace_info>(e);
      ASSERT_NE(p, nullptr);
      ASSERT_GT(p->size(), 0);
    }
  }
}

TEST(throw, system_error_stream)
{
  {
    std::error_code error_code = std::make_error_code(std::errc::operation_not_permitted);
    try{
      CORIO_THROW<std::system_error>(error_code) << "Hello, world!";
    }
    catch (std::system_error const &e) {
      EXPECT_EQ(e.code(), error_code);
      std::string what("Hello, world!");
      what += ": ";
      what += error_code.message();
      EXPECT_EQ(e.what(), what);
      {
        char const * const *p = boost::get_error_info<boost::throw_file>(e);
        ASSERT_NE(p, nullptr);
        ASSERT_NE(*p, nullptr);
        EXPECT_THAT(*p, testing::ContainsRegex("corio/test/util/throw\\.cpp$"));
      }
      {
        int const *p = boost::get_error_info<boost::throw_line>(e);
        ASSERT_NE(p, nullptr);
        EXPECT_EQ(*p, 48);
      }
      {
        char const * const *p = boost::get_error_info<boost::throw_function>(e);
        ASSERT_NE(p, nullptr);
        ASSERT_NE(*p, nullptr);
      }
      {
        boost::stacktrace::stacktrace const *p = boost::get_error_info<corio::stacktrace_info>(e);
        ASSERT_NE(p, nullptr);
        EXPECT_GT(p->size(), 0u);
      }
    }
  }
  {
    try{
      CORIO_THROW<std::system_error>(0, std::system_category()) << "Hello, world!";
    }
    catch (std::system_error const &e) {
      std::error_code ec(0, std::system_category());
      EXPECT_EQ(e.code(), ec);
      EXPECT_THAT(e.what(), testing::ContainsRegex("^Hello, world!"));
      {
        char const * const *p = boost::get_error_info<boost::throw_file>(e);
        ASSERT_NE(p, nullptr);
        ASSERT_NE(*p, nullptr);
        EXPECT_THAT(*p, testing::ContainsRegex("corio/test/util/throw\\.cpp$"));
      }
      {
        int const *p = boost::get_error_info<boost::throw_line>(e);
        ASSERT_NE(p, nullptr);
        EXPECT_EQ(*p, 81);
      }
      {
        char const * const *p = boost::get_error_info<boost::throw_function>(e);
        ASSERT_NE(p, nullptr);
        ASSERT_NE(*p, nullptr);
      }
      {
        boost::stacktrace::stacktrace const *p = boost::get_error_info<corio::stacktrace_info>(e);
        ASSERT_NE(p, nullptr);
        EXPECT_GT(p->size(), 0u);
      }
    }
  }
}

TEST(throw, filesystem_error_stream)
{
  {
    std::error_code ec = std::make_error_code(std::errc::operation_not_permitted);
    try{
      CORIO_THROW<std::filesystem::filesystem_error>(ec) << "Hello, world!";
    }
    catch (std::filesystem::filesystem_error const &e) {
      EXPECT_THAT(e.what(), testing::HasSubstr("Hello, world!"));
      {
        char const * const *p = boost::get_error_info<boost::throw_file>(e);
        ASSERT_NE(p, nullptr);
        ASSERT_NE(*p, nullptr);
        EXPECT_THAT(*p, testing::ContainsRegex("corio/test/util/throw\\.cpp$"));
      }
      {
        int const *p = boost::get_error_info<boost::throw_line>(e);
        ASSERT_NE(p, nullptr);
        EXPECT_EQ(*p, 117);
      }
      {
        char const * const *p = boost::get_error_info<boost::throw_function>(e);
        ASSERT_NE(p, nullptr);
        ASSERT_NE(*p, nullptr);
      }
      {
        boost::stacktrace::stacktrace const *p = boost::get_error_info<corio::stacktrace_info>(e);
        ASSERT_NE(p, nullptr);
        EXPECT_GT(p->size(), 0u);
      }
    }
  }
}

TEST(throw, stream_manip_ostream)
{
  try{
    CORIO_THROW<std::runtime_error>() << std::endl << "Hello, world!";
  }
  catch (std::runtime_error &e) {
    EXPECT_STREQ(e.what(), "\nHello, world!");
    {
      char const * const *p = boost::get_error_info<boost::throw_file>(e);
      ASSERT_NE(p, nullptr);
      ASSERT_NE(*p, nullptr);
      EXPECT_THAT(*p, testing::ContainsRegex("corio/test/util/throw\\.cpp$"));
    }
    {
      int const *p = boost::get_error_info<boost::throw_line>(e);
      ASSERT_NE(p, nullptr);
      EXPECT_EQ(*p, 149);
    }
    {
      char const * const *p = boost::get_error_info<boost::throw_function>(e);
      ASSERT_NE(p, nullptr);
      EXPECT_NE(*p, nullptr);
    }
    {
      boost::stacktrace::stacktrace const *p = boost::get_error_info<corio::stacktrace_info>(e);
      ASSERT_NE(p, nullptr);
      EXPECT_GT(p->size(), 0u);
    }
  }
}

TEST(throw, stream_manip_ios_base)
{
  try{
    CORIO_THROW<std::runtime_error>() << "Hello, world!: " << std::boolalpha << true;
  }
  catch (std::runtime_error const &e) {
    EXPECT_STREQ(e.what(), "Hello, world!: true");
    {
      char const * const *p = boost::get_error_info<boost::throw_file>(e);
      ASSERT_NE(p, nullptr);
      ASSERT_NE(*p, nullptr);
      EXPECT_THAT(*p, testing::ContainsRegex("corio/test/util/throw\\.cpp$"));
    }
    {
      int const *p = boost::get_error_info<boost::throw_line>(e);
      ASSERT_NE(p, nullptr);
      EXPECT_EQ(*p, 180);
    }
    {
      char const * const *p = boost::get_error_info<boost::throw_function>(e);
      ASSERT_NE(p, nullptr);
      EXPECT_NE(*p, nullptr);
    }
    {
      boost::stacktrace::stacktrace const *p = boost::get_error_info<corio::stacktrace_info>(e);
      ASSERT_NE(p, nullptr);
      EXPECT_GT(p->size(), 0u);
    }
  }
}

TEST(throw, terminate)
{
  EXPECT_DEATH({
      []() noexcept -> void{
        CORIO_THROW<std::runtime_error>("Hello, world!");
      }();
    }, "^Program terminates due to an uncaught exception\\.\n"
    ".*corio/test/util/throw\\.cpp:212: .*: .*std::runtime_error.*: Hello, world!\n"
    "Stack trace:\n  ");
}

namespace{

void raw_throw_()
{
  throw std::runtime_error("Hello, world!");
}

} // namespace *unnamed*

TEST(throw, terminate_raw_throw)
{
  EXPECT_DEATH({
      []() noexcept -> void{
        raw_throw_();
      }();
    }, "^Program terminates due to an uncaught exception\\.\n"
    ".*std::runtime_error.*: Hello, world!$");
}

namespace{

void throw_nested()
{
  try{
    CORIO_THROW<std::runtime_error>("A nested exception.");
  }
  catch (...) {
    std::error_code ec = std::make_error_code(std::errc::operation_not_permitted);
    CORIO_THROW<std::system_error>(ec, "Hello, world!");
  }
}

TEST(throw, terminate_nested)
{
  EXPECT_DEATH({
      []() noexcept -> void{
        throw_nested();
      }();
    }, "^Program terminates due to an uncaught exception\\.\n"
    ".*corio/test/util/throw\\.cpp:247: throw_nested: .*: Hello, world!: Operation not permitted\n"
    "Stack trace:(\n  .*)*\n"
    "Nested exception:\n"
    "  .*corio/test/util/throw\\.cpp:243: throw_nested: .*: A nested exception\\.\n"
    "  Stack trace:(\n    .*)*");
}

} // namespace *unnamed*
