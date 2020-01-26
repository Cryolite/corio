#if !defined(CORIO_ENABLE_IF_EXECUTOR_HPP_INCLUDE_GUARD)
#define CORIO_ENABLE_IF_EXECUTOR_HPP_INCLUDE_GUARD

#include <corio/core/is_executor.hpp>
#include <type_traits>


namespace corio{

template<typename T, typename U = void>
struct enable_if_executor
  : public std::enable_if<corio::is_executor_v<T>, U>
{};

template<typename T, typename U = void>
using enable_if_executor_t = typename enable_if_executor<T, U>::type;

template<typename T, typename U = void>
struct disable_if_executor
  : public std::enable_if<!corio::is_executor_v<T>, U>
{};

template<typename T, typename U = void>
using disable_if_executor_t = typename disable_if_executor<T, U>::type;

} // namespace corio

#endif // !defined(CORIO_ENABLE_IF_EXECUTOR_HPP_INCLUDE_GUARD)
