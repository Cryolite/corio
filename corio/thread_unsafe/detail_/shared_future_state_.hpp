#if !defined(CORIO_THREAD_UNSAFE_DETAIL_SHARED_FUTURE_STATE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_DETAIL_SHARED_FUTURE_STATE_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/mutex.hpp>
#include <corio/core/is_executor.hpp>
#include <corio/core/enable_if_execution_context.hpp>
#include <corio/util/assert.hpp>
#include <type_traits>
#include <variant>
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
  using async_get_argument_type = std::variant<R, std::exception_ptr>;

private:
  class impl_
  {
  private:
    using mutex_type_ = corio::thread_unsafe::basic_mutex<executor_type>;
    using lock_type_ = typename mutex_type_::lock_type;

  public:
    impl_()
      : mtx_(),
        value_(std::in_place_index<1u>, nullptr),
        refcount_(0u)
    {
      mtx_.try_lock();
    }

    explicit impl_(executor_type &&executor)
      : mtx_(std::move(executor)),
        value_(std::in_place_index<1u>, nullptr),
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

    void set_executor(executor_type const &executor)
    {
      CORIO_ASSERT(refcount_ > 0u);
      mtx_.set_executor(executor);
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
      std::exception_ptr const *p = std::get_if<1u>(&std::as_const(value_));
      return p == nullptr || *p != nullptr;
    }

    void set_value(R const &value)
    {
      CORIO_ASSERT(!ready());
      CORIO_ASSERT(!mtx_.try_lock());
      value_.template emplace<0u>(value);
      mtx_.unlock();
    }

    void set_value(R &&value)
    {
      CORIO_ASSERT(!ready());
      CORIO_ASSERT(!mtx_.try_lock());
      value_.template emplace<0u>(std::move(value));
      mtx_.unlock();
    }

    void set_exception(std::exception_ptr p)
    {
      CORIO_ASSERT(!ready());
      CORIO_ASSERT(!mtx_.try_lock());
      value_.template emplace<1u>(std::move(p));
      mtx_.unlock();
    }

    template<typename CompletionHandler>
    void async_get(CompletionHandler &&h)
    {
      CORIO_ASSERT(refcount_ > 0u);
      mtx_.async_lock(
        [h_ = std::move(h), &value = value_](lock_type_ lock) -> void{
          lock = lock_type_();
          std::move(h_)(std::move(value));
        });
    }

    template<typename CompletionHandler>
    void async_wait(CompletionHandler &&h)
    {
      CORIO_ASSERT(refcount_ > 0u);
      mtx_.async_lock(
        [h_ = std::move(h)](lock_type_ lock) -> void{
          lock = lock_type_();
          std::move(h_)();
        });
    }

  private:
    mutex_type_ mtx_;
    async_get_argument_type value_;
    std::size_t refcount_;
  }; // class impl_

public:
  shared_future_state_()
    : p_(new impl_())
  {
    p_->acquire();
  }

  shared_future_state_(std::nullptr_t) noexcept
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

  void set_executor(executor_type const &executor)
  {
    CORIO_ASSERT(p_ != nullptr);
    p_->set_executor(executor);
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

  void set_value(R const &value)
  {
    CORIO_ASSERT(p_ != nullptr);
    p_->set_value(value);
  }

  void set_value(R &&value)
  {
    CORIO_ASSERT(p_ != nullptr);
    p_->set_value(std::move(value));
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
      [self = *this, h_ = std::move(h)](async_get_argument_type r) -> void{
        std::move(h_)(std::move(r));
      });
  }

  template<typename CompletionHandler>
  void async_wait(CompletionHandler &&h)
  {
    CORIO_ASSERT(p_ != nullptr);
    p_->async_wait(
      [self = *this, h_ = std::move(h)](async_get_argument_type) -> void{
        std::move(h_)();
      });
  }

private:
  impl_ *p_;
}; // class shared_future_state_

} // namespace corio::thread_unsafe::detail_

#endif // !defined(CORIO_THREAD_UNSAFE_DETAIL_SHARED_FUTURE_STATE_HPP_INCLUDE_GUARD)
