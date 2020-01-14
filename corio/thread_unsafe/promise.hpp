#if !defined(CORIO_THREAD_UNSAFE_PROMISE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_PROMISE_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/promise_fwd.hpp>
#include <corio/util/assert.hpp>
#include <utility>
#include <experimental/coroutine>
#include <exception>


namespace corio::thread_unsafe{

namespace detail_{

template<typename R>
promise_base_<R>::promise_base_() noexcept
  : p_()
{}

template<typename R>
promise_base_<R>::~promise_base_()
{
  if (p_ != nullptr && p_->decrement_reference_count() == 0u) {
    delete p_;
  }
}

template<typename R>
std::experimental::suspend_always promise_base_<R>::initial_suspend()
{
  return {};
}

template<typename R>
void promise_base_<R>::unhandled_exception()
{
  CORIO_ASSERT(p_ != nullptr);
  p_->set_exception(std::current_exception());
}

template<typename R>
std::experimental::suspend_always promise_base_<R>::final_suspend()
{
  return {};
}

} // namespace detail_

template<typename R>
promise<R>::promise() noexcept
  : base_type_()
{}

template<typename R>
auto promise<R>::get_return_object() -> coroutine_type
{
  CORIO_ASSERT(p_ == nullptr);
  p_ = new state_type_();
  p_->increment_reference_count();
  return { handle_type_::from_promise(*this), p_ };
}

template<typename R>
void promise<R>::return_value(R const &value)
{
  CORIO_ASSERT(p_ != nullptr);
  p_->set_value(value);
}

template<typename R>
void promise<R>::return_value(R &&value)
{
  CORIO_ASSERT(p_ != nullptr);
  p_->set_value(std::move(value));
}

template<typename R>
promise<R &>::promise() noexcept
  : base_type_()
{}

template<typename R>
auto promise<R &>::get_return_object() -> coroutine_type
{
  CORIO_ASSERT(p_ == nullptr);
  p_ = new state_type_();
  p_->increment_reference_count();
  return { handle_type_::from_promise(*this), p_ };
}

template<typename R>
void promise<R &>::return_value(R &value)
{
  CORIO_ASSERT(p_ != nullptr);
  p_->set_value(value);
}

promise<void>::promise() noexcept
  : base_type_()
{}

auto promise<void>::get_return_object() -> coroutine_type
{
  CORIO_ASSERT(p_ == nullptr);
  p_ = new state_type_();
  p_->increment_reference_count();
  return { handle_type_::from_promise(*this), p_ };
}

void promise<void>::return_void()
{
  CORIO_ASSERT(p_ != nullptr);
}

} // namespace corio::thread_unsafe

#endif // !defined(CORIO_CORE_PROMISE_HPP_INCLUDE_GUARD)
