#if !defined(CORIO_THREAD_UNSAFE_PROMISE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_PROMISE_HPP_INCLUDE_GUARD

//#include <corio/thread_unsafe/future.hpp>
#include <corio/thread_unsafe/detail_/shared_future_state_.hpp>
#include <corio/util/throw.hpp>
#include <boost/asio/execution_context.hpp>
#include <future>
#include <type_traits>
#include <utility>
#include <exception>


namespace corio::thread_unsafe{

namespace detail_{

template<typename R, typename ExecutionContext = void>
class promise_base_
{
protected:
  using context_type = ExecutionContext;

  static_assert(!std::is_const_v<context_type>);
  static_assert(!std::is_volatile_v<context_type>);
  static_assert(!std::is_reference_v<context_type>);
  static_assert(std::is_convertible_v<context_type &, boost::asio::execution_context &>);

private:
  using state_type_ = corio::thread_unsafe::detail_::shared_future_state_<R>;

protected:
  promise_base_()
    : state_(),
      p_ctx_(),
      future_already_retrieved_()
  {}

  explicit promise_base_(context_type &ctx)
    : state_(),
      p_ctx_(&ctx),
      future_already_retrieved_()
  {}

  promise_base_(promise_base_ const &) = delete;

  promise_base_(promise_base_ &&rhs) noexcept
    : state_(std::move(rhs.state_)),
      p_ctx_(rhs.p_ctx_)
      future_already_retrieved_(rhs.future_already_retrieved_)
  {
    rhs.p_ctx_ = nullptr;
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
    swap(p_ctx_, rhs.p_ctx_);
    swap(future_already_retrieved_, rhs.future_already_retrieved_);
  }

  promise_base_ &operator=(promise_base_ const &) = delete;

  promise_base_ &operator=(promise_base_ &&rhs) noexcept
  {
    promise_base_(std::move(rhs)).swap(*this);
    return *this;
  }

  context_type *context() noexcept
  {
    if (!state_.valid()) {
      CORIO_THROW<std::future_error>(std::future_errc::no_state);
    }
    return p_ctx_;
  }

  corio::thread_unsafe::future<R, context_type> get_future()
  {
    if (future_already_retrieved_) {
      CORIO_THROW<std::future_error>(std::future_errc::future_already_retrieved);
    }
    if (!state_.valid()) {
      CORIO_THROW<std::future_error>(std::future_errc::no_state);
    }
    corio::thread_unsafe::future<R, context_type> future(state_, p_ctx_);
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
    state_.set_exception_(std::move(p), p_ctx_);
  }

protected:
  state_type_ state_;
  bool future_already_retrieved_;
}; // class promise_base_

} // namespace detail_

template<typename R, typename ExecutionContext>
class promise
  : protected detail_::promise_base_<R, ExecutionContext>
{
private:
  using base_type_ = promise_base_<R>;

public:
  using typename base_type_::context_type;

  promise()
    : base_type_()
  {}

  friend void swap(promise &lhs, promise &rhs)
  {
    lhs.swap(rhs);
  }

  using base_type_::context;

  using base_type_::get_future;

  void set_value(R const &value)
  {
    if (state_.ready()) {
      CORIO_THROW<std::future_error>(std::future_errc::promise_already_satisfied);
    }
    if (!state_.valid()) {
      CORIO_THROW<std::future_error>(std::future_errc::no_state);
    }
    state_.set_value(value, p_ctx_);
  }

  void set_value(R &&value)
  {
    if (state_.ready()) {
      CORIO_THROW<std::future_error>(std::future_errc::promise_already_satisfied);
    }
    if (!state_.valid()) {
      CORIO_THROW<std::future_error>(std::future_errc::no_state);
    }
    state_.set_value(std::move(value), p_ctx_);
  }

  using base_type_::set_exception;
}; // class promise

template<typename R, typename ExecutionContext>
class promise
  : protected detail_::promise_base_<R &, ExecutionContext>
{
private:
  using base_type_ = promise_base_<R &>;

public:
  using typename base_type_::context_type;

  promise()
    : base_type_()
  {}

  friend void swap(promise &lhs, promise &rhs)
  {
    lhs.swap(rhs);
  }

  using base_type_::context;

  using base_type_::get_future;

  void set_value(R &value)
  {
    if (state_.ready()) {
      CORIO_THROW<std::future_error>(std::future_errc::promise_already_satisfied);
    }
    if (!state_.valid()) {
      CORIO_THROW<std::future_error>(std::future_errc::no_state);
    }
    state_.set_value(value, p_ctx_);
  }

  using base_type_::set_exception;
}; // class promise<R &, ExecutionContext>

template<typename ExecutionContext>
class promise
  : protected detail_::promise_base_<void, ExecutionContext>
{
private:
  using base_type_ = promise_base_<void>;

public:
  using typename base_type_::context_type;

  promise()
    : base_type_()
  {}

  friend void swap(promise &lhs, promise &rhs)
  {
    lhs.swap(rhs);
  }

  using base_type_::context;

  using base_type_::get_future;

  void set_value()
  {
    if (state_.ready()) {
      CORIO_THROW<std::future_error>(std::future_errc::promise_already_satisfied);
    }
    if (!state_.valid()) {
      CORIO_THROW<std::future_error>(std::future_errc::no_state);
    }
    state_.set_value(p_ctx_);
  }

  using base_type_::set_exception;
}; // class promise<void, ExecutionContext>

} // namespace corio::thread_unsafe

#endif // !defined(CORIO_THREAD_UNSAFE_PROMISE_HPP_INCLUDE_GUARD)
