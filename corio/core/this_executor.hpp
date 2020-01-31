#if !defined(CORIO_CORE_THIS_EXECUTOR_HPP_INCLUDE_GUARD)
#define CORIO_CORE_THIS_EXECUTOR_HPP_INCLUDE_GUARD


namespace corio{

struct this_executor_t
{};

inline constexpr this_executor_t this_executor{};

} // namespace corio

#define CORIO_THIS_EXECUTOR() co_await ::corio::this_executor

#endif // !defined(CORIO_CORE_THIS_EXECUTOR_HPP_INCLUDE_GUARD)
