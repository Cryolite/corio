#if !defined(CORIO_CORE_ENABLE_IF_EXECUTION_CONTEXT_HPP_INCLUDE_GUARD)
#define CORIO_CORE_ENABLE_IF_EXECUTION_CONTEXT_HPP_INCLUDE_GUARD

#include <corio/core/is_execution_context.hpp>
#include <type_traits>


namespace corio{

template<typename T, typename U = void>
struct enable_if_execution_context
  : public std::enable_if<corio::is_execution_context_v<T>, U>
{};

template<typename T, typename U = void>
using enable_if_execution_context_t = typename enable_if_execution_context<T, U>::type;

template<typename T, typename U = void>
struct disable_if_execution_context
  : public std::enable_if<!corio::is_execution_context_v<T>, U>
{};

template<typename T, typename U = void>
using disable_if_execution_context_t = typename disable_if_execution_context<T, U>::type;

} // namespace corio

#endif // !defined(CORIO_CORE_ENABLE_IF_EXECUTION_CONTEXT_HPP_INCLUDE_GUARD)
