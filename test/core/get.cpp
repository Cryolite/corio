#include <corio/core/get.hpp>

#include <gtest/gtest.h>
#include <type_traits>
#include <functional>
#include <variant>
#include <exception>


TEST(get, value_lref)
{
  std::variant<int, std::exception_ptr> v(std::in_place_index<0u>, 42);
  static_assert(std::is_same_v<decltype(corio::get(v)), int>);
  EXPECT_EQ(corio::get(v), 42);
}

TEST(get, value_clref)
{
  std::variant<int, std::exception_ptr> v(std::in_place_index<0u>, 42);
  static_assert(std::is_same_v<decltype(corio::get(std::as_const(v))), int>);
  EXPECT_EQ(corio::get(v), 42);
}

TEST(get, value_rref)
{
  std::variant<int, std::exception_ptr> v(std::in_place_index<0u>, 42);
  static_assert(std::is_same_v<decltype(corio::get(std::move(v))), int>);
  EXPECT_EQ(corio::get(v), 42);
}

TEST(get, value_crref)
{
  std::variant<int, std::exception_ptr> v(std::in_place_index<0u>, 42);
  static_assert(std::is_same_v<decltype(corio::get(std::move(std::as_const(v)))), int>);
  EXPECT_EQ(corio::get(v), 42);
}

TEST(get, lref_lref)
{
  int i = 42;
  std::variant<std::reference_wrapper<int>, std::exception_ptr> v(std::in_place_index<0u>, std::ref(i));
  static_assert(std::is_same_v<decltype(corio::get(v)), int &>);
  EXPECT_EQ(&corio::get(v), &i);
}

TEST(get, lref_clref)
{
  int i = 42;
  std::variant<std::reference_wrapper<int>, std::exception_ptr> v(std::in_place_index<0u>, std::ref(i));
  static_assert(std::is_same_v<decltype(corio::get(std::as_const(v))), int const &>);
  EXPECT_EQ(&corio::get(v), &i);
}

TEST(get, lref_rref)
{
  int i = 42;
  std::variant<std::reference_wrapper<int>, std::exception_ptr> v(std::in_place_index<0u>, std::ref(i));
  static_assert(std::is_same_v<decltype(corio::get(std::move(v))), int &>);
  EXPECT_EQ(&corio::get(v), &i);
}

TEST(get, lref_crref)
{
  int i = 42;
  std::variant<std::reference_wrapper<int>, std::exception_ptr> v(std::in_place_index<0u>, std::ref(i));
  static_assert(std::is_same_v<decltype(corio::get(std::move(std::as_const(v)))), int const &>);
  EXPECT_EQ(&corio::get(v), &i);
}

TEST(get, exception)
{
  auto p = std::make_exception_ptr(std::runtime_error("Hello, world!"));
  std::variant<int, std::exception_ptr> v(std::in_place_index<1u>, p);
  EXPECT_THROW(corio::get(v), std::runtime_error);
}
