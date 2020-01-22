#if !defined(CORIO_IS_EXECUTOR_HPP_INCLUDE_GUARD)
#define CORIO_IS_EXECUTOR_HPP_INCLUDE_GUARD

#include <boost/asio/is_executor.hpp>
#include <boost/asio/executor.hpp>
#include <type_traits>


namespace corio{

template<typename T>
struct is_executor
  : public std::bool_constant<!std::is_const_v<T> && !std::is_volatile_v<T>
                              && !std::is_reference_v<T> && boost::asio::is_executor<T>::value>
{};

template<typename T>
using is_executor_t = typename is_executor<T>::type;

template<typename T>
inline constexpr bool is_executor_v = is_executor<T>::value;

} // namespace corio

#endif // !defined(CORIO_IS_EXECUTOR_HPP_INCLUDE_GUARD)
