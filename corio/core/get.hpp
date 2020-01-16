#if !defined(CORIO_CORE_GET_HPP_INCLUDE_GUARD)
#define CORIO_CORE_GET_HPP_INCLUDE_GUARD

#include <corio/util/assert.hpp>
#include <type_traits>
#include <variant>
#include <utility>
#include <exception>
#include <cstddef>


namespace corio{

template<typename T>
auto get(std::variant<T, std::exception_ptr> const &v)
{
  try {
    return std::get<0u>(v);
  }
  catch (std::bad_variant_access const &) {
    std::exception_ptr p = std::get<1u>(v);
    std::rethrow_exception(std::move(p));
  }
}

template<typename T>
auto get(std::variant<T, std::exception_ptr> &&v)
{
  try {
    return std::get<0u>(v);
  }
  catch (std::bad_variant_access const &) {
    std::exception_ptr p = std::get<1u>(v);
    std::rethrow_exception(std::move(p));
  }
}

template<typename T>
T &get(std::variant<std::reference_wrapper<T>, std::exception_ptr> &v)
{
  try {
    return std::get<0u>(v);
  }
  catch (std::bad_variant_access const &) {
    std::exception_ptr p = std::get<1u>(v);
    std::rethrow_exception(std::move(p));
  }
}

template<typename T>
T const &get(std::variant<std::reference_wrapper<T>, std::exception_ptr> const &v)
{
  try {
    return std::get<0u>(v);
  }
  catch (std::bad_variant_access const &) {
    std::exception_ptr p = std::get<1u>(v);
    std::rethrow_exception(std::move(p));
  }
}

template<typename T>
T &get(std::variant<std::reference_wrapper<T>, std::exception_ptr> &&v)
{
  try {
    return std::get<0u>(std::move(v));
  }
  catch (std::bad_variant_access const &) {
    std::exception_ptr p = std::get<1u>(v);
    std::rethrow_exception(std::move(p));
  }
}

template<typename T>
T const &get(std::variant<std::reference_wrapper<T>, std::exception_ptr> const &&v)
{
  try {
    return std::get<0u>(std::move(v));
  }
  catch (std::bad_variant_access const &) {
    std::exception_ptr p = std::get<1u>(v);
    std::rethrow_exception(std::move(p));
  }
}

} // namespace corio

#endif // !defined(CORIO_CORE_GET_HPP_INCLUDE_GUARD)
