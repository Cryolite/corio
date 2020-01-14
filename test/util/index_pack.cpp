#include <corio/util/index_pack.hpp>

#include <gtest/gtest.h>


namespace{

int i = 0;

void nullary()
{
  i = 136;
}

} // namespace *unnamed*

TEST(index_pack, nullary)
{
  CORIO_INDEX_PACK(is, 0){
    nullary(is...);
  };
  EXPECT_EQ(i, 136);
}

TEST(index_pack, unary)
{
  CORIO_INDEX_PACK(is, 1){
    EXPECT_EQ(is..., 0);
  };
}

TEST(index_pack, left_binary_fold)
{
  CORIO_INDEX_PACK(is, 10){
    EXPECT_EQ((0 + ... + is), 45);
  };
}
