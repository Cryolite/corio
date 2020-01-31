#if !defined(CORIO_THREAD_UNSAFE_DETAIL_USE_FUTURE_TOKEN_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_DETAIL_USE_FUTURE_TOKEN_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/promise.hpp>
#include <corio/core/is_executor.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/system/system_error.hpp>
#include <boost/system/error_code.hpp>
#include <type_traits>
#include <tuple>
#include <utility>
#include <system_error>
#include <exception>


namespace corio::thread_unsafe::detail_{

template<typename Executor, typename NoThrow>
class use_future_token_;

template<typename Executor, bool NoThrow>
class use_future_token_<Executor, std::bool_constant<NoThrow> >
{
public:
  using executor_type = Executor;
  static_assert(corio::is_executor_v<executor_type>);
  using nothrow_type = std::bool_constant<NoThrow>;

  explicit use_future_token_(executor_type &&executor)
    : executor_(std::move(executor))
  {}

  use_future_token_(use_future_token_ const &rhs) = default;

  use_future_token_(use_future_token_ &&rhs) = default;

  use_future_token_ &operator=(use_future_token_ const &) = delete;

  executor_type get_executor() const
  {
    return executor_;
  }

  constexpr bool await_ready() noexcept
  {
    return true;
  }

  void await_suspend(std::experimental::coroutine_handle<>) const noexcept
  {}

  use_future_token_ &await_resume() &
  {
    return *this;
  }

private:
  executor_type executor_;
}; // class use_future_token_

template<typename Executor, typename NoThrow, typename... Args>
class use_future_handler_;

template<typename Executor, bool NoThrow>
class use_future_handler_<Executor, std::bool_constant<NoThrow> >
{
public:
  using executor_type = Executor;
  static_assert(corio::is_executor_v<executor_type>);
  using nothrow_type = std::bool_constant<NoThrow>;
  using token_type = use_future_token_<executor_type, nothrow_type>;

private:
  using promise_type_ = corio::thread_unsafe::basic_promise<void, executor_type>;

public:
  using future_type = typename promise_type_::future_type;

  explicit use_future_handler_(token_type const &token)
    : promise_(token.get_executor())
  {}

  use_future_handler_(use_future_handler_ const &) = delete;

  use_future_handler_(use_future_handler_ &&rhs) noexcept
    : promise_(std::move(rhs.promise_))
  {}

  use_future_handler_ &operator=(use_future_handler_ const &) = delete;

  executor_type get_executor() const
  {
    return promise_.get_executor();
  }

  future_type get_future()
  {
    return promise_.get_future();
  }

  void operator()()
  {
    promise_.set_value();
  }

private:
  promise_type_ promise_;
}; // class use_future_handler_<Executor, NoThrow, void()>

template<typename Executor, bool NoThrow, typename Head, typename... Tails>
class use_future_handler_<Executor, std::bool_constant<NoThrow>, Head, Tails...>
{
public:
  using executor_type = Executor;
  static_assert(corio::is_executor_v<executor_type>);
  using nothrow_type = std::bool_constant<NoThrow>;
  using token_type = use_future_token_<executor_type, nothrow_type>;

private:
  using may_throw_ = std::bool_constant<
    !nothrow_type::value
    && (std::is_same_v<std::decay_t<Head>, std::error_code>
        || std::is_same_v<std::decay_t<Head>, boost::system::error_code>)>;
  using result_type_ = std::conditional_t<
    may_throw_::value,
    std::conditional_t<
      sizeof...(Tails) == 0u,
      void,
      std::conditional_t<
        sizeof...(Tails) == 1u,
        std::tuple_element_t<0u, std::tuple<std::decay_t<Tails>..., int> >,
        std::tuple<std::decay_t<Tails>...> > >,
    std::conditional_t<
      sizeof...(Tails) == 0u,
      std::decay_t<Head>,
      std::tuple<std::decay_t<Head>, std::decay_t<Tails>...> > >;
  using promise_type_ = corio::thread_unsafe::basic_promise<result_type_, executor_type>;

public:
  using future_type = typename promise_type_::future_type;

  explicit use_future_handler_(token_type const &token)
    : promise_(token.get_executor())
  {}

  use_future_handler_(use_future_handler_ const &) = delete;

  use_future_handler_(use_future_handler_ &&rhs) noexcept
    : promise_(std::move(rhs.promise_))
  {}

  use_future_handler_ &operator=(use_future_handler_ const &) = delete;

  executor_type get_executor() const
  {
    return promise_.get_executor();
  }

  future_type get_future()
  {
    return promise_.get_future();
  }

  void operator()(Head head, Tails... tails)
  {
    if constexpr (nothrow_type::value) {
      if constexpr (sizeof...(Tails) == 0u) {
        promise_.set_value(std::forward<Head>(head));
      }
      else {
        result_type_ result(std::forward<Head>(head), std::forward<Tails>(tails)...);
        promise_.set_value(std::move(result));
      }
    }
    else {
      if constexpr (std::is_same_v<std::decay_t<Head>, std::error_code>) {
        if (!!head) {
          std::system_error error(std::forward<Head>(head));
          std::exception_ptr p = std::make_exception_ptr(std::move(error));
          promise_.set_exception(std::move(p));
          return;
        }
      }
      else if constexpr (std::is_same_v<std::decay_t<Head>, boost::system::error_code>) {
        if (!!head) {
          boost::system::system_error error(std::forward<Head>(head));
          std::exception_ptr p = std::make_exception_ptr(std::move(error));
          promise_.set_exception(std::move(p));
          return;
        }
      }
      if constexpr (sizeof...(Tails) <= 1u) {
        promise_.set_value(std::forward<Tails>(tails)...);
      }
      else {
        result_type_ result(std::forward<Tails>(tails)...);
        promise_.set_value(std::move(result));
      }
    }
  }

private:
  promise_type_ promise_;
}; // class use_future_handler_<Executor, std::bool_constant<NoThrow>, void(Head, Tails...)>

} // namespace corio::thread_unsafe::detail_

namespace boost::asio{

template<typename Executor, typename NoThrow, typename Signature>
class async_result<corio::thread_unsafe::detail_::use_future_token_<Executor, NoThrow>, Signature>;

template<typename Executor, bool NoThrow, typename... Args>
class async_result<
  corio::thread_unsafe::detail_::use_future_token_<Executor, std::bool_constant<NoThrow> >,
  void(Args...)>
{
public:
  using executor_type = Executor;
  static_assert(corio::is_executor_v<executor_type>);
  using nothrow_type = std::bool_constant<NoThrow>;
  using completion_handler_type = corio::thread_unsafe::detail_::use_future_handler_<
    executor_type, nothrow_type, Args...>;
  using return_type = typename completion_handler_type::future_type;

  explicit async_result(completion_handler_type &h)
    : future_(h.get_future())
  {}

  async_result(async_result const &) = delete;

  async_result &operator=(async_result const &) = delete;

  template<typename Initiation, typename CompletionToken, typename... Brgs>
  static return_type initiate(Initiation &&initiation, CompletionToken &&token, Brgs &&... brgs)
  {
    completion_handler_type completion_handler(std::forward<CompletionToken>(token));
    async_result r(completion_handler);
    std::forward<Initiation>(initiation)(std::move(completion_handler), std::forward<Brgs>(brgs)...);
    return r.get();
  }

  return_type get()
  {
    return std::move(future_);
  }

private:
  return_type future_;
};

} // namespace boost::asio

#endif // !defined(CORIO_THREAD_UNSAFE_DETAIL_USE_FUTURE_TOKEN_HPP_INCLUDE_GUARD)
