#if !defined(CORIO_CORE_USE_FUTURE_HPP_INCLUDE_GUARD)
#define CORIO_CORE_USE_FUTURE_HPP_INCLUDE_GUARD

#include <type_traits>
#include <new>


namespace corio{

template<typename NoThrow>
struct use_future_t;

template<>
struct use_future_t<std::true_type>
{};

template<>
struct use_future_t<std::false_type>
{
  constexpr use_future_t<std::true_type> operator()(std::nothrow_t)
  {
    return use_future_t<std::true_type>();
  }
}; // struct use_future_t

inline constexpr use_future_t<std::false_type> use_future;

} // namespace corio

#define CORIO_USE_FUTURE(...) co_await ::corio::use_future __VA_OPT__((__VA_ARGS__))

#endif // !defined(CORIO_CORE_USE_FUTURE_HPP_INCLUDE_GUARD)
