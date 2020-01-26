#if !defined(CORIO_UTIL_SCOPE_GUARD_HPP_INCLUDE_GUARD)
#define CORIO_UTIL_SCOPE_GUARD_HPP_INCLUDE_GUARD

#include <boost/preprocessor/cat.hpp>
#include <utility>


namespace corio::detail_{

template<typename F>
class scope_guard_
{
public:
  explicit scope_guard_(F &&f)
    : f_(std::move(f))
  {}

  scope_guard_(scope_guard_ const &) = delete;

  scope_guard_ &operator=(scope_guard_ const &) = delete;

  ~scope_guard_() noexcept(false)
  {
    f_();
  }

private:
  F f_;
}; // class scope_guard_

struct scope_guard_arg_t_
{
  template<typename F>
  scope_guard_<F> operator->*(F &&f) const
  {
    return scope_guard_<F>(std::move(f));
  }
}; // struct scope_guard_arg_t_

constexpr scope_guard_arg_t_ scope_guard_arg_;

} // namespace corio::detail_

#define CORIO_SCOPE_GUARD \
[[maybe_unused]] auto const &BOOST_PP_CAT(corio_scope_guard_, __LINE__) \
  = ::corio::detail_::scope_guard_arg_ ->* [&]() -> void                \
  /**/

#endif // !defined(CORIO_UTIL_SCOPE_GUARD_HPP_INCLUDE_GUARD)
