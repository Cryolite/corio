#if !defined(CORIO_CORE_IS_EXECUTOR_AWARE_HPP_INCLUDE_GUARD)
#define CORIO_CORE_IS_EXECUTOR_AWARE_HPP_INCLUDE_GUARD

#include <corio/core/is_executor.hpp>
#include <type_traits>
#include <utility>


namespace corio{

namespace detail_{

template<typename T>
inline constexpr bool has_executor_type_(char)
{
  return false;
}

template<typename T>
using has_executor_type_impl_t_ = bool;

template<typename T>
inline constexpr has_executor_type_impl_t_<typename T::executor_type> has_executor_type_(int)
{
  return corio::is_executor_v<typename T::executor_type>;
}

template<typename T>
inline constexpr bool has_get_executor_(char)
{
  return false;
}

template<typename T>
inline constexpr decltype(std::declval<T &>().get_executor(), false) has_get_executor_(int)
{
  return std::is_same_v<decltype(std::declval<T &>().get_executor()), typename T::executor_type>;
}

template<typename T>
struct is_executor_aware_impl_
  : public std::bool_constant<corio::detail_::has_get_executor_<T>(0)>
{};

} // namespace detail_

template<typename T>
struct is_executor_aware
  : public std::conditional_t<
      detail_::has_executor_type_<T>(0),
      detail_::is_executor_aware_impl_<T>,
      std::false_type>
{}; // struct is_executor_aware

template<typename T>
inline constexpr bool is_executor_aware_v = is_executor_aware<T>::value;

} // namespace corio

#endif // !defined(CORIO_CORE_IS_EXECUTOR_AWARE_HPP_INCLUDE_GUARD)
