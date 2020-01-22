#if !defined(CORIO_UTIL_MOVE_ONLY_FUNCTION_HPP_INCLUDE_GUARD)
#define CORIO_UTIL_MOVE_ONLY_FUNCTION_HPP_INCLUDE_GUARD

#include <type_traits>
#include <functional>
#include <utility>
#include <typeinfo>


namespace corio{

template<typename>
class move_only_function;

template<typename R, typename... ArgTypes>
class move_only_function<R(ArgTypes...)>
{
public:
  using result_type = R;

  move_only_function() = default;

  move_only_function(std::nullptr_t) noexcept
    : f_(nullptr)
  {}

  move_only_function(move_only_function const &) = delete;

  move_only_function(move_only_function &&rhs) noexcept
    : f_(std::move(rhs.f_))
  {}

  template<typename F>
  move_only_function(F &&f, std::enable_if_t<!std::is_reference_v<F> > * = nullptr)
    : f_()
  {
    class copy_as_move
    {
    private:
      using decayed_type_ = std::decay_t<F>;

    public:
      explicit copy_as_move(F &&f)
        : f_(std::move<F>(f))
      {}

      copy_as_move(copy_as_move &rhs)
        : f_(std::move(rhs.f_))
      {}

      copy_as_move(copy_as_move &&rhs)
        : f_(std::move(rhs.f_))
      {}

      copy_as_move &operator=(copy_as_move const &) = delete;

      R operator()(ArgTypes... args)
      {
        return std::invoke<R>(f_, std::forward<ArgTypes>(args)...);
      }

    private:
      decayed_type_ f_;
    };

    f_ = copy_as_move(std::forward<F>(f));
  }

  void swap(move_only_function &rhs) noexcept
  {
    f_.swap(rhs.f_);
  }

  friend void swap(move_only_function &lhs, move_only_function &rhs) noexcept
  {
    lhs.swap(rhs);
  }

  move_only_function &operator=(move_only_function const &) = delete;

  move_only_function &operator=(move_only_function &&rhs) noexcept
  {
    move_only_function(std::move(rhs)).swap(*this);
    return *this;
  }

  move_only_function &operator=(std::nullptr_t) noexcept
  {
    move_only_function(nullptr).swap(*this);
    return *this;
  }

  template<typename F>
  move_only_function &operator=(F &&f)
  {
    move_only_function(std::forward<F>(f)).swap(*this);
    return *this;
  }

  template<typename F>
  move_only_function &operator=(std::reference_wrapper<F> f) noexcept
  {
    move_only_function(f).swap(*this);
    return *this;
  }

  template<typename R_, typename... ArgTypes_>
  friend bool operator==(move_only_function<R_(ArgTypes_...)> &lhs, std::nullptr_t) noexcept
  {
    return lhs.f_ == nullptr;
  }

  explicit operator bool() const noexcept
  {
    return !!f_;
  }

  R operator()(ArgTypes... args)
  {
    return std::invoke(f_, std::forward<ArgTypes>(args)...);
  }

  std::type_info const &target_type() const noexcept
  {
    return f_.target_type();
  }

  template<typename T>
  T *target() noexcept
  {
    return f_.target();
  }

  template<typename T>
  T const *target() const noexcept
  {
    return f_.target();
  }

private:
  std::function<R(ArgTypes...)> f_;
}; // class move_only_function

} // namespace corio

#endif // !defined(CORIO_UTIL_MOVE_ONLY_FUNCTION_HPP_INCLUDE_GUARD)
