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
  T const *p = std::get_if<0u>(&v);
  if (BOOST_LIKELY(p != nullptr)) /*[[likely]]*/ {
    return *p;
  }
  std::rethrow_exception(*std::get_if<1u>(&v));
}

template<typename T>
auto get(std::variant<T, std::exception_ptr> &&v)
{
  T *p = std::get_if<0u>(&v);
  if (BOOST_LIKELY(p != nullptr)) /*[[likely]]*/ {
    return std::move(*p);
  }
  std::rethrow_exception(*std::get_if<1u>(&v));
}

template<typename T>
T &get(std::variant<std::reference_wrapper<T>, std::exception_ptr> &v)
{
  auto p = std::get_if<0u>(&v);
  if (BOOST_LIKELY(p != nullptr)) /*[[likely]]*/ {
    return *p;
  }
  std::rethrow_exception(*std::get_if<1u>(&v));
}

template<typename T>
T const &get(std::variant<std::reference_wrapper<T>, std::exception_ptr> const &v)
{
  auto const *p = std::get_if<0u>(&v);
  if (BOOST_LIKELY(p != nullptr)) /*[[likely]]*/ {
    return *p;
  }
  std::rethrow_exception(*std::get_if<1u>(&v));
}

template<typename T>
T &get(std::variant<std::reference_wrapper<T>, std::exception_ptr> &&v)
{
  auto *p = std::get_if<0u>(&v);
  if (BOOST_LIKELY(p != nullptr)) /*[[likely]]*/ {
    return *p;
  }
  std::rethrow_exception(*std::get_if<1u>(&v));
}

template<typename T>
T const &get(std::variant<std::reference_wrapper<T>, std::exception_ptr> const &&v)
{
  auto const *p = std::get_if<0u>(&v);
  if (BOOST_LIKELY(p != nullptr)) /*[[likely]]*/ {
    return *p;
  }
  std::rethrow_exception(*std::get_if<1u>(&v));
}

inline void get(std::variant<std::monostate, std::exception_ptr> &v)
{
  std::monostate *p = std::get_if<0u>(&v);
  if (BOOST_LIKELY(p != nullptr)) /*[[likely]]*/ {
    return;
  }
  std::rethrow_exception(*std::get_if<1u>(&v));
}

inline void get(std::variant<std::monostate, std::exception_ptr> const &v)
{
  std::monostate const *p = std::get_if<0u>(&v);
  if (BOOST_LIKELY(p != nullptr)) /*[[likely]]*/ {
    return;
  }
  std::rethrow_exception(*std::get_if<1u>(&v));
}

inline void get(std::variant<std::monostate, std::exception_ptr> &&v)
{
  std::monostate *p = std::get_if<0u>(&v);
  if (BOOST_LIKELY(p != nullptr)) /*[[likely]]*/ {
    return;
  }
  std::rethrow_exception(*std::get_if<1u>(&v));
}

inline void get(std::variant<std::monostate, std::exception_ptr> const &&v)
{
  std::monostate const *p = std::get_if<0u>(&v);
  if (BOOST_LIKELY(p != nullptr)) /*[[likely]]*/ {
    return;
  }
  std::rethrow_exception(*std::get_if<1u>(&v));
}

} // namespace corio

#endif // !defined(CORIO_CORE_GET_HPP_INCLUDE_GUARD)
