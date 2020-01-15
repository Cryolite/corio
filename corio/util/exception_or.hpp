#if !defined(CORIO_UTIL_EXCEPTION_OR_HPP_INCLUDE_GUARD)
#define CORIO_UTIL_EXCEPTION_OR_HPP_INCLUDE_GUARD

#include <type_traits>
#include <variant>
#include <utility>
#include <exception>


namespace corio{

namespace detail_{

class exception_holder_
{
public:
  explicit exception_holder_(std::exception_ptr p) noexcept
    : p_(std::move(p))
  {}

  exception_holder_(exception_holder_ const &) = default;

  exception_holder_(exception_holder_ &&rhs) noexcept
    : p_(rhs.p_)
  {
    rhs.p_ = nullptr;
  }

  void swap(exception_holder_ &rhs) noexcept
  {
    using std::swap;
    swap(p_, rhs.p_);
  }

  friend void swap(exception_holder_ &lhs, exception_holder_ &rhs) noexcept
  {
    lhs.swap(rhs);
  }

  exception_holder_ &operator=(exception_holder_ const &) = default;

  exception_holder_ &operator=(exception_holder_ &&rhs) noexcept
  {
    exception_holder_(std::move(rhs)).swap(*this);
    return *this;
  }

private:
  std::exception_ptr p_;
}; // class exception_holder_

} // namespace detail_

template<typename T>
class exception_or
{
private:
  using value_type_ = std::variant<T, detail_::exception_holder_>;

  explicit exception_or(T const &value)
    : value_(value)
  {}

  explicit exception_or(T &&value)
    : value_(std::move(value))
  {}

  explicit exception_or(detail_::exception_holder_ const &exception) noexcept
    : value_(exception)
  {}

  explicit exception_or(detail_::exception_holder_ &&exception) noexcept
    : value_(std::move(exception))
  {}

public:
  friend exception_or<T> make_value(T const &value)
  {
    return exception_or<T>(value);
  }

  friend exception_or<T> make_value(T &&value)
  {
    return exception_or<T>(std::move(value));
  }

  friend exception_or<T> make_exception(std::exception_ptr p) noexcept
  {
    return exception_or<T>(detail_::exception_holder_(std::move(p)));
  }

  exception_or<T>(exception_or<T> const &) = default;

  exception_or<T>(exception_or<T> &&) = default;

  void swap(exception_or<T> &rhs) noexcept(noexcept(swap(value_, rhs.value_)))
  {
    using std::swap;
    swap(value_, rhs.value_);
  }

  friend void swap(exception_or<T> &lhs, exception_or<T> &rhs) noexcept(noexcept(lhs.swap(rhs)))
  {
    lhs.swap(rhs);
  }

  exception_or<T> &operator=(exception_or<T> const &rhs) noexcept(noexcept(exception_or<T>(rhs).swap(*this)))
  {
    exception_or<T>(rhs).swap(*this);
    return *this;
  }

  exception_or<T> &operator=(exception_or<T> &&rhs) noexcept(noexcept(exception_or<T>(std::move(rhs)).swap(*this)))
  {
    exception_or<T>(std::move(rhs)).swap(*this);
    return *this;
  }

  T move()
  {
    auto visitor = [](auto &&v) -> T{
      using type = std::decay_t<std::remove_reference_t<decltype(v)> >;
      if constexpr (std::is_same_v<type, detail_::exception_holder_>) {
        v.rethrow();
      }
      else {
        static_assert(std::is_same_v<type, T>);
        return std::move(v);
      }
    };
    return std::visit(std::move(visitor), std::move(value_));
  }

private:
  value_type_ value_;
}; // class excepton_or

} // namespace corio

#endif // !defined(CORIO_UTIL_EXCEPTION_OR_HPP_INCLUDE_GUARD)
