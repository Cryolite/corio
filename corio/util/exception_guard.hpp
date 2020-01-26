#if !defined(CORIO_UTIL_EXCEPTION_GUARD_HPP_INCLUDE_GUARD)
#define CORIO_UTIL_EXCEPTION_GUARD_HPP_INCLUDE_GUARD

#include <boost/preprocessor/cat.hpp>
#include <utility>
#include <exception>


namespace corio::detail_{

template<typename F>
class exception_guard_
{
public:
  explicit exception_guard_(F &&f)
    : f_(std::move(f)),
      n_uncaught_(std::uncaught_exceptions()),
      dismissed_()
  {}

  exception_guard_(exception_guard_ const &) = delete;

  exception_guard_ &operator=(exception_guard_ const &) = delete;

  void dismiss() noexcept
  {
    dismissed_ = true;
  }

  ~exception_guard_()
  {
    if (std::uncaught_exceptions() > n_uncaught_ && !dismissed_) {
      f_();
    }
  }

private:
  F f_;
  int n_uncaught_;
  bool dismissed_;
}; // class exception_guard_

struct exception_guard_arg_t_
{
  template<typename F>
  exception_guard_<F> operator->*(F &&f) const
  {
    return exception_guard_<F>(std::move(f));
  }
}; // struct exception_guard_arg_t_

constexpr exception_guard_arg_t_ exception_guard_arg_;

} // namespace corio::detail_

#define CORIO_EXCEPTION_GUARD_NAMED_(NAME)                           \
auto NAME = ::corio::detail_::exception_guard_arg_ ->* [&]() -> void \
  /**/

#define CORIO_EXCEPTION_GUARD_()                                            \
[[maybe_unused]] auto const &BOOST_PP_CAT(corio_exception_guard_, __LINE__) \
  = ::corio::detail_::exception_guard_arg_ ->* [&]() -> void                \
  /**/

#define CORIO_EXCEPTION_GUARD(...)                         \
CORIO_EXCEPTION_GUARD_ ## __VA_OPT__(NAMED_) (__VA_ARGS__) \
  /**/

#endif // !defined(CORIO_UTIL_EXCEPTION_GUARD_HPP_INCLUDE_GUARD)
