#if !defined(CORIO_THREAD_UNSAFE_FUTURE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_FUTURE_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/detail_/shared_future_state_.hpp>
#include <corio/core/is_executor.hpp>
#include <corio/core/error.hpp>
#include <corio/util/expected.hpp>
#include <corio/util/throw.hpp>
#include <corio/util/assert.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/executor.hpp>
#include <boost/config.hpp>
#include <type_traits>
#include <utility>
#include <experimental/coroutine>


namespace corio::thread_unsafe{

namespace detail_{

template<typename R, typename Executor>
class promise_mixin_;

template<typename R, typename Executor>
class future_mixin_
{
protected:
  static_assert(!std::is_const_v<R>);
  static_assert(!std::is_volatile_v<R>);
  static_assert(!std::is_rvalue_reference_v<R>);
  using executor_type = Executor;
  static_assert(corio::is_executor_v<executor_type>);
  using state_type_ = corio::thread_unsafe::detail_::shared_future_state_<R, executor_type>;

  explicit future_mixin_(state_type_ const &state) noexcept
    : state_(state)
  {}

  future_mixin_() noexcept
    : state_(nullptr)
  {}

  future_mixin_(future_mixin_ const &) = delete;

  future_mixin_(future_mixin_ &&rhs) noexcept
    : state_(std::move(rhs.state_))
  {}

  void swap(future_mixin_ &rhs) noexcept
  {
    using std::swap;
    swap(state_, rhs.state_);
  }

  future_mixin_ &operator=(future_mixin_ const &) = delete;

  future_mixin_ &operator=(future_mixin_ &&rhs) noexcept
  {
    future_mixin_(std::move(rhs)).swap(*this);
    return *this;
  }

  bool has_executor() const
  {
    if (BOOST_UNLIKELY(!valid())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::no_future_state_error>();
    }
    return state_.has_executor();
  }

  void set_executor(executor_type const &executor)
  {
    set_executor(executor_type(executor));
  }

  void set_executor(executor_type &&executor)
  {
    if (BOOST_UNLIKELY(!valid())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::no_future_state_error>();
    }
    state_.set_executor(std::move(executor));
  }

  executor_type get_executor() const
  {
    if (BOOST_UNLIKELY(!valid())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::no_future_state_error>();
    }
    return state_.get_executor();
  }

  bool valid() const noexcept
  {
    return state_.valid();
  }

  template<typename CompletionToken>
  auto async_wait(CompletionToken &&token)
  {
    if (BOOST_UNLIKELY(!valid())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::no_future_state_error>();
    }
    using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void()>;
    using completion_handler_type = typename async_result_type::completion_handler_type;
    completion_handler_type h(std::forward<CompletionToken>(token));
    async_result_type result(h);
    state_.async_wait(std::move(h));
    return result.get();
  }

  bool await_ready() const noexcept
  {
    return false;
  }

  bool await_suspend(std::experimental::coroutine_handle<> h)
  {
    async_wait([h_ = std::move(h)]() mutable -> void { h_.resume(); });
    return true;
  }

protected:
  state_type_ state_;
}; // class future_mixin_

} // namespace detail_

template<typename R, typename Executor>
class basic_future
  : private detail_::future_mixin_<R, Executor>
{
public:
  using executor_type = Executor;
  static_assert(corio::is_executor_v<executor_type>);

private:
  using mixin_type_ = detail_::future_mixin_<R, executor_type>;
  using typename mixin_type_::state_type_;
  using mixin_type_::state_;

  friend class detail_::promise_mixin_<R, executor_type>;

  explicit basic_future(state_type_ const &state)
    : mixin_type_(state)
  {}

public:
  basic_future() = default;

  void swap(basic_future &rhs) noexcept
  {
    mixin_type_::swap(rhs);
  }

  friend void swap(basic_future &lhs, basic_future &rhs) noexcept
  {
    lhs.swap(rhs);
  }

  using mixin_type_::has_executor;

  using mixin_type_::set_executor;

  using mixin_type_::get_executor;

  using mixin_type_::valid;

  template<typename CompletionToken>
  auto async_get(CompletionToken &&token)
  {
    if (BOOST_UNLIKELY(!valid())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::no_future_state_error>();
    }
    using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void(corio::expected<R>)>;
    using completion_handler_type = typename async_result_type::completion_handler_type;
    completion_handler_type h(std::forward<CompletionToken>(token));
    async_result_type result(h);
    state_.async_get(std::move(h));
    return result.get();
  }

  using mixin_type_::async_wait;

  using mixin_type_::await_ready;

  using mixin_type_::await_suspend;

  R await_resume()
  {
    CORIO_ASSERT(state_.ready());
    return state_.get();
  }
}; // basic_future

template<typename R, typename Executor>
class basic_future<R &, Executor>
  : private detail_::future_mixin_<R &, Executor>
{
public:
  using executor_type = Executor;
  static_assert(corio::is_executor_v<executor_type>);

private:
  using mixin_type_ = detail_::future_mixin_<R &, executor_type>;
  using typename mixin_type_::state_type_;
  using mixin_type_::state_;

  friend class detail_::promise_mixin_<R &, executor_type>;

  explicit basic_future(state_type_ const &state)
    : mixin_type_(state)
  {}

public:
  basic_future() = default;

  void swap(basic_future &rhs) noexcept
  {
    mixin_type_::swap(rhs);
  }

  friend void swap(basic_future &lhs, basic_future &rhs) noexcept
  {
    lhs.swap(rhs);
  }

  using mixin_type_::has_executor;

  using mixin_type_::set_executor;

  using mixin_type_::get_executor;

  using mixin_type_::valid;

  template<typename CompletionToken>
  auto async_get(CompletionToken &&token)
  {
    if (BOOST_UNLIKELY(!valid())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::no_future_state_error>();
    }
    using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void(corio::expected<R &>)>;
    using completion_handler_type = typename async_result_type::completion_handler_type;
    completion_handler_type h(std::forward<CompletionToken>(token));
    async_result_type result(h);
    state_.async_get(std::move(h));
    return result.get();
  }

  using mixin_type_::async_wait;

  using mixin_type_::await_ready;

  using mixin_type_::await_suspend;

  R &await_resume()
  {
    CORIO_ASSERT(state_.ready());
    return state_.get();
  }
}; // basic_future<R &, Executor>

template<typename Executor>
class basic_future<void, Executor>
  : private detail_::future_mixin_<void, Executor>
{
public:
  using executor_type = Executor;
  static_assert(corio::is_executor_v<executor_type>);

private:
  using mixin_type_ = detail_::future_mixin_<void, executor_type>;
  using typename mixin_type_::state_type_;
  using mixin_type_::state_;

  friend class detail_::promise_mixin_<void, executor_type>;

  explicit basic_future(state_type_ const &state)
    : mixin_type_(state)
  {}

public:
  basic_future() = default;

  void swap(basic_future &rhs) noexcept
  {
    mixin_type_::swap(rhs);
  }

  friend void swap(basic_future &lhs, basic_future &rhs) noexcept
  {
    lhs.swap(rhs);
  }

  using mixin_type_::has_executor;

  using mixin_type_::set_executor;

  using mixin_type_::get_executor;

  using mixin_type_::valid;

  template<typename CompletionToken>
  auto async_get(CompletionToken &&token)
  {
    if (BOOST_UNLIKELY(!valid())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::no_future_state_error>();
    }
    using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void(corio::expected<void>)>;
    using completion_handler_type = typename async_result_type::completion_handler_type;
    completion_handler_type h(std::forward<CompletionToken>(token));
    async_result_type result(h);
    state_.async_get(std::move(h));
    return result.get();
  }

  using mixin_type_::async_wait;

  using mixin_type_::await_ready;

  using mixin_type_::await_suspend;

  void await_resume()
  {
    CORIO_ASSERT(state_.ready());
    state_.get();
  }
}; // basic_future<void, Executor>

template<typename R>
using future = basic_future<R, boost::asio::executor>;

} // namespace corio::thread_unsafe

#endif // !defined(CORIO_THREAD_UNSAFE_FUTURE_HPP_INCLUDE_GUARD)
