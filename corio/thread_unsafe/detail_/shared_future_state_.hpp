#if !defined(CORIO_THREAD_UNSAFE_DETAIL_SHARED_FUTURE_STATE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_DETAIL_SHARED_FUTURE_STATE_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/condition_variable.hpp>
#include <corio/core/is_executor.hpp>
#include <corio/util/expected.hpp>
#include <corio/util/assert.hpp>
#include <future>
#include <condition_variable>
#include <chrono>
#include <type_traits>
#include <utility>
#include <exception>
#include <cstddef>


namespace corio::thread_unsafe::detail_{

template<typename R, typename Executor>
class shared_future_state_
{
private:
  static_assert(!std::is_const_v<R>);
  static_assert(!std::is_volatile_v<R>);
  static_assert(!std::is_rvalue_reference_v<R>);

public:
  using executor_type = Executor;
  static_assert(corio::is_executor_v<executor_type>);

private:
  class impl_
  {
  private:
    using cv_type_ = corio::thread_unsafe::basic_condition_variable<executor_type>;

  public:
    explicit impl_(executor_type &&executor)
      : value_(),
        cv_(std::move(executor)),
        refcount_()
    {}

    impl_(impl_ const &) = delete;

    impl_ &operator=(impl_ const &) = delete;

    void acquire() noexcept
    {
      ++refcount_;
    }

    std::size_t release() noexcept
    {
      CORIO_ASSERT(refcount_ > 0u);
      return --refcount_;
    }

    executor_type get_executor() const
    {
      CORIO_ASSERT(refcount_ > 0u);
      return cv_.get_executor();
    }

    bool ready() const noexcept
    {
      CORIO_ASSERT(refcount_ > 0u);
      return !!value_;
    }

    template<typename... Args>
    void set_value(Args &&... args)
    {
      CORIO_ASSERT(!ready());
      static_assert(sizeof...(Args) <= 1u);
      value_.emplace(std::forward<Args>(args)...);
      cv_.notify_all();
    }

    void set_exception(std::exception_ptr p)
    {
      CORIO_ASSERT(!ready());
      value_.set_exception(std::move(p));
      cv_.notify_all();
    }

    template<typename CompletionHandler>
    void async_get(CompletionHandler &&h)
    {
      CORIO_ASSERT(refcount_ > 0u);
      // A NOTE ON THE LIFETIME OF `*this`. Since `this` is captured by the
      // following closure types, someone must guarantee that the object
      // pointed to by `this` lives until the callback `h` is invoked. This
      // condition is always satisfied because, only `basic_promise::set_value`
      // or `basic_promise::set_exception` invokes the callback and the object
      // of the type `basic_promise` shares the ownership of the object pointed
      // to by `this`.
      cv_.async_wait(
        [this]() -> bool{ return ready(); },
        [h_ = std::move(h), this]() -> void{
          std::move(h_)(std::move(value_));
        });
    }

    template<typename CompletionHandler>
    void async_wait(CompletionHandler &&h)
    {
      CORIO_ASSERT(refcount_ > 0u);
      cv_.async_wait([this]() -> bool{ return ready(); }, std::move(h));
    }

    template<typename Clock, typename Duration, typename CompletionHandler>
    void async_wait_until(std::chrono::time_point<Clock, Duration> const &abs_time, CompletionHandler &&h)
    {
      CORIO_ASSERT(refcount_ > 0u);
      cv_.async_wait_until(
        abs_time,
        [this]() -> bool{ return ready(); },
        [h_ = std::move(h)](std::cv_status status) mutable -> void{
          std::move(h_)(status == std::cv_status::no_timeout
                        ? std::future_status::ready
                        : std::future_status::timeout);
        });
    }

    R get()
    {
      CORIO_ASSERT(ready());
      return corio::get(value_);
    }

  private:
    corio::expected<R> value_;
    cv_type_ cv_;
    std::size_t refcount_;
  }; // class impl_

public:
  shared_future_state_() noexcept
    : p_()
  {}

  explicit shared_future_state_(executor_type &&executor)
    : p_(new impl_(std::move(executor)))
  {
    p_->acquire();
  }

  shared_future_state_(shared_future_state_ const &rhs) noexcept
    : p_(rhs.p_)
  {
    if (p_ != nullptr) {
      p_->acquire();
    }
  }

  shared_future_state_(shared_future_state_ &&rhs) noexcept
    : p_(rhs.p_)
  {
    rhs.p_ = nullptr;
  }

  ~shared_future_state_()
  {
    if (p_ != nullptr && p_->release() == 0u) {
      delete p_;
    }
  }

  void swap(shared_future_state_ &rhs) noexcept
  {
    using std::swap;
    swap(p_, rhs.p_);
  }

  friend void swap(shared_future_state_ &lhs, shared_future_state_ &rhs) noexcept
  {
    lhs.swap(rhs);
  }

  shared_future_state_ &operator=(shared_future_state_ const &rhs) noexcept
  {
    shared_future_state_(rhs).swap(*this);
    return *this;
  }

  shared_future_state_ &operator=(shared_future_state_ &&rhs) noexcept
  {
    shared_future_state_(std::move(rhs)).swap(*this);
    return *this;
  }

  executor_type get_executor() const
  {
    CORIO_ASSERT(p_ != nullptr);
    return p_->get_executor();
  }

  bool valid() const noexcept
  {
    return p_ != nullptr;
  }

  bool ready() const noexcept
  {
    return p_ != nullptr && p_->ready();
  }

  template<typename... Args>
  void set_value(Args &&... args)
  {
    CORIO_ASSERT(p_ != nullptr);
    static_assert(sizeof...(Args) <= 1u);
    p_->set_value(std::forward<Args>(args)...);
  }

  void set_exception(std::exception_ptr p)
  {
    CORIO_ASSERT(p_ != nullptr);
    p_->set_exception(std::move(p));
  }

  template<typename CompletionHandler>
  void async_get(CompletionHandler &&h)
  {
    CORIO_ASSERT(p_ != nullptr);
    p_->async_get(std::move(h));
  }

  template<typename CompletionHandler>
  void async_wait(CompletionHandler &&h) const
  {
    CORIO_ASSERT(p_ != nullptr);
    p_->async_wait(std::move(h));
  }

  template<typename Clock, typename Duration, typename CompletionHandler>
  void async_wait_until(std::chrono::time_point<Clock, Duration> const &abs_time,
                        CompletionHandler &&h) const
  {
    CORIO_ASSERT(p_ != nullptr);
    p_->async_wait_until(abs_time, std::move(h));
  }

  R get()
  {
    CORIO_ASSERT(p_ != nullptr);
    return p_->get();
  }

private:
  impl_ *p_;
}; // class shared_future_state_

} // namespace corio::thread_unsafe::detail_

#endif // !defined(CORIO_THREAD_UNSAFE_DETAIL_SHARED_FUTURE_STATE_HPP_INCLUDE_GUARD)
