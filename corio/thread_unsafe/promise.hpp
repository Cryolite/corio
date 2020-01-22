#if !defined(CORIO_THREAD_UNSAFE_PROMISE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_PROMISE_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/future.hpp>
#include <corio/thread_unsafe/detail_/shared_future_state_.hpp>
#include <corio/core/enable_if_execution_context.hpp>
#include <corio/core/error.hpp>
#include <corio/util/throw.hpp>
#include <boost/asio/executor.hpp>
#include <boost/config.hpp>
#include <type_traits>
#include <functional>
#include <variant>
#include <utility>
#include <exception>


namespace corio::thread_unsafe{

namespace detail_{

template<typename R, typename Executor>
class promise_mixin_
{
private:
  static_assert(!std::is_const_v<R>);
  static_assert(!std::is_volatile_v<R>);
  static_assert(!std::is_rvalue_reference_v<R>);

protected:
  using executor_type = Executor;
  static_assert(corio::is_executor_v<executor_type>);

private:
  using value_impl_type_ = std::conditional_t<
    std::is_lvalue_reference_v<R>,
    std::reference_wrapper<std::remove_reference_t<R> >,
    std::conditional_t<std::is_void_v<R>, std::monostate, R> >;
  using state_type_ = corio::thread_unsafe::detail_::shared_future_state_<value_impl_type_, executor_type>;

protected:
  using future_type = corio::thread_unsafe::basic_future<R, executor_type>;

  promise_mixin_()
    : state_(),
      future_already_retrieved_()
  {}

  explicit promise_mixin_(executor_type &&executor)
    : state_(std::move(executor)),
      future_already_retrieved_()
  {}

  promise_mixin_(promise_mixin_ const &) = delete;

  promise_mixin_(promise_mixin_ &&rhs) noexcept
    : state_(std::move(rhs.state_)),
      future_already_retrieved_(rhs.future_already_retrieved_)
  {
    rhs.future_already_retrieved_ = false;
  }

  ~promise_mixin_()
  {
    if (future_already_retrieved_ && !state_.ready()) {
      std::exception_ptr p = std::make_exception_ptr(corio::broken_promise_error());
      state_.set_exception(std::move(p));
    }
  }

  void swap(promise_mixin_ &rhs) noexcept
  {
    using std::swap;
    swap(state_, rhs.state_);
    swap(future_already_retrieved_, rhs.future_already_retrieved_);
  }

  promise_mixin_ &operator=(promise_mixin_ const &) = delete;

  promise_mixin_ &operator=(promise_mixin_ &&rhs) noexcept
  {
    promise_mixin_(std::move(rhs)).swap(*this);
    return *this;
  }

  bool has_executor() const
  {
    if (BOOST_UNLIKELY(!state_.valid())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::no_future_state_error>();
    }
    return state_.has_executor();
  }

  void set_executor(executor_type const &executor)
  {
    if (BOOST_UNLIKELY(!state_.valid())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::no_future_state_error>();
    }
    state_.set_executor(executor);
  }

  void set_executor(executor_type &&executor)
  {
    if (BOOST_UNLIKELY(!state_.valid())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::no_future_state_error>();
    }
    state_.set_executor(std::move(executor));
  }

  executor_type get_executor() const
  {
    if (BOOST_UNLIKELY(!state_.valid())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::no_future_state_error>();
    }
    return state_.get_executor();
  }

  future_type get_future()
  {
    if (BOOST_UNLIKELY(future_already_retrieved_)) /*[[unlikely]]*/ {
      CORIO_THROW<corio::future_already_retrieved_error>();
    }
    if (BOOST_UNLIKELY(!state_.valid())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::no_future_state_error>();
    }
    future_type future(state_);
    future_already_retrieved_ = true;
    return future;
  }

  void set_exception(std::exception_ptr p)
  {
    if (BOOST_UNLIKELY(state_.ready())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::promise_already_satisfied_error>();
    }
    if (BOOST_UNLIKELY(!state_.valid())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::no_future_state_error>();
    }
    state_.set_exception(std::move(p));
  }

protected:
  state_type_ state_;
  bool future_already_retrieved_ = false;
}; // class promise_mixin_

} // namespace detail_

template<typename R, typename Executor>
class basic_promise
  : private detail_::promise_mixin_<R, Executor>
{
public:
  using executor_type = Executor;
  static_assert(corio::is_executor_v<executor_type>);

private:
  using mixin_type_ = detail_::promise_mixin_<R, executor_type>;
  using mixin_type_::state_;

public:
  using typename mixin_type_::future_type;

  basic_promise() = default;

  explicit basic_promise(executor_type const &executor)
    : basic_promise(executor_type(executor))
  {}

  explicit basic_promise(executor_type &&executor)
    : mixin_type_(std::move(executor))
  {}

  template<typename ExecutionContext>
  explicit basic_promise(
    ExecutionContext &ctx,
    corio::enable_if_execution_context_t<ExecutionContext> * = nullptr,
    corio::disable_if_executor_t<ExecutionContext> * = nullptr)
    : basic_promise(ctx.get_executor())
  {}

  void swap(basic_promise &rhs) noexcept
  {
    mixin_type_::swap(rhs);
  }

  friend void swap(basic_promise &lhs, basic_promise &rhs) noexcept
  {
    lhs.swap(rhs);
  }

  using mixin_type_::has_executor;

  using mixin_type_::set_executor;

  using mixin_type_::get_executor;

  using mixin_type_::get_future;

  void set_value(R const &value)
  {
    if (BOOST_UNLIKELY(state_.ready())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::promise_already_satisfied_error>();
    }
    if (BOOST_UNLIKELY(!state_.valid())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::no_future_state_error>();
    }
    state_.set_value(value);
  }

  void set_value(R &&value)
  {
    if (BOOST_UNLIKELY(state_.ready())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::promise_already_satisfied_error>();
    }
    if (BOOST_UNLIKELY(!state_.valid())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::no_future_state_error>();
    }
    state_.set_value(std::move(value));
  }

  using mixin_type_::set_exception;
}; // class basic_promise

template<typename R, typename Executor>
class basic_promise<R &, Executor>
  : private detail_::promise_mixin_<R &, Executor>
{
public:
  using executor_type = Executor;
  static_assert(corio::is_executor_v<executor_type>);

private:
  using mixin_type_ = detail_::promise_mixin_<R &, executor_type>;
  using mixin_type_::state_;

public:
  using typename mixin_type_::future_type;

  basic_promise() = default;

  explicit basic_promise(executor_type const &executor)
    : basic_promise(executor_type(executor))
  {}

  explicit basic_promise(executor_type &&executor)
    : mixin_type_(std::move(executor))
  {}

  template<typename ExecutionContext>
  explicit basic_promise(
    ExecutionContext &ctx,
    corio::enable_if_execution_context_t<ExecutionContext> * = nullptr,
    corio::disable_if_executor_t<ExecutionContext> * = nullptr)
    : basic_promise(ctx.get_executor())
  {}

  void swap(basic_promise &rhs) noexcept
  {
    mixin_type_::swap(rhs);
  }

  friend void swap(basic_promise &lhs, basic_promise &rhs)
  {
    lhs.swap(rhs);
  }

  using mixin_type_::has_executor;

  using mixin_type_::set_executor;

  using mixin_type_::get_executor;

  using mixin_type_::get_future;

  void set_value(R &value)
  {
    if (BOOST_UNLIKELY(state_.ready())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::promise_already_satisfied_error>();
    }
    if (BOOST_UNLIKELY(!state_.valid())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::no_future_state_error>();
    }
    state_.set_value(std::ref(value));
  }

  using mixin_type_::set_exception;
}; // class basic_promise<R &, Executor>

template<typename Executor>
class basic_promise<void, Executor>
  : private detail_::promise_mixin_<void, Executor>
{
public:
  using executor_type = Executor;
  static_assert(corio::is_executor_v<executor_type>);

private:
  using mixin_type_ = detail_::promise_mixin_<void, executor_type>;
  using mixin_type_::state_;

public:
  using typename mixin_type_::future_type;

  basic_promise() = default;

  explicit basic_promise(executor_type const &executor)
    : basic_promise(executor_type(executor))
  {}

  explicit basic_promise(executor_type &&executor)
    : mixin_type_(std::move(executor))
  {}

  template<typename ExecutionContext>
  explicit basic_promise(
    ExecutionContext &ctx,
    corio::enable_if_execution_context_t<ExecutionContext> * = nullptr,
    corio::disable_if_executor_t<ExecutionContext> * = nullptr)
    : basic_promise(ctx.get_executor())
  {}

  void swap(basic_promise &rhs) noexcept
  {
    mixin_type_::swap(rhs);
  }

  friend void swap(basic_promise &lhs, basic_promise &rhs) noexcept
  {
    lhs.swap(rhs);
  }

  using mixin_type_::has_executor;

  using mixin_type_::set_executor;

  using mixin_type_::get_executor;

  using mixin_type_::get_future;

  void set_value()
  {
    if (BOOST_UNLIKELY(state_.ready())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::promise_already_satisfied_error>();
    }
    if (BOOST_UNLIKELY(!state_.valid())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::no_future_state_error>();
    }
    state_.set_value(std::monostate());
  }

  using mixin_type_::set_exception;
}; // class basic_promise<void, Executor>

template<typename R>
using promise = basic_promise<R, boost::asio::executor>;

} // namespace corio::thread_unsafe

#endif // !defined(CORIO_THREAD_UNSAFE_PROMISE_HPP_INCLUDE_GUARD)
