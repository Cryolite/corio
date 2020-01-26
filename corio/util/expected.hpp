#if !defined(CORIO_UTIL_EXPECTED_HPP_INCLUDE_GUARD)
#define CORIO_UTIL_EXPECTED_HPP_INCLUDE_GUARD

#include <corio/util/throw.hpp>
#include <boost/config.hpp>
#include <type_traits>
#include <variant>
#include <utility>
#include <stdexcept>
#include <exception>


namespace corio{

class bad_expected_access
  : public std::logic_error
{
public:
  bad_expected_access()
    : std::logic_error("Attempting to access an `corio::expected' object that does not contain a value.")
  {}
}; // class bad_expected_access

struct exception_arg_t
{};

inline constexpr exception_arg_t exception_arg{};

template<typename T>
class expected;

template<typename T>
class expected
{
private:
  using impl_type_ = std::variant<std::exception_ptr, T>;

public:
  constexpr expected() noexcept
    : v_()
  {}

  template<typename... Args>
  explicit(sizeof...(Args) == 0u) expected(std::in_place_t, Args &&... args)
    : v_(std::in_place_index<1u>, std::forward<Args>(args)...)
  {}

  expected(exception_arg_t, std::exception_ptr p)
    : v_(std::in_place_index<0u>, std::move(p))
  {
    if (BOOST_UNLIKELY(*std::get_if<0u>(v_) == nullptr)) /*[[unlikely]]*/ {
      CORIO_THROW<std::invalid_argument>("Attempting to assign a null pointer constant of the type"
                                         " `std::exception_ptr' to a `corio::expected' object.");
    }
  }

  expected(expected const &rhs) = default;

  expected(expected &&rhs) = default;

  void swap(expected &rhs) noexcept(std::is_nothrow_swappable_v<impl_type_>)
  {
    using std::swap;
    swap(v_, rhs.v_);
  }

  friend void swap(expected &lhs, expected &rhs) noexcept(noexcept(lhs.swap(rhs)))
  {
    lhs.swap(rhs);
  }

  expected &operator=(expected const &rhs)
  {
    expected(rhs).swap(*this);
    return *this;
  }

  expected &operator=(expected &&rhs)
  {
    expected(std::move(rhs)).swap(*this);
    return *this;
  }

  template<typename... Args>
  T &emplace(Args &&... args)
  {
    return v_.template emplace<1u>(std::forward<Args>(args)...);
  }

  void set_exception(std::exception_ptr p)
  {
    if (BOOST_UNLIKELY(p == nullptr)) /*[[unlikely]]*/ {
      CORIO_THROW<std::invalid_argument>("Attempting to assign a null pointer constant of the type"
                                         " `std::exception_ptr' to a `corio::expected' object.");
    }
    v_.template emplace<0u>(std::move(p));
  }

  explicit operator bool() const noexcept
  {
    return std::get_if<0u>(&v_) == nullptr || *std::get_if<0u>(&v_) != nullptr;
  }

  T &get() &
  {
    T *p = std::get_if<1u>(&v_);
    if (BOOST_UNLIKELY(p == nullptr)) /*[[unlikely]]*/ {
      std::exception_ptr p = *std::get_if<0u>(&v_);
      if (BOOST_UNLIKELY(p == nullptr)) /*[[unlikely]]*/ {
        CORIO_THROW<corio::bad_expected_access>();
      }
      std::rethrow_exception(std::move(p));
    }
    return *p;
  }

  T const &get() const &
  {
    T const *p = std::get_if<1u>(&v_);
    if (BOOST_UNLIKELY(p == nullptr)) /*[[unlikely]]*/ {
      std::exception_ptr p = *std::get_if<0u>(&v_);
      if (BOOST_UNLIKELY(p == nullptr)) /*[[unlikely]]*/ {
        CORIO_THROW<corio::bad_expected_access>();
      }
      std::rethrow_exception(std::move(p));
    }
    return *p;
  }

  T &&get() &&
  {
    T *p = std::get_if<1u>(&v_);
    if (BOOST_UNLIKELY(p == nullptr)) /*[[unlikely]]*/ {
      std::exception_ptr p = std::move(*std::get_if<0u>(&v_));
      if (BOOST_UNLIKELY(p == nullptr)) /*[[unlikely]]*/ {
        CORIO_THROW<corio::bad_expected_access>();
      }
      std::rethrow_exception(std::move(p));
    }
    return std::move(*p);
  }

  T const &&get() const &&
  {
    T const *p = std::get_if<1u>(&v_);
    if (BOOST_UNLIKELY(p == nullptr)) /*[[unlikely]]*/ {
      std::exception_ptr p =*std::get_if<0u>(&v_);
      if (BOOST_UNLIKELY(p == nullptr)) /*[[unlikely]]*/ {
        CORIO_THROW<corio::bad_expected_access>();
      }
      std::rethrow_exception(std::move(p));
    }
    return std::move(*p);
  }

private:
  impl_type_ v_;
}; // class expected

template<typename T>
class expected<T &>
  : private expected<std::reference_wrapper<T> >
{
private:
  using mixin_type_ = expected<std::reference_wrapper<T> >;

public:
  constexpr expected() noexcept
    : mixin_type_()
  {}

  expected(std::in_place_t, T &r) noexcept
    : mixin_type_(std::in_place, r)
  {}

  expected(expected const &rhs) = default;

  expected(expected &&rhs) = default;

  void swap(expected &rhs) noexcept(noexcept(static_cast<mixin_type_ &>(*this).swap(rhs)))
  {
    static_cast<mixin_type_ &>(*this).swap(rhs);
  }

  friend void swap(expected &lhs, expected &rhs) noexcept(noexcept(lhs.swap(rhs)))
  {
    lhs.swap(rhs);
  }

  expected &operator=(expected const &rhs)
  {
    expected(rhs).swap(*this);
    return *this;
  }

  expected &operator=(expected &&rhs)
  {
    expected(std::move(rhs)).swap(*this);
    return *this;
  }

  T &emplace(T &r)
  {
    return mixin_type_::emplace(r);
  }

  using mixin_type_::set_exception;

  using mixin_type_::operator bool;

  T &get() &
  {
    return static_cast<mixin_type_ &>(*this).get();
  }

  T const &get() const &
  {
    return static_cast<mixin_type_ const &>(*this).get();
  }

  T &get() &&
  {
    return static_cast<mixin_type_ &&>(*this).get();
  }

  T const &get() const &&
  {
    return static_cast<mixin_type_ const &&>(*this).get();
  }
}; // class expected<T &>

template<>
class expected<void>
  : private expected<std::monostate>
{
private:
  using mixin_type_ = expected<std::monostate>;

public:
  constexpr expected() noexcept
    : mixin_type_()
  {}

  explicit expected(std::in_place_t) noexcept
    : mixin_type_(std::in_place, std::monostate())
  {}

  expected(expected const &rhs) = default;

  expected(expected &&rhs) = default;

  void swap(expected &rhs) noexcept
  {
    static_cast<mixin_type_ &>(*this).swap(rhs);
  }

  friend void swap(expected &lhs, expected &rhs) noexcept
  {
    lhs.swap(rhs);
  }

  expected &operator=(expected const &rhs)
  {
    expected(rhs).swap(*this);
    return *this;
  }

  expected &operator=(expected &&rhs)
  {
    expected(std::move(rhs)).swap(*this);
    return *this;
  }

  void emplace()
  {
    mixin_type_::emplace(std::monostate());
  }

  using mixin_type_::set_exception;

  using mixin_type_::operator bool;

  void get() &
  {
    static_cast<mixin_type_ &>(*this).get();
  }

  void get() const &
  {
    static_cast<mixin_type_ const &>(*this).get();
  }

  void get() &&
  {
    static_cast<mixin_type_ &&>(*this).get();
  }

  void get() const &&
  {
    static_cast<mixin_type_ const &&>(*this).get();
  }
}; // class expected<void>

template<typename T>
auto get(expected<T> &e) -> decltype(e.get())
{
  return e.get();
}

template<typename T>
auto get(expected<T> const &e) -> decltype(e.get())
{
  return e.get();
}

template<typename T>
auto get(expected<T> &&e) -> decltype(std::move(e).get())
{
  return std::move(e).get();
}

template<typename T>
auto get(expected<T> const &&e) -> decltype(std::move(e).get())
{
  return std::move(e).get();
}

} // namespace corio

#endif // !defined(CORIO_UTIL_EXPECTED_HPP_INCLUDE_GUARD)
