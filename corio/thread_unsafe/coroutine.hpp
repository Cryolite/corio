#if !defined(CORIO_THREAD_UNSAFE_COROUTINE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_COROUTINE_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/coroutine_fwd.hpp>
#include <corio/util/assert.hpp>
#include <utility>


namespace corio::thread_unsafe{

namespace detail_{

template<typename R>
coroutine_base_<R>::coroutine_base_(handle_type_ &&handle, state_type_ *p) noexcept
  : handle_(std::move(handle)),
    p_(p)
{
  handle = nullptr;
  p_->increment_reference_count();
}

template<typename R>
coroutine_base_<R>::coroutine_base_() noexcept
  : handle_(),
    p_()
{}

template<typename R>
coroutine_base_<R>::coroutine_base_(coroutine_base_ &&rhs) noexcept
  : handle_(std::move(rhs.handle_)),
    p_(rhs.p_)
{
  rhs.handle_ = nullptr;
  rhs.p_ = nullptr;
}

template<typename R>
coroutine_base_<R>::~coroutine_base_()
{
  if (p_ != nullptr && p_->decrement_reference_count() == 0u) {
    delete p_;
  }
}

template<typename R>
void coroutine_base_<R>::swap(coroutine_base_ &rhs) noexcept
{
  using std::swap;
  swap(handle_, rhs.handle_);
  swap(p_, rhs.p_);
}

template<typename R>
auto coroutine_base_<R>::operator=(coroutine_base_ &&rhs) noexcept -> coroutine_base_ &
{
  coroutine_base_(std::move(rhs)).swap(*this);
  return *this;
}

template<typename R>
void coroutine_base_<R>::operator()()
{
  CORIO_ASSERT(handle_ != nullptr);
  CORIO_ASSERT(p_ != nullptr);
  handle_.resume();
}

template<typename R>
bool coroutine_base_<R>::done() const
{
  CORIO_ASSERT(handle_ != nullptr);
  CORIO_ASSERT(p_ != nullptr);
  return handle_.done();
}

} // namespace detail_

template<typename R>
coroutine<R>::coroutine(handle_type_ &&handle, state_type_ *p) noexcept
  : base_type_(std::move(handle), p)
{}

template<typename R>
coroutine<R &>::coroutine(handle_type_ &&handle, state_type_ *p) noexcept
  : base_type_(std::move(handle), p)
{}

coroutine<void>::coroutine(handle_type_ &&handle, state_type_ *p) noexcept
  : base_type_(std::move(handle), p)
{}

} // namespace corio::thread_unsafe

#endif // !defined(CORIO_THREAD_UNSAFE_COROUTINE_HPP_INCLUDE_GUARD)
