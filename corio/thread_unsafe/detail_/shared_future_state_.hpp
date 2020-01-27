#if !defined(CORIO_THREAD_UNSAFE_DETAIL_SHARED_FUTURE_STATE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_DETAIL_SHARED_FUTURE_STATE_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/mutex.hpp>
#include <corio/core/is_executor.hpp>
#include <corio/util/expected.hpp>
#include <corio/util/assert.hpp>
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
    using mutex_type_ = corio::thread_unsafe::basic_mutex<executor_type>;
    using lock_type_ = typename mutex_type_::lock_type;

  public:
    impl_()
      : mtx_(),
        value_(),
        refcount_(0u)
    {
      mtx_.try_lock();
    }

    explicit impl_(executor_type &&executor)
      : mtx_(std::move(executor)),
        value_(),
        refcount_(0u)
    {
      mtx_.try_lock();
    }

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

    bool has_executor() const noexcept
    {
      CORIO_ASSERT(refcount_ > 0u);
      return mtx_.has_executor();
    }

    void set_executor(executor_type &&executor)
    {
      CORIO_ASSERT(refcount_ > 0u);
      mtx_.set_executor(std::move(executor));
    }

    executor_type get_executor() const
    {
      CORIO_ASSERT(refcount_ > 0u);
      return mtx_.get_executor();
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
      CORIO_ASSERT(!mtx_.try_lock());
      static_assert(sizeof...(Args) <= 1u);
      value_.emplace(std::forward<Args>(args)...);
      mtx_.unlock();
    }

    void set_exception(std::exception_ptr p)
    {
      CORIO_ASSERT(!ready());
      CORIO_ASSERT(!mtx_.try_lock());
      value_.set_exception(std::move(p));
      mtx_.unlock();
    }

    template<typename CompletionHandler>
    void async_get(CompletionHandler &&h)
    {
      CORIO_ASSERT(refcount_ > 0u);
      // A NOTE ON THE LIFETIME OF `value_`. Since `value_` is captured by
      // reference, someone must guarantee that the object pointed to by `this`
      // lives until the following callback is invoked. This condition is
      // always satisfied because, only `basic_promise::set_value` or
      // `basic_promise::set_exception` invokes the callback and the object of
      // the type `basic_promise` shares the ownership of the object pointed to
      // by `this`.
      mtx_.async_lock(
        [h_ = std::move(h), &value = value_](lock_type_ lock) mutable -> void{
          lock = lock_type_();
          std::move(h_)(std::move(value));
        });
    }

    template<typename CompletionHandler>
    void async_wait(CompletionHandler &&h)
    {
      CORIO_ASSERT(refcount_ > 0u);
      mtx_.async_lock(
        [h_ = std::move(h)](lock_type_ lock) mutable -> void{
          lock = lock_type_();
          std::move(h_)();
        });
    }

    R get()
    {
      CORIO_ASSERT(refcount_ > 0u);
      return corio::get(value_);
    }

  private:
    mutex_type_ mtx_;
    expected<R> value_;
    std::size_t refcount_;
  }; // class impl_

public:
  shared_future_state_()
    : p_(new impl_())
  {
    p_->acquire();
  }

  explicit shared_future_state_(std::nullptr_t) noexcept
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

  bool has_executor() const
  {
    CORIO_ASSERT(p_ != nullptr);
    return p_->has_executor();
  }

  void set_executor(executor_type &&executor)
  {
    CORIO_ASSERT(p_ != nullptr);
    p_->set_executor(std::move(executor));
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
    p_->async_get(
      [h_ = std::move(h)](corio::expected<R> value) mutable -> void{
        std::move(h_)(std::move(value));
      });
  }

  template<typename CompletionHandler>
  void async_wait(CompletionHandler &&h)
  {
    CORIO_ASSERT(p_ != nullptr);
    p_->async_wait(
      [h_ = std::move(h)]() mutable -> void{
        std::move(h_)();
      });
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
