#if !defined(CORIO_UTIL_IS_EMPTY_HPP_INCLUDE_GUARD)
#define CORIO_UTIL_IS_EMPTY_HPP_INCLUDE_GUARD

#include <memory>


namespace corio{

template<typename T>
bool is_empty(std::shared_ptr<T> const &p) noexcept
{
  return !p.owner_before(std::shared_ptr<T>()) && !std::shared_ptr<T>().owner_before(p);
}

template<typename T>
bool is_empty(std::weak_ptr<T> const &p) noexcept
{
  return !p.owner_before(std::weak_ptr<T>()) && !std::weak_ptr<T>().owner_before(p);
}

} // namespace corio

#endif // !defined(CORIO_UTIL_IS_EMPTY_HPP_INCLUDE_GUARD)
