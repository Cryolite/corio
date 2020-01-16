#if !defined(CORIO_THREAD_UNSAFE_CONTEXTLESS_DETAIL_SHARED_FUTURE_STATE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_CONTEXTLESS_DETAIL_SHARED_FUTURE_STATE_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/contextless/mutex.hpp>
#include <corio/util/assert.hpp>
#include <boost/asio/async_result.hpp>
#include <type_traits>
#include <variant>
#include <utility>
#include <exception>
#include <cstddef>


namespace corio::thread_unsafe::contextless::detail_{

template<typename R>
class shared_future_state_
{
public:
  using result_type = std::variant<R, std::exception_ptr>;

private:
  class impl_
  {
  private:
    using mutex_type_ = corio::thread_unsafe::contextless::mutex;

  public:
    impl_()
      : value_(std::in_place_index<1u>, nullptr),
        mutex_(),
        refcount_(1u)
    {}

    impl_(impl_ const &) = delete;

    impl_ &operator=(impl_ const &) = delete;

    void notify_reference_copy() noexcept
    {
      ++refcount_;
    }

    std::size_t release_reference() noexcept
    {
      CORIO_ASSERT(refcount_ > 0u);
      return --refcount_;
    }

    template<typename ExecutionContext>
    void set_context(ExecutionContext &ctx)
    {
      mutex_.set_context(ctx);
      mutex_.try_lock();
    }

    bool has_context() const noexcept
    {
      return mutex_.has_context();
    }

    bool ready() const noexcept
    {
      CORIO_ASSERT(refcount_ > 0u);
      auto visitor = [](auto &v) noexcept -> bool{
        using type = std::decay_t<std::remove_reference_t<decltype(v)> >;
        if constexpr (std::is_same_v<type, std::exception_ptr>) {
          return v == nullptr;
        }
        else {
          static_assert(std::is_same_v<type, R>);
          return false;
        }
      };
      return !(value_.index() == 1u && std::visit(std::move(visitor), value_));
    }

    void set_value(R const &value)
    {
      CORIO_ASSERT(!ready());
      CORIO_ASSERT(!mutex_.try_lock());
      value_.template emplace<0u>(value);
      mutex_.unlock();
    }

    void set_value(R &&value)
    {
      CORIO_ASSERT(!ready());
      CORIO_ASSERT(!mutex_.try_lock());
      value_.template emplace<0u>(std::move(value));
      mutex_.unlock();
    }

    void set_exception(std::exception_ptr p)
    {
      CORIO_ASSERT(!ready());
      CORIO_ASSERT(!mutex_.try_lock());
      value_.template emplace<1u>(std::move(p));
      mutex_.unlock();
    }

    template<typename CompletionToken>
    auto async_get(CompletionToken &&token)
    {
      CORIO_ASSERT(refcount_ > 0u);
      using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void(result_type)>;
      using completion_handler_type = typename async_result_type::completion_handler_type;
      completion_handler_type completion_handler(std::move(token));
      async_result_type async_result(completion_handler);
      mutex_.async_lock(
        [h = std::move(completion_handler), &value = value_]() -> void{
          std::move(h)(std::move(value));
        });
      return async_result.get();
    }

    template<typename CompletionToken>
    auto async_wait(CompletionToken &&token)
    {
      CORIO_ASSERT(refcount_ > 0u);
      using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void()>;
      using completion_handler_type = typename async_result_type::completion_handler_type;
      completion_handler_type completion_handler(std::move(token));
      async_result_type async_result(completion_handler);
      mutex_.async_lock(std::move(completion_handler));
      return async_result.get();
    }

  private:
    result_type value_;
    mutex_type_ mutex_;
    std::size_t refcount_;
  }; // class impl_

public:
  shared_future_state_()
    : p_(new impl_())
  {}

  explicit shared_future_state_(nullptr_t) noexcept
    : p_()
  {}

  shared_future_state_(shared_future_state_ const &rhs) noexcept
    : p_(rhs.p_)
  {
    if (p_ != nullptr) {
      p_->notify_reference_copy();
    }
  }

  shared_future_state_(shared_future_state_ &&rhs) noexcept
    : p_(rhs.p_)
  {
    rhs.p_ = nullptr;
  }

  ~shared_future_state_() noexcept
  {
    if (p_ != nullptr && p_->release_reference() == 0u) {
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

  template<typename ExecutionContext>
  void set_context(ExecutionContext &ctx)
  {
    CORIO_ASSERT(p_ != nullptr);
    p_->set_context(ctx);
  }

  bool has_context() const noexcept
  {
    return p_ != nullptr && p_->has_context();
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

  template<typename CompletionToken>
  auto async_get(CompletionToken &&token)
  {
    CORIO_ASSERT(p_ != nullptr);
    using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void(result_type)>;
    using completion_handler_type = typename async_result_type::completion_handler_type;
    completion_handler_type completion_handler(std::move(token));
    async_result_type async_result(completion_handler);
    p_->async_get(
      [self = *this, h = std::move(completion_handler)](result_type r) -> void{
        std::move(h)(std::move(r));
      });
    return async_result.get();
  }

  template<typename CompletionToken>
  auto async_wait(CompletionToken &&token)
  {
    CORIO_ASSERT(p_ != nullptr);
    using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void()>;
    using completion_handler_type = typename async_result_type::completion_handler_type;
    completion_handler_type completion_handler(std::move(token));
    async_result_type async_result(completion_handler);
    p_->async_wait(
      [self = *this, h = std::move(completion_handler)]() -> void{
        std::move(h)();
      });
    return async_result.get();
  }

private:
  impl_ *p_;
}; // class shared_future_state_

template<typename R>
class shared_future_state_<R &>
  : private shared_future_state_<R *>
{
private:
  using base_type_ = shared_future_state_<R *>;

public:
  using result_type = std::variant<R &, std::exception_ptr>;

  shared_future_state_()
    : base_type_()
  {}

  explicit shared_future_state_(nullptr_t) noexcept
    : base_type_(nullptr)
  {}

  shared_future_state_(shared_future_state_ const &rhs) = default;

  shared_future_state_(shared_future_state_ &&rhs) = default;

  void swap(shared_future_state_ &rhs) noexcept
  {
    using std::swap;
    swap(static_cast<base_type_ &>(rhs));
  }

  friend void swap(shared_future_state_ &lhs, shared_future_state_ &rhs) noexcept
  {
    lhs.swap(rhs);
  }

  shared_future_state_ &operator=(shared_future_state_ const &rhs)
  {
    shared_future_state_(rhs).swap(*this);
    return *this;
  }

  shared_future_state_ &operator=(shared_future_state_ &&rhs)
  {
    shared_future_state_(std::move(rhs)).swap(*this);
    return *this;
  }

  using base_type_::set_context;

  using base_type_::has_context;

  using base_type_::valid;

  using base_type_::ready;

  void set_value(R &value)
  {
    base_type_::set_value(&value);
  }

  void set_exception(std::exception_ptr p)
  {
    base_type_::set_exception(std::move(p));
  }

  template<typename CompletionToken>
  auto async_get(CompletionToken &&token)
  {
    using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void(result_type)>;
    using completion_handler_type = typename async_result_type::completion_handler_type;
    completion_handler_type completion_handler(std::move(token));
    async_result_type async_result(completion_handler);
    base_type_::async_get(
      [h = std::move(completion_handler)](R *p) -> void{
        CORIO_ASSERT(p != nullptr);
        std::move(h)(*p);
      });
    return async_result.get();
  }

  template<typename CompletionToken>
  auto async_wait(CompletionToken &&token)
  {
    return base_type_::async_wait(std::move(token));
  }
}; // class shared_future_state_<R &>

template<>
class shared_future_state_<void>
{
public:
  using result_type = std::variant<std::monostate, std::exception_ptr>;

private:
  class impl_
  {
  private:
    using mutex_type_ = corio::thread_unsafe::contextless::mutex;

  public:
    impl_()
      : value_(std::in_place_index<1u>, nullptr),
        mutex_(),
        refcount_(1u)
    {}

    impl_(impl_ const &) = delete;

    impl_ &operator=(impl_ &) = delete;

    template<typename ExecutionContext>
    void set_context(ExecutionContext &ctx)
    {
      mutex_.set_context(ctx);
      mutex_.try_lock();
    }

    bool has_context() const noexcept
    {
      return mutex_.has_context();
    }

    void notify_reference_copy() noexcept
    {
      ++refcount_;
    }

    std::size_t release_reference() noexcept
    {
      CORIO_ASSERT(refcount_ > 0u);
      return --refcount_;
    }

    bool ready() const noexcept
    {
      CORIO_ASSERT(refcount_ > 0u);
      auto visitor = [](auto &v) noexcept -> bool{
        using type = std::decay_t<std::remove_reference_t<decltype(v)> >;
        if constexpr (std::is_same_v<type, std::exception_ptr>) {
          return v == nullptr;
        }
        else {
          static_assert(std::is_same_v<type, std::monostate>);
          return false;
        }
      };
      return !(value_.index() == 1u && std::visit(std::move(visitor), value_));
    }

    void set_value()
    {
      CORIO_ASSERT(!ready());
      CORIO_ASSERT(!mutex_.try_lock());
      value_.emplace<0u>();
      mutex_.unlock();
    }

    void set_exception(std::exception_ptr p)
    {
      CORIO_ASSERT(!ready());
      CORIO_ASSERT(!mutex_.try_lock());
      value_.emplace<1u>(std::move(p));
      mutex_.unlock();
    }

    template<typename CompletionToken>
    auto async_get(CompletionToken &&token)
    {
      CORIO_ASSERT(ready());
      using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void(result_type)>;
      using completion_handler_type = typename async_result_type::completion_handler_type;
      completion_handler_type completion_handler(std::move(token));
      async_result_type async_result(completion_handler);
      mutex_.async_lock(
        [h = std::move(completion_handler), &value = value_]() -> void{
          std::move(h)(std::move(value));
        });
      return async_result.get();
    }

    template<typename CompletionToken>
    auto async_wait(CompletionToken &&token)
    {
      CORIO_ASSERT(ready());
      using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void(result_type)>;
      using completion_handler_type = typename async_result_type::completion_handler_type;
      completion_handler_type completion_handler(std::move(token));
      async_result_type async_result(completion_handler);
      mutex_.async_lock(std::move(completion_handler));
      return async_result.get();
    }

  private:
    result_type value_;
    mutex_type_ mutex_;
    std::size_t refcount_;
  }; // class impl_

public:
  shared_future_state_()
    : p_(new impl_())
  {}

  explicit shared_future_state_(nullptr_t) noexcept
    : p_()
  {}

  shared_future_state_(shared_future_state_ const &rhs) noexcept
    : p_(rhs.p_)
  {
    if (p_ != nullptr) {
      p_->notify_reference_copy();
    }
  }

  shared_future_state_(shared_future_state_ &&rhs) noexcept
    : p_(rhs.p_)
  {
    rhs.p_ = nullptr;
  }

  ~shared_future_state_() noexcept
  {
    if (p_ != nullptr && p_->release_reference() == 0u) {
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

  shared_future_state_ &operator=(shared_future_state_ const &rhs)
  {
    shared_future_state_(rhs).swap(*this);
    return *this;
  }

  shared_future_state_ &operator=(shared_future_state_ &&rhs)
  {
    shared_future_state_(std::move(rhs)).swap(*this);
    return *this;
  }

  template<typename ExecutionContext>
  void set_context(ExecutionContext &ctx)
  {
    CORIO_ASSERT(p_ != nullptr);
    p_->set_context(ctx);
  }

  bool has_context() const noexcept
  {
    return p_ != nullptr && p_->has_context();
  }

  bool valid() const noexcept
  {
    return p_ != nullptr;
  }

  bool ready() const noexcept
  {
    return p_ != nullptr && p_->ready();
  }

  void set_value()
  {
    CORIO_ASSERT(p_ != nullptr);
    p_->set_value();
  }

  void set_exception(std::exception_ptr p)
  {
    CORIO_ASSERT(p_ != nullptr);
    p_->set_exception(std::move(p));
  }

  template<typename CompletionToken>
  auto async_get(CompletionToken &&token)
  {
    CORIO_ASSERT(p_ != nullptr);
    using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void(std::exception_ptr)>;
    using completion_handler_type = typename async_result_type::completion_handler_type;
    completion_handler_type completion_handler(std::move(token));
    async_result_type async_result(completion_handler);
    p_->async_get(
      [self = *this, h = std::move(completion_handler)]() -> void{
        std::move(h)();
      });
    return async_result.get();
  }

  template<typename CompletionToken>
  auto async_wait(CompletionToken &&token)
  {
    CORIO_ASSERT(p_ != nullptr);
    using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void()>;
    using completion_handler_type = typename async_result_type::completion_handler_type;
    completion_handler_type completion_handler(std::move(token));
    async_result_type async_result(completion_handler);
    p_->async_wait(
      [self = *this, h = std::move(completion_handler)]() -> void{
        std::move(h)();
      });
    return async_result.get();
  }

private:
  impl_ *p_;
}; // class shared_future_state_<void>

} // namespace corio::thread_unsafe::contextless::detail_

#endif // !defined(CORIO_THREAD_UNSAFE_CONTEXTLESS_DETAIL_SHARED_FUTURE_STATE_HPP_INCLUDE_GUARD)
