#if !defined(CORIO_THREAD_UNSAFE_DETAIL_COROUTINE_PROMISE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_DETAIL_COROUTINE_PROMISE_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/detail_/coroutine_promise_fwd_.hpp>
#include <corio/util/assert.hpp>
#include <utility>
#include <experimental/coroutine>
#include <exception>


namespace corio::thread_unsafe::detail_{

template<typename R>
coroutine_promise_base_<R>::coroutine_promise_base_() noexcept
  : p_()
{}

template<typename R>
coroutine_promise_base_<R>::~coroutine_promise_base_()
{
  if (p_ != nullptr && p_->decrement_reference_count() == 0u) {
    delete p_;
  }
}

template<typename R>
std::experimental::suspend_always coroutine_promise_base_<R>::initial_suspend()
{
  CORIO_ASSERT(p_ != nullptr);
  return {};
}

template<typename R>
void coroutine_promise_base_<R>::unhandled_exception()
{
  CORIO_ASSERT(p_ != nullptr);
  p_->set_exception(std::current_exception());
}

template<typename R>
std::experimental::suspend_always coroutine_promise_base_<R>::final_suspend()
{
  CORIO_ASSERT(p_ != nullptr);
  return {};
}

template<typename R>
coroutine_promise_<R>::coroutine_promise_() noexcept
  : base_type_()
{}

template<typename R>
auto coroutine_promise_<R>::get_return_object() -> coroutine_type
{
  CORIO_ASSERT(p_ == nullptr);
  p_ = new state_type_();
  p_->increment_reference_count();
  return { handle_type_::from_promise(*this), p_ };
}

template<typename R>
void coroutine_promise_<R>::return_value(R const &value)
{
  CORIO_ASSERT(p_ != nullptr);
  p_->set_value(value);
}

template<typename R>
void coroutine_promise_<R>::return_value(R &&value)
{
  CORIO_ASSERT(p_ != nullptr);
  p_->set_value(std::move(value));
}

template<typename R>
coroutine_promise_<R &>::coroutine_promise_() noexcept
  : base_type_()
{}

template<typename R>
auto coroutine_promise_<R &>::get_return_object() -> coroutine_type
{
  CORIO_ASSERT(p_ == nullptr);
  p_ = new state_type_();
  p_->increment_reference_count();
  return { handle_type_::from_promise(*this), p_ };
}

template<typename R>
void coroutine_promise_<R &>::return_value(R &value)
{
  CORIO_ASSERT(p_ != nullptr);
  p_->set_value(value);
}

coroutine_promise_<void>::coroutine_promise_() noexcept
  : base_type_()
{}

auto coroutine_promise_<void>::get_return_object() -> coroutine_type
{
  CORIO_ASSERT(p_ == nullptr);
  p_ = new state_type_();
  p_->increment_reference_count();
  return { handle_type_::from_promise(*this), p_ };
}

void coroutine_promise_<void>::return_void()
{
  CORIO_ASSERT(p_ != nullptr);
}

} // namespace corio::thread_unsafe::detail_

#endif // !defined(CORIO_THREAD_UNSAFE_DETAIL_COROUTINE_PROMISE_HPP_INCLUDE_GUARD)
