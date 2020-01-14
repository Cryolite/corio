#if !defined(CORIO_THREAD_UNSAFE_PROMISE_FWD_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_PROMISE_FWD_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/coroutine_fwd.hpp>
#include <corio/thread_unsafe/detail_/shared_coroutine_state_.hpp>
#include <experimental/coroutine>


namespace corio::thread_unsafe{

template<typename R>
class promise;

namespace detail_{

template<typename R>
class promise_base_
{
protected:
  using handle_type_ = std::experimental::coroutine_handle<promise<R> >;
  using state_type_ = corio::thread_unsafe::detail_::shared_coroutine_state_<R>;

  using coroutine_type = corio::thread_unsafe::coroutine<R>;

  promise_base_() noexcept;

  promise_base_(promise_base_ const &) = delete;

  ~promise_base_();

  promise_base_ &operator=(promise_base_ const &) = delete;

  std::experimental::suspend_always initial_suspend();

  void unhandled_exception();

  std::experimental::suspend_always final_suspend();

protected:
  state_type_ *p_;
}; // class promise_base_

} // namespace detail_

template<typename R>
class promise
  : protected detail_::promise_base_<R>
{
private:
  using base_type_ = detail_::promise_base_<R>;
  using typename base_type_::handle_type_;
  using typename base_type_::state_type_;
  using base_type_::p_;

public:
  using typename base_type_::coroutine_type;

  promise() noexcept;

  coroutine_type get_return_object();

  using base_type_::initial_suspend;

  void return_value(R const &value);

  void return_value(R &&value);

  using base_type_::unhandled_exception;

  using base_type_::final_suspend;
}; // class promise

template<typename R>
class promise<R &>
  : protected detail_::promise_base_<R &>
{
private:
  using base_type_ = detail_::promise_base_<R &>;
  using typename base_type_::handle_type_;
  using typename base_type_::state_type_;
  using base_type_::p_;

public:
  using typename base_type_::coroutine_type;

  promise() noexcept;

  coroutine_type get_return_object();

  using base_type_::initial_suspend;

  void return_value(R &value);

  using base_type_::unhandled_exception;

  using base_type_::final_suspend;
}; // class promise<R &>

template<>
class promise<void>
  : protected detail_::promise_base_<void>
{
private:
  using base_type_ = detail_::promise_base_<void>;
  using typename base_type_::handle_type_;
  using typename base_type_::state_type_;
  using base_type_::p_;

public:
  using typename base_type_::coroutine_type;

  promise() noexcept;

  coroutine_type get_return_object();

  using base_type_::initial_suspend;

  void return_void();

  using base_type_::unhandled_exception;

  using base_type_::final_suspend;
}; // class promise<void>

} // namespace corio

#endif // !defined(CORIO_CORE_PROMISE_FWD_HPP_INCLUDE_GUARD)
