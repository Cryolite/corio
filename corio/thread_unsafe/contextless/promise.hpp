#if !defined(CORIO_THREAD_UNSAFE_CONTEXTLESS_PROMISE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_CONTEXTLESS_PROMISE_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/contextless/future.hpp>
#include <corio/thread_unsafe/contextless/detail_/shared_future_state_.hpp>
#include <corio/core/error.hpp>
#include <corio/util/throw.hpp>
#include <utility>
#include <exception>


namespace corio::thread_unsafe::contextless{

namespace detail_{

template<typename R>
class promise_base_
{
private:
  using state_type_ = corio::thread_unsafe::contextless::detail_::shared_future_state_<R>;

protected:
  promise_base_()
    : state_(),
      future_already_retrieved_()
  {}

  promise_base_(promise_base_ const &) = delete;

  promise_base_(promise_base_ &&rhs) noexcept
    : state_(std::move(rhs.state_)),
      future_already_retrieved_(rhs.future_already_retrieved_)
  {
    rhs.future_already_retrieved_ = false;
  }

  ~promise_base_()
  {
    if (state_.has_context() && !state_.ready()) {
      try{
        CORIO_THROW<corio::broken_promise_error>("");
      }
      catch (...) {
        state_.set_exception(std::current_exception());
      }
    }
  }

  void swap(promise_base_ &rhs) noexcept
  {
    using std::swap;
    swap(state_, rhs.state_);
    swap(future_already_retrieved_, rhs.future_already_retrieved_);
  }

  promise_base_ &operator=(promise_base_ const &) = delete;

  promise_base_ &operator=(promise_base_ &&rhs) noexcept
  {
    promise_base_(std::move(rhs)).swap(*this);
    return *this;
  }

  corio::thread_unsafe::contextless::future<R> get_future()
  {
    if (future_already_retrieved_) {
      CORIO_THROW<corio::future_already_retrieved_error>("");
    }
    if (!state_.valid()) {
      CORIO_THROW<corio::no_future_state_error>("");
    }
    corio::thread_unsafe::contextless::future<R> future(state_);
    future_already_retrieved_ = true;
    return future;
  }

  void set_exception(std::exception_ptr p)
  {
    if (state_.ready()) {
      CORIO_THROW<corio::promise_already_satisfied_error>("");
    }
    if (!state_.valid()) {
      CORIO_THROW<corio::no_future_state_error>("");
    }
    state_.set_exception(std::move(p));
  }

protected:
  state_type_ state_;
  bool future_already_retrieved_;
}; // class promise_base_

} // namespace detail_

template<typename R>
class promise
  : private detail_::promise_base_<R>
{
private:
  using base_type_ = detail_::promise_base_<R>;
  using base_type_::state_;

public:
  promise()
    : base_type_()
  {}

  void swap(promise &rhs) noexcept
  {
    base_type_::swap(rhs);
  }

  friend void swap(promise &lhs, promise &rhs) noexcept
  {
    lhs.swap(rhs);
  }

  using base_type_::get_future;

  void set_value(R const &value)
  {
    if (state_.ready()) {
      CORIO_THROW<corio::promise_already_satisfied_error>("");
    }
    if (!state_.valid()) {
      CORIO_THROW<corio::no_future_state_error>("");
    }
    state_.set_value(value);
  }

  void set_value(R &&value)
  {
    if (state_.ready()) {
      CORIO_THROW<corio::promise_already_satisfied_error>("");
    }
    if (!state_.valid()) {
      CORIO_THROW<corio::no_future_state_error>("");
    }
    state_.set_value(std::move(value));
  }

  using base_type_::set_exception;
}; // class promise

template<typename R>
class promise<R &>
  : private detail_::promise_base_<R &>
{
private:
  using base_type_ = detail_::promise_base_<R &>;
  using base_type_::state_;

public:
  promise()
    : base_type_()
  {}

  void swap(promise &rhs) noexcept
  {
    base_type_::swap(rhs);
  }

  friend void swap(promise &lhs, promise &rhs)
  {
    lhs.swap(rhs);
  }

  using base_type_::get_future;

  void set_value(R &value)
  {
    if (state_.ready()) {
      CORIO_THROW<corio::promise_already_satisfied_error>("");
    }
    if (!state_.valid()) {
      CORIO_THROW<corio::no_future_state_error>("");
    }
    state_.set_value(value);
  }

  using base_type_::set_exception;
}; // class promise<R &>

template<>
class promise<void>
  : private detail_::promise_base_<void>
{
private:
  using base_type_ = detail_::promise_base_<void>;
  using base_type_::state_;

public:
  explicit promise()
    : base_type_()
  {}

  void swap(promise &rhs) noexcept
  {
    base_type_::swap(rhs);
  }

  friend void swap(promise &lhs, promise &rhs) noexcept
  {
    lhs.swap(rhs);
  }

  using base_type_::get_future;

  void set_value()
  {
    if (state_.ready()) {
      CORIO_THROW<corio::promise_already_satisfied_error>("");
    }
    if (!state_.valid()) {
      CORIO_THROW<corio::no_future_state_error>("");
    }
    state_.set_value();
  }

  using base_type_::set_exception;
}; // class promise<void>

} // namespace corio::thread_unsafe::contextless

#endif // !defined(CORIO_THREAD_UNSAFE_CONTEXTLESS_PROMISE_HPP_INCLUDE_GUARD)
