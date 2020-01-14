#include <corio/util/assert.hpp>

#include <gtest/gtest.h>


namespace{

template<typename Char, typename Traits>
std::basic_ios<Char, Traits> &ios_manip(std::basic_ios<Char, Traits> &x) noexcept
{
  return x;
}

} // namespace *unnamed*

TEST(corio_test, assert_expr_pass)
{
  CORIO_ASSERT(true);
  EXPECT_TRUE(true);
}

TEST(corio_test, assert_expr_pass_stream)
{
  CORIO_ASSERT(true) << 42;
  EXPECT_TRUE(true);
}

TEST(corio_test, assert_expr_pass_stream_manip_ostream)
{
  CORIO_ASSERT(true) << std::endl;
  EXPECT_TRUE(true);
}

TEST(corio_test, assert_expr_pass_stream_manip_ios)
{
  CORIO_ASSERT(true) << ios_manip;
  EXPECT_TRUE(true);
}

TEST(corio_test, assert_expr_pass_stream_manip_ios_base)
{
  CORIO_ASSERT(true) << std::boolalpha;
  EXPECT_TRUE(true);
}

TEST(corio_test, assert_expression_pass_unparenthesized_comma)
{
  CORIO_ASSERT(std::pair<int, int>(42, 43).first == 42);
  EXPECT_TRUE(true);
}

TEST(corio_test, assert_expr_fail)
{
#if defined(CORIO_ENABLE_ASSERT)
  EXPECT_DEATH({
      CORIO_ASSERT(false);
    }, "corio/test/util/assert\\.cpp:56: TestBody: Assertion `false' failed\\.");
#endif // defined(CORIO_ENABLE_ASSERT)
}

TEST(corio_test, assert_expr_fail_stream)
{
#if defined(CORIO_ENABLE_ASSERT)
  EXPECT_DEATH({
      CORIO_ASSERT(false) << 42;
    }, "corio/test/util/assert\\.cpp:65: TestBody: Assertion `false' failed\\.\n"
    "Message: 42");
#endif // defined(CORIO_ENABLE_ASSERT)
}

TEST(corio_test, assert_expr_fail_stream_manip_ostream)
{
#if defined(CORIO_ENABLE_ASSERT)
  EXPECT_DEATH({
      CORIO_ASSERT(false) << std::endl << 42;
    }, "corio/test/util/assert\\.cpp:75: TestBody: Assertion `false' failed\\.\n"
    "Message: \n42");
#endif // defined(CORIO_ENABLE_ASSERT)
}

TEST(corio_test, assert_expr_fail_stream_manip_ios)
{
#if defined(CORIO_ENABLE_ASSERT)
  EXPECT_DEATH({
      CORIO_ASSERT(false) << ios_manip;
    }, "corio/test/util/assert\\.cpp:85: TestBody: Assertion `false' failed\\.\n");
#endif // defined(CORIO_ENABLE_ASSERT)
}

TEST(corio_test, assert_expr_fail_stream_manip_ios_base)
{
#if defined(CORIO_ENABLE_ASSERT)
  EXPECT_DEATH({
      CORIO_ASSERT(false) << std::boolalpha << true;
    }, "corio/test/util/assert\\.cpp:94: TestBody: Assertion `false' failed\\.\n"
    "Message: true\n");
#endif // defined(CORIO_ENABLE_ASSERT)
}

TEST(corio_test, assert_expr_fail_unparenthesized_comma)
{
#if defined(CORIO_ENABLE_ASSERT)
  EXPECT_DEATH({
      CORIO_ASSERT(std::pair<int, int>(42, 43).first == 43);
    }, "corio/test/util/assert\\.cpp:104: TestBody: Assertion"
    " `std::pair<int, int>\\(42, 43\\)\\.first == 43' failed\\.\n");
#endif // defined(CORIO_ENABLE_ASSERT)
}

TEST(corio_test, assert_block_pass)
{
  CORIO_ASSERT(){
    return true;
  };
  EXPECT_TRUE(true);
}

TEST(corio_test, assert_block_pass_stream)
{
  CORIO_ASSERT(){
    return true;
  } << 42;
  EXPECT_TRUE(true);
}

TEST(corio_test, assert_block_pass_stream_manip_ostream)
{
  CORIO_ASSERT(){
    return true;
  } << std::endl << 42;
  EXPECT_TRUE(true);
}

TEST(corio_test, assert_block_pass_stream_manip_ios)
{
  CORIO_ASSERT(){
    return true;
  } << ios_manip << 42;
  EXPECT_TRUE(true);
}

TEST(corio_test, assert_block_pass_stream_manip_ios_base)
{
  CORIO_ASSERT(){
    return true;
  } << std::boolalpha;
  EXPECT_TRUE(true);
}

TEST(corio_test, assert_block_fail)
{
#if defined(CORIO_ENABLE_ASSERT)
  EXPECT_DEATH({
      CORIO_ASSERT(){
        return false;
      };
    }, "corio/test/util/assert\\.cpp:154: TestBody: Assertion failed\\.");
#endif // defined(CORIO_ENABLE_ASSERT)
}

TEST(corio_test, assert_block_fail_stream)
{
#if defined(CORIO_ENABLE_ASSERT)
  EXPECT_DEATH({
      CORIO_ASSERT(){
        return false;
      } << 42;
    }, "corio/test/util/assert\\.cpp:165: TestBody: Assertion failed\\.\n"
    "Message: 42");
#endif // defined(CORIO_ENABLE_ASSERT)
}

TEST(corio_test, assert_block_fail_stream_manip_ostream)
{
#if defined(CORIO_ENABLE_ASSERT)
  EXPECT_DEATH({
      CORIO_ASSERT(){
        return false;
      } << std::endl << 42;
    }, "corio/test/util/assert\\.cpp:177: TestBody: Assertion failed\\.\n"
    "Message: \n42");
#endif // defined(CORIO_ENABLE_ASSERT)
}

TEST(corio_test, assert_block_fail_stream_manip_ios)
{
#if defined(CORIO_ENABLE_ASSERT)
  EXPECT_DEATH({
      CORIO_ASSERT(){
        return false;
      } << ios_manip << 42;
    }, "corio/test/util/assert\\.cpp:189: TestBody: Assertion failed\\.\n"
    "Message: 42");
#endif // defined(CORIO_ENABLE_ASSERT)
}

TEST(corio_test, assert_block_fail_stream_manip_ios_base)
{
#if defined(CORIO_ENABLE_ASSERT)
  EXPECT_DEATH({
      CORIO_ASSERT(){
        return false;
      } << std::boolalpha << true;
    }, "corio/test/util/assert\\.cpp:201: TestBody: Assertion failed\\.\n"
    "Message: true\n");
#endif // defined(CORIO_ENABLE_ASSERT)
}
