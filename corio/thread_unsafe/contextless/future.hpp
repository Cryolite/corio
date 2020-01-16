#if !defined(CORIO_THREAD_UNSAFE_CONTEXTLESS_FUTURE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_CONTEXTLESS_FUTURE_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/contextless/detail_/shared_future_state_.hpp>
#include <corio/core/get.hpp>
#include <corio/core/error.hpp>
#include <corio/util/throw.hpp>
#include <variant>
#include <utility>
#include <exception>


namespace corio::thread_unsafe::contextless{

namespace detail_{

template<typename R>
class promise_base_;

} // namespace detail_

template<typename R>
class future
{
public:
  using result_type = std::variant<R, std::exception_ptr>;

private:
  using state_type_ = corio::thread_unsafe::contextless::detail_::shared_future_state_<R>;

  friend class detail_::promise_base_<R>;

  explicit future(state_type_ const &state)
    : state_(state)
  {}

public:
  future() noexcept
    : state_(nullptr)
  {}

  future(future const &) = delete;

  future(future &&rhs) = default;

  void swap(future &rhs) noexcept
  {
    using std::swap;
    swap(state_, rhs.state_);
  }

  friend void swap(future &lhs, future &rhs) noexcept
  {
    lhs.swap(rhs);
  }

  future &operator=(future const &) = delete;

  future &operator=(future &&rhs) noexcept
  {
    future(std::move(rhs)).swap(*this);
    return *this;
  }

  template<typename ExecutionContext>
  void set_context(ExecutionContext &ctx)
  {
    if (!state_.valid()) {
      CORIO_THROW<corio::no_future_state_error>("");
    }
    state_.set_context(ctx);
  }

  bool valid() const noexcept
  {
    return state_.valid();
  }

  template<typename CompletionToken>
  void async_get(CompletionToken &&token)
  {
    if (!state_.valid()) {
      CORIO_THROW<corio::no_future_state_error>("");
    }
    return state_.async_get(std::move(token));
  }

  template<typename CompletionToken>
  void async_wait(CompletionToken &&token)
  {
    if (!state_.valid()) {
      CORIO_THROW<corio::no_future_state_error>("");
    }
    return state_.async_wait(std::move(token));
  }

private:
  state_type_ state_;
}; // class future

} // namespace corio::thread_unsafe::contextless

#endif // !defined(CORIO_THREAD_UNSAFE_CONTEXTLESS_FUTURE_HPP_INCLUDE_GUARD)
