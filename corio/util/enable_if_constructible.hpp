#if !defined(CORIO_UTIL_ENABLE_IF_CONSTRUCTIBLE_HPP_INCLUDE_GUARD)
#define CORIO_UTIL_ENABLE_IF_CONSTRUCTIBLE_HPP_INCLUDE_GUARD

#include <type_traits>


namespace corio{

template<typename T, typename... Args>
struct enable_if_constructible
  : public std::enable_if<std::is_constructible_v<T, Args...> >
{}; // struct enable_if_constructible

template<typename T, typename... Args>
using enable_if_constructible_t = typename enable_if_constructible<T, Args...>::type;

} // namespace corio

#endif // !defined(CORIO_UTIL_ENABLE_IF_CONSTRUCTIBLE_HPP_INCLUDE_GUARD)
