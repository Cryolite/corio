#if !defined(CORIO_THREAD_UNSAFE_DETAIL_COROUTINE_PROMISE_FWD_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_DETAIL_COROUTINE_PROMISE_FWD_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/coroutine_fwd.hpp>
#include <corio/thread_unsafe/detail_/shared_coroutine_state_.hpp>
#include <experimental/coroutine>


namespace corio::thread_unsafe::detail_{

template<typename R>
class coroutine_promise_;

template<typename R>
class coroutine_promise_base_
{
protected:
  using handle_type_ = std::experimental::coroutine_handle<coroutine_promise_<R> >;
  using state_type_ = corio::thread_unsafe::detail_::shared_coroutine_state_<R>;

  using coroutine_type = corio::thread_unsafe::coroutine<R>;

  coroutine_promise_base_() noexcept;

  coroutine_promise_base_(coroutine_promise_base_ const &) = delete;

  ~coroutine_promise_base_();

  coroutine_promise_base_ &operator=(coroutine_promise_base_ const &) = delete;

  std::experimental::suspend_always initial_suspend();

  void unhandled_exception();

  std::experimental::suspend_always final_suspend();

protected:
  state_type_ *p_;
}; // class coroutine_promise_base_

template<typename R>
class coroutine_promise_
  : private detail_::coroutine_promise_base_<R>
{
private:
  using base_type_ = detail_::coroutine_promise_base_<R>;
  using typename base_type_::handle_type_;
  using typename base_type_::state_type_;
  using base_type_::p_;

public:
  using typename base_type_::coroutine_type;

  coroutine_promise_() noexcept;

  coroutine_type get_return_object();

  using base_type_::initial_suspend;

  using base_type_::unhandled_exception;

  void return_value(R const &value);

  void return_value(R &&value);

  using base_type_::final_suspend;
}; // class coroutine_promise_

template<typename R>
class coroutine_promise_<R &>
  : protected detail_::coroutine_promise_base_<R &>
{
private:
  using base_type_ = detail_::coroutine_promise_base_<R &>;
  using typename base_type_::handle_type_;
  using typename base_type_::state_type_;
  using base_type_::p_;

public:
  using typename base_type_::coroutine_type;

  coroutine_promise_() noexcept;

  coroutine_type get_return_object();

  using base_type_::initial_suspend;

  using base_type_::unhandled_exception;

  void return_value(R &value);

  using base_type_::final_suspend;
}; // class coroutine_promise_<R &>

template<>
class coroutine_promise_<void>
  : protected detail_::coroutine_promise_base_<void>
{
private:
  using base_type_ = detail_::coroutine_promise_base_<void>;
  using typename base_type_::handle_type_;
  using typename base_type_::state_type_;
  using base_type_::p_;

public:
  using typename base_type_::coroutine_type;

  coroutine_promise_() noexcept;

  coroutine_type get_return_object();

  using base_type_::initial_suspend;

  using base_type_::unhandled_exception;

  void return_void();

  using base_type_::final_suspend;
}; // class coroutine_promise_<void>

} // namespace corio::thread_unsafe::detail_

#endif // !defined(CORIO_THREAD_UNSAFE_DETAIL_COROUTINE_PROMISE_FWD_HPP_INCLUDE_GUARD)
