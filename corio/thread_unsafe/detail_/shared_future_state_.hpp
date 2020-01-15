#if !defined(CORIO_THREAD_UNSAFE_DETAIL_SHARED_FUTURE_STATE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_DETAIL_SHARED_FUTURE_STATE_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/mutex.hpp>
#include <corio/util/exception_or.hpp>
#include <corio/util/assert.hpp>
#include <boost/asio/async_result.hpp>
#include <type_traits>
#include <variant>
#include <optional>
#include <tuple>
#include <utility>
#include <exception>
#include <cstddef>


namespace corio::thread_unsafe::detail_{

template<typename R>
class shared_future_state_
{
private:
  using value_type_ = std::tuple<std::optional<R>, std::exception_ptr, corio::thread_unsafe::mutex, std::size_t>;
  static constexpr std::size_t value_ = 0u;
  static constexpr std::size_t exception_ = 1u;
  static constexpr std::size_t mutex_ = 2u;
  static constexpr std::size_t refcount_ = 3u;

public:
  shared_future_state_()
    : p_(new value_type_())
  {
    ++std::get<refcount_>(*p_);
    std::get<mutex_>(*p_).try_lock();
  }

  shared_future_state_(shared_future_state_ const &rhs) noexcept
    : p_(rhs.p_)
  {
    if (p_ != nullptr) {
      ++std::get<refcount_>(*p_);
    }
  }

  shared_future_state_(shared_future_state_ &&rhs) noexcept
    : p_(rhs.p_)
  {
    rhs.p_ = nullptr;
  }

  ~shared_future_state_() noexcept
  {
    CORIO_ASSERT(p_ == nullptr || std::get<refcount_>(*p_) > 0u);
    if (p_ != nullptr && --std::get<refcount_>(*p_) == 0u) {
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

  bool valid() const noexcept
  {
    CORIO_ASSERT(p_ == nullptr || std::get<refcount_>(*p_) > 0u);
    return p_ != nullptr;
  }

  bool ready() const noexcept
  {
    if (p_ == nullptr) {
      return false;
    }
    CORIO_ASSERT(std::get<refcount_>(*p_) > 0u);
    CORIO_ASSERT(!std::get<value_>(*p_).has_value() || std::get<exception_>(*p_) == nullptr);
    return std::get<value_>(*p_).has_value() || std::get<exception_>(*p_) != nullptr;
  }

  void set_value(R const &value)
  {
    CORIO_ASSERT(p_ != nullptr);
    CORIO_ASSERT(std::get<refcount_>(*p_) > 0u);
    CORIO_ASSERT(!std::get<value_>(*p_).has_value());
    CORIO_ASSERT(std::get<exception_>(*p_) == nullptr);
    std::get_<value_>(*p_).emplace(value);
    std::get<mutex_>(*p_).unlock();
  }

  void set_value(R &&value)
  {
    CORIO_ASSERT(p_ != nullptr);
    CORIO_ASSERT(std::get<refcount_>(*p_) > 0u);
    CORIO_ASSERT(!std::get<value_>(*p_).has_value());
    CORIO_ASSERT(std::get<exception_>(*p_) == nullptr);
    std::get_<value_>(*p_).emplace(std::move(value));
    std::get<mutex_>(*p_).unlock();
  }

  void set_exception(std::exception_ptr p)
  {
    CORIO_ASSERT(p_ != nullptr);
    CORIO_ASSERT(std::get<refcount_>(*p_) > 0u);
    CORIO_ASSERT(!std::get<value_>(*p_).has_value());
    CORIO_ASSERT(std::get<exception_>(*p_) == nullptr);
    std::get<exception_>(*p_) = std::move(p);
    std::get<mutex_>(*p_).unlock();
  }

  template<typename CompletionToken>
  auto async_get(CompletionToken &&token)
  {
    CORIO_ASSERT(p_ != nullptr);
    CORIO_ASSERT(std::get<refcount_>(*p_) > 0u);

    using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void(corio::exception_or<R>)>;
    using completion_handler_type = typename async_result_type::completion_handler_type;
    completion_handler_type completion_handler(std::move(token));
    async_result_type async_result(completion_handler);

    std::get<mutex_>(*p_).async_lock(
      [state = *this, completion_handler_ = std::move(completion_handler)]() -> void{
        CORIO_ASSERT(state.p_ != nullptr);
        CORIO_ASSERT(std::get<refcount_>(*state.p_) > 0u);
        CORIO_ASSERT(std::get<value_>(*state.p_).has_value() != (std::get<exception_>(*state.p_) != nullptr));
        CORIO_ASSERT(!std::get<mutex_>(*state.p_).try_lock());
        std::get<mutex_>(*state.p_).unlock();
        std::exception_ptr p_exception = std::get<exception_>(*state.p_);
        if (p_exception != nullptr) {
          auto result = corio::exception_or<R>::make_exception(std::move(p_exception));
          std::move(completion_handler_)(std::move(result));
        }
        else {
          R &value = std::get<value_>(*state.p_).value();
          auto result = corio::exception_or<R>::make_value(std::move(value));
          std::move(completion_handler_)(std::move(result));
        }
      });
    return async_result.get();
  }

  template<typename CompletionToken>
  auto async_wait(CompletionToken &&token)
  {
    CORIO_ASSERT(p_ != nullptr);
    CORIO_ASSERT(std::get<refcount_>(*p_) > 0u);

    using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void()>;
    using completion_handler_type = typename async_result_type::completion_handler_type;
    completion_handler_type completion_handler(std::move(token));
    async_result_type async_result(completion_handler);

    std::get<mutex_>(*p_).async_lock(
      [state = *this, completion_handler_ = std::move(completion_handler)]() -> void{
        CORIO_ASSERT(state.p_ != nullptr);
        CORIO_ASSERT(std::get<refcount_>(*state.p_) > 0u);
        CORIO_ASSERT(std::get<value_>(*state.p_).has_value() != (std::get<exception_>(*state.p_) != nullptr));
        CORIO_ASSERT(!std::get<mutex_>(*state.p_).try_lock());
        std::get<mutex_>(*state.p_).unlock();
        std::move(completion_handler_)();
      });
    return async_result.get();
  }

private:
  value_type_ *p_;
}; // class shared_future_state_

template<typename R>
class shared_future_state_<R &>
{
private:
  using value_type_ = std::tuple<std::variant<R *, std::exception_ptr>, corio::thread_unsafe::mutex, std::size_t>;
  static constexpr std::size_t value_ = 0u;
  static constexpr std::size_t mutex_ = 1u;
  static constexpr std::size_t refcount_ = 2u;

public:
  shared_future_state_()
    : p_(new value_type_())
  {
    ++std::get<refcount_>(*p_);
    std::get<mutex_>(*p_).try_lock();
  }

  shared_future_state_(shared_future_state_ const &rhs) noexcept
    : p_(rhs.p_)
  {
    if (p_ != nullptr) {
      ++std::get<refcount_>(*p_);
    }
  }

  shared_future_state_(shared_future_state_ &&rhs) noexcept
    : p_(rhs.p_)
  {
    rhs.p_ = nullptr;
  }

  ~shared_future_state_() noexcept
  {
    CORIO_ASSERT(p_ == nullptr || std::get<refcount_>(*p_) > 0u);
    if (p_ != nullptr && --std::get<refcount_>(*p_) == 0u) {
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

  bool valid() const noexcept
  {
    CORIO_ASSERT(p_ == nullptr || std::get<refcount_>(*p_) > 0u);
    return p_ != nullptr;
  }

  bool ready() const noexcept
  {
    if (p_ == nullptr) {
      return false;
    }
    CORIO_ASSERT(std::get<refcount_>(*p_) > 0u);
    auto visitor = [](auto &v) -> bool{
      return v != nullptr;
    };
    return std::visit(std::move(visitor), std::get<value_>(*p_));
  }

  void set_value(R &value)
  {
    CORIO_ASSERT(p_ != nullptr);
    CORIO_ASSERT(std::get<refcount_>(*p_) > 0u);
    CORIO_ASSERT(!ready());
    std::get<value_>(*p_).emplace<R *>(&value);
    std::get<mutex_>(*p_).unlock();
  }

  void set_exception(std::exception_ptr p)
  {
    CORIO_ASSERT(p_ != nullptr);
    CORIO_ASSERT(std::get<refcount_>(*p_) > 0u);
    CORIO_ASSERT(!ready());
    std::get<value_>(*p_).emplace<std::exception_ptr>(std::move(p));
    std::get<mutex_>(*p_).unlock();
  }

  template<typename CompletionToken>
  auto async_get(CompletionToken &&token)
  {
    CORIO_ASSERT(p_ != nullptr);
    CORIO_ASSERT(std::get<refcount_>(*p_) > 0u);

    using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void(corio::exception_or<R &>)>;
    using completion_handler_type = typename async_result_type::completion_handler_type;
    completion_handler_type completion_handler(std::move(token));
    async_result_type async_result(completion_handler);

    std::get<mutex_>(*p_).async_lock(
      [state = *this, completion_handler_ = std::move(completion_handler)]() -> void{
        CORIO_ASSERT(state.p_ != nullptr);
        CORIO_ASSERT(std::get<refcount_>(*state.p_) > 0u);
        CORIO_ASSERT(!std::get<mutex_>(*state.p_).try_lock());
        std::get<mutex_>(*state.p_).unlock();
        auto visitor = [h = std::move(completion_handler_)](auto v) -> void{
          using type = std::decay_t<std::remove_reference_t<decltype(v)> >;
          if constexpr (std::is_same_v<type, R *>) {
            auto result = corio::exception_or<R &>::make_value(*v);
            std::move(h)(std::move(result));
          }
          else {
            static_assert(std::is_same_v<type, std::exception_ptr>);
            auto result = corio::exception_or<R &>::make_exception(std::move(v));
            std::move(h)(std::move(result));
          }
        };
        std::visit(std::move(visitor), std::move(std::get<value_>(*state.p_)));
      });
    return async_result.get();
  }

  template<typename CompletionToken>
  auto async_wait(CompletionToken &&token)
  {
    CORIO_ASSERT(p_ != nullptr);
    CORIO_ASSERT(std::get<refcount_>(*p_) > 0u);

    using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void()>;
    using completion_handler_type = typename async_result_type::completion_handler_type;
    completion_handler_type completion_handler(std::move(token));
    async_result_type async_result(completion_handler);

    std::get<mutex_>(*p_).async_lock(
      [state = *this, completion_handler_ = std::move(completion_handler)]() -> void{
        CORIO_ASSERT(state.p_ != nullptr);
        CORIO_ASSERT(std::get<refcount_>(*state.p_) > 0u);
        CORIO_ASSERT(!std::get<mutex_>(*state.p_).try_lock());
        std::get<mutex_>(*state.p_).unlock();
        std::move(completion_handler_)();
      });
    return async_result.get();
  }

private:
  value_type_ *p_;
}; // class shared_future_state_<R &>

template<>
class shared_future_state_<void>
{
private:
  using value_type_ = std::tuple<std::exception_ptr, corio::thread_unsafe::mutex, std::size_t>;
  static constexpr std::size_t exception_ = 0u;
  static constexpr std::size_t mutex_ = 1u;
  static constexpr std::size_t refcount_ = 2u;

public:
  shared_future_state_()
    : p_(new value_type_())
  {
    ++std::get<refcount_>(*p_);
    std::get<mutex_>(*p_).try_lock();
  }

  shared_future_state_(shared_future_state_ const &rhs) noexcept
    : p_(rhs.p_)
  {
    if (p_ != nullptr) {
      ++std::get<refcount_>(*p_);
    }
  }

  shared_future_state_(shared_future_state_ &&rhs) noexcept
    : p_(rhs.p_)
  {
    rhs.p_ = nullptr;
  }

  ~shared_future_state_() noexcept
  {
    CORIO_ASSERT(p_ == nullptr || std::get<refcount_>(*p_) > 0u);
    if (p_ != nullptr && --std::get<refcount_>(*p_) == 0u) {
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

  bool valid() const noexcept
  {
    CORIO_ASSERT(p_ == nullptr || std::get<refcount_>(*p_) > 0u);
    return p_ != nullptr;
  }

  bool ready() const noexcept
  {
    if (p_ == nullptr) {
      return false;
    }
    CORIO_ASSERT(std::get<refcount_>(*p_) > 0u);
    bool result = std::get<mutex_>(*p_).try_lock();
    if (result) {
      std::get<mutex_>(*p_).unlock();
    }
    return result;
  }

  void set_value()
  {
    CORIO_ASSERT(p_ != nullptr);
    CORIO_ASSERT(std::get<refcount_>(*p_) > 0u);
    CORIO_ASSERT(!ready());
    std::get<mutex_>(*p_).unlock();
  }

  void set_exception(std::exception_ptr p)
  {
    CORIO_ASSERT(p_ != nullptr);
    CORIO_ASSERT(std::get<refcount_>(*p_) > 0u);
    CORIO_ASSERT(!ready());
    std::get<exception_>(*p_) = std::move(p);
    std::get<mutex_>(*p_).unlock();
  }

  template<typename CompletionToken>
  auto async_get(CompletionToken &&token)
  {
    CORIO_ASSERT(p_ != nullptr);
    CORIO_ASSERT(std::get<refcount_>(*p_) > 0u);

    using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void(corio::exception_or<void>)>;
    using completion_handler_type = typename async_result_type::completion_handler_type;
    completion_handler_type completion_handler(std::move(token));
    async_result_type async_result(completion_handler);

    std::get<mutex_>(*p_).async_lock(
      [state = *this, completion_handler_ = std::move(completion_handler)]() -> void{
        CORIO_ASSERT(state.p_ != nullptr);
        CORIO_ASSERT(std::get<refcount_>(*state.p_) > 0u);
        CORIO_ASSERT(!std::get<mutex_>(*state.p_).try_lock());
        std::get<mutex_>(*state.p_).unlock();
        if (std::get<exception_>(*state.p_) == nullptr) {
          auto result = corio::exceptoin_or<void>::make_value();
          std::move(completion_handler_)(std::move(result));
        }
        else {
          std::exception_ptr p = std::get<exception_>(*state.p_);
          auto result = corio::exception_or<void>::make_exception(std::move(p));
          std::move(completion_handler_)(std::move(result));
        }
      });
    return async_result.get();
  }

  template<typename CompletionToken>
  auto async_wait(CompletionToken &&token)
  {
    CORIO_ASSERT(p_ != nullptr);
    CORIO_ASSERT(std::get<refcount_>(*p_) > 0u);

    using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void()>;
    using completion_handler_type = typename async_result_type::completion_handler_type;
    completion_handler_type completion_handler(std::move(token));
    async_result_type async_result(completion_handler);

    std::get<mutex_>(*p_).async_lock(
      [state = *this, completion_handler_ = std::move(completion_handler)]() -> void{
        CORIO_ASSERT(state.p_ != nullptr);
        CORIO_ASSERT(std::get<refcount_>(*state.p_) > 0u);
        CORIO_ASSERT(!std::get<mutex_>(*state.p_).try_lock());
        std::get<mutex_>(*state.p_).unlock();
        std::move(completion_handler_)();
      });
    return async_result.get();
  }

private:
  value_type_ *p_;
}; // class shared_future_state_<void>

} // namespace corio::thread_unsafe::detail_

#endif // !defined(CORIO_THREAD_UNSAFE_DETAIL_SHARED_FUTURE_STATE_HPP_INCLUDE_GUARD)
