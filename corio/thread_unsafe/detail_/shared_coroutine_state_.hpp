#if !defined(CORIO_THREADS_UNSAFE_DETAIL_SHARED_COROUTINE_STATE_HPP_INCLUDE_GUARD)
#define CORIO_THREADS_UNSAFE_DETAIL_SHARED_COROUTINE_STATE_HPP_INCLUDE_GUARD

#include <corio/util/assert.hpp>
#include <corio/util/scope_guard.hpp>
#include <optional>
#include <utility>
#include <exception>
#include <cstddef>


namespace corio::thread_unsafe::detail_{

template<typename R>
class shared_coroutine_state_
{
public:
  shared_coroutine_state_()
    : value_(std::nullopt),
      p_exception_(),
      reference_count_(0u)
  {}

  shared_coroutine_state_(shared_coroutine_state_ const &) = delete;

  shared_coroutine_state_ &operator=(shared_coroutine_state_ const &) = delete;

  void increment_reference_count() noexcept
  {
    ++reference_count_;
  }

  [[nodiscard]] std::size_t decrement_reference_count() noexcept
  {
    CORIO_ASSERT(reference_count_ > 0u);
    return --reference_count_;
  }

  void set_value(R const &value)
  {
    CORIO_ASSERT(reference_count_ > 0u);
    CORIO_ASSERT(!value_.has_value());
    CORIO_ASSERT(p_exception_ == nullptr);
    value_.emplace(value);
  }

  void set_value(R &&value)
  {
    CORIO_ASSERT(reference_count_ > 0u);
    CORIO_ASSERT(!value_.has_value());
    CORIO_ASSERT(p_exception_ == nullptr);
    value_.emplace(std::move(value));
  }

  void set_exception(std::exception_ptr p_exception)
  {
    CORIO_ASSERT(reference_count_ > 0u);
    CORIO_ASSERT(!value_.has_value());
    CORIO_ASSERT(p_exception_ == nullptr);
    p_exception_ = p_exception;
  }

  R get()
  {
    CORIO_ASSERT(reference_count_ > 0u);
    CORIO_ASSERT(value_.has_value() != (p_exception_ != nullptr));
    if (p_exception_ != nullptr) {
      CORIO_SCOPE_GUARD{
        p_exception_ = nullptr;
      };
      std::rethrow_exception(p_exception_);
    }
    CORIO_SCOPE_GUARD{
      value_ = std::nullopt;
    };
    return std::move(value_.value());
  }

private:
  std::optional<R> value_;
  std::exception_ptr p_exception_;
  std::size_t reference_count_;
}; // class shared_coroutine_state_

template<typename R>
class shared_coroutine_state_<R &>
{
public:
  shared_coroutine_state_()
    : p_(),
      p_exception_(),
      reference_count_(0u)
  {}

  shared_coroutine_state_(shared_coroutine_state_ const &) = delete;

  shared_coroutine_state_ &operator=(shared_coroutine_state_ const &) = delete;

  void increment_reference_count() noexcept
  {
    ++reference_count_;
  }

  [[nodiscard]] std::size_t decrement_reference_count() noexcept
  {
    CORIO_ASSERT(reference_count_ > 0u);
    return --reference_count_;
  }

  void set_value(R &value)
  {
    CORIO_ASSERT(reference_count_ > 0u);
    CORIO_ASSERT(p_ == nullptr);
    CORIO_ASSERT(p_exception_ == nullptr);
    p_ = &value;
  }

  void set_exception(std::exception_ptr p_exception)
  {
    CORIO_ASSERT(reference_count_ > 0u);
    CORIO_ASSERT(p_ == nullptr);
    CORIO_ASSERT(p_exception_ == nullptr);
    p_exception_ = p_exception;
  }

  R &get()
  {
    CORIO_ASSERT(reference_count_ > 0u);
    CORIO_ASSERT((p_ != nullptr) != (p_exception_ != nullptr));
    if (p_exception_ != nullptr) {
      CORIO_SCOPE_GUARD{
        p_exception_ = nullptr;
      };
      std::rethrow_exception(p_exception_);
    }
    CORIO_SCOPE_GUARD{
      p_ = nullptr;
    };
    return *p_;
  }

private:
  R *p_;
  std::exception_ptr p_exception_;
  std::size_t reference_count_;
}; // class shared_coroutine_state_

template<>
class shared_coroutine_state_<void>
{
public:
  shared_coroutine_state_()
    : p_exception_(),
      reference_count_(0u)
  {}

  shared_coroutine_state_(shared_coroutine_state_ const &) = delete;

  shared_coroutine_state_ &operator=(shared_coroutine_state_ const &) = delete;

  void increment_reference_count() noexcept
  {
    ++reference_count_;
  }

  [[nodiscard]] std::size_t decrement_reference_count() noexcept
  {
    CORIO_ASSERT(reference_count_ > 0u);
    return --reference_count_;
  }

  void set_exception(std::exception_ptr p_exception)
  {
    CORIO_ASSERT(reference_count_ > 0u);
    CORIO_ASSERT(p_exception_ == nullptr);
    p_exception_ = p_exception;
  }

  void get()
  {
    CORIO_ASSERT(reference_count_ > 0u);
    if (p_exception_ != nullptr) {
      CORIO_SCOPE_GUARD{
        p_exception_ = nullptr;
      };
      std::rethrow_exception(p_exception_);
    }
  }

private:
  std::exception_ptr p_exception_;
  std::size_t reference_count_;
}; // class shared_coroutine_state_

} // namespace corio::thread_unsafe::detail_

#endif // !defined(CORIO_THREADS_UNSAFE_DETAIL_SHARED_COROUTINE_STATE_HPP_INCLUDE_GUARD)
