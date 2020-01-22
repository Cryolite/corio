#if !defined(CORIO_CORE_IS_EXECUTION_CONTEXT_HPP_INCLUDE_GUARD)
#define CORIO_CORE_IS_EXECUTION_CONTEXT_HPP_INCLUDE_GUARD

#include <boost/asio/execution_context.hpp>
#include <type_traits>


namespace corio{

template<typename T>
struct is_execution_context
  : public std::bool_constant<
      !std::is_const_v<T> && !std::is_volatile_v<T> && !std::is_reference_v<T>
      && !std::is_void_v<T> && std::is_convertible_v<T &, boost::asio::execution_context &> >
{};

template<typename T>
using is_execution_context_t = typename is_execution_context<T>::type;

template<typename T>
inline constexpr bool is_execution_context_v = is_execution_context<T>::value;

} // namespace corio

#endif // !defined(CORIO_CORE_IS_EXECUTION_CONTEXT_HPP_INCLUDE_GUARD)
