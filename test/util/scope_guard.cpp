#include <corio/util/scope_guard.hpp>
#include <gtest/gtest.h>
#include <stdexcept>


TEST(scope_guard, basic)
{
  int i = 42;
  {
    CORIO_SCOPE_GUARD{
      i += 1;
    };
    ASSERT_EQ(i, 42);
  }
  ASSERT_EQ(i, 43);
}

TEST(scope_guard, exception)
{
  int i = 42;
  try{
    CORIO_SCOPE_GUARD{
      i *= 2;
    };
    EXPECT_EQ(i, 42);
    throw std::runtime_error("An exception.");
  }
  catch (std::runtime_error const &e) {
    EXPECT_EQ(i, 84);
  }
}

TEST(scope_guard, multiple_same_level)
{
  int i = 42;
  {
    CORIO_SCOPE_GUARD{
      i += 2;
    };
    EXPECT_EQ(i, 42);
    CORIO_SCOPE_GUARD{
      i *= 2;
    };
    EXPECT_EQ(i, 42);
  }
  EXPECT_EQ(i, 86);
}

TEST(scope_guard, multiple_nested)
{
  int i = 42;
  {
    CORIO_SCOPE_GUARD{
      i *= 2;
    };
    EXPECT_EQ(i, 42);
    {
      CORIO_SCOPE_GUARD{
        i += 2;
      };
      EXPECT_EQ(i, 42);
    }
    EXPECT_EQ(i, 44);
  }
  EXPECT_EQ(i, 88);
}

namespace{

int i = 42;

int f()
{
  CORIO_SCOPE_GUARD{
    i -= 2;
  };
  return i;
}

} // namespace *unnamed*

TEST(scope_guard, after_return)
{
  EXPECT_EQ(i, 42);
  EXPECT_EQ(f(), 42);
  EXPECT_EQ(i, 40);
}
