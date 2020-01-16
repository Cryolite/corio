#if !defined(CORIO_THREAD_UNSAFE_PROMISE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_PROMISE_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/future.hpp>
#include <corio/thread_unsafe/detail_/shared_future_state_.hpp>
#include <corio/util/throw.hpp>
#include <boost/asio/execution_context.hpp>
#include <future>
#include <type_traits>
#include <utility>
#include <exception>


namespace corio::thread_unsafe{

namespace detail_{

template<typename R, typename ExecutionContext>
class promise_base_
{
protected:
  using context_type = ExecutionContext;

  static_assert(!std::is_const_v<context_type>);
  static_assert(!std::is_volatile_v<context_type>);
  static_assert(!std::is_reference_v<context_type>);
  static_assert(std::is_convertible_v<context_type &, boost::asio::execution_context &>);

private:
  using state_type_ = corio::thread_unsafe::detail_::shared_future_state_<R, context_type>;

protected:
  promise_base_()
    : state_(),
      future_already_retrieved_()
  {}

  explicit promise_base_(context_type &ctx)
    : state_(ctx),
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
    if (!state_.ready()) {
      std::exception_ptr p = std::make_exception_ptr(std::future_error(std::future_errc::broken_promise));
      state_.set_exception(std::move(p));
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

  corio::thread_unsafe::basic_future<R, context_type> get_future()
  {
    if (future_already_retrieved_) {
      CORIO_THROW<std::future_error>(std::future_errc::future_already_retrieved);
    }
    if (!state_.valid()) {
      CORIO_THROW<std::future_error>(std::future_errc::no_state);
    }
    corio::thread_unsafe::basic_future<R, context_type> future(state_);
    future_already_retrieved_ = true;
    return future;
  }

  void set_exception(std::exception_ptr p)
  {
    if (state_.ready()) {
      CORIO_THROW<std::future_error>(std::future_errc::promise_already_satisfied);
    }
    if (!state_.valid()) {
      CORIO_THROW<std::future_error>(std::future_errc::no_state);
    }
    state_.set_exception(std::move(p));
  }

protected:
  state_type_ state_;
  bool future_already_retrieved_;
}; // class promise_base_

} // namespace detail_

template<typename R, typename ExecutionContext>
class basic_promise
  : private detail_::promise_base_<R, ExecutionContext>
{
private:
  using base_type_ = detail_::promise_base_<R, ExecutionContext>;
  using base_type_::state_;

public:
  using typename base_type_::context_type;

  explicit basic_promise(context_type &ctx)
    : base_type_(ctx)
  {}

  void swap(basic_promise &rhs) noexcept
  {
    base_type_::swap(rhs);
  }

  friend void swap(basic_promise &lhs, basic_promise &rhs) noexcept
  {
    lhs.swap(rhs);
  }

  using base_type_::get_future;

  void set_value(R const &value)
  {
    if (state_.ready()) {
      CORIO_THROW<std::future_error>(std::future_errc::promise_already_satisfied);
    }
    if (!state_.valid()) {
      CORIO_THROW<std::future_error>(std::future_errc::no_state);
    }
    state_.set_value(value);
  }

  void set_value(R &&value)
  {
    if (state_.ready()) {
      CORIO_THROW<std::future_error>(std::future_errc::promise_already_satisfied);
    }
    if (!state_.valid()) {
      CORIO_THROW<std::future_error>(std::future_errc::no_state);
    }
    state_.set_value(std::move(value));
  }

  using base_type_::set_exception;
}; // class basic_promise

template<typename R, typename ExecutionContext>
class basic_promise<R &, ExecutionContext>
  : private detail_::promise_base_<R &, ExecutionContext>
{
private:
  using base_type_ = detail_::promise_base_<R &, ExecutionContext>;
  using base_type_::state_;

public:
  using typename base_type_::context_type;

  explicit basic_promise(context_type &ctx)
    : base_type_(ctx)
  {}

  void swap(basic_promise &rhs) noexcept
  {
    base_type_::swap(rhs);
  }

  friend void swap(basic_promise &lhs, basic_promise &rhs)
  {
    lhs.swap(rhs);
  }

  using base_type_::get_future;

  void set_value(R &value)
  {
    if (state_.ready()) {
      CORIO_THROW<std::future_error>(std::future_errc::promise_already_satisfied);
    }
    if (!state_.valid()) {
      CORIO_THROW<std::future_error>(std::future_errc::no_state);
    }
    state_.set_value(value);
  }

  using base_type_::set_exception;
}; // class basic_promise<R &, ExecutionContext>

template<typename ExecutionContext>
class basic_promise<void, ExecutionContext>
  : private detail_::promise_base_<void, ExecutionContext>
{
private:
  using base_type_ = detail_::promise_base_<void, ExecutionContext>;
  using base_type_::state_;

public:
  using typename base_type_::context_type;

  explicit basic_promise(context_type &ctx)
    : base_type_(ctx)
  {}

  void swap(basic_promise &rhs) noexcept
  {
    base_type_::swap(rhs);
  }

  friend void swap(basic_promise &lhs, basic_promise &rhs) noexcept
  {
    lhs.swap(rhs);
  }

  using base_type_::get_future;

  void set_value()
  {
    if (state_.ready()) {
      CORIO_THROW<std::future_error>(std::future_errc::promise_already_satisfied);
    }
    if (!state_.valid()) {
      CORIO_THROW<std::future_error>(std::future_errc::no_state);
    }
    state_.set_value();
  }

  using base_type_::set_exception;
}; // class basic_promise<void, ExecutionContext>

} // namespace corio::thread_unsafe

#endif // !defined(CORIO_THREAD_UNSAFE_PROMISE_HPP_INCLUDE_GUARD)
