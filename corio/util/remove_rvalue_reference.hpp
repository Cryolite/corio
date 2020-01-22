#if !defined(CORIO_UTIL_REMOVE_RVALUE_REFERENCE_HPP_INCLUDE_GUARD)
#define CORIO_UTIL_REMOVE_RVALUE_REFERENCE_HPP_INCLUDE_GUARD

#include <type_traits>


namespace corio{

template<typename T>
struct remove_rvalue_reference
  : public std::type_identity<T>
{};

template<typename T>
struct remove_rvalue_reference<T &&>
  : public std::type_identity<T>
{};

template<typename T>
using remove_rvalue_reference_t = typename remove_rvalue_reference<T>::type;

} // namespace corio

#endif // !defined(CORIO_UTIL_REMOVE_RVALUE_REFERENCE_HPP_INCLUDE_GUARD)
