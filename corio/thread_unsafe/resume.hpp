#if !defined(CORIO_THREAD_UNSAFE_RESUME_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_RESUME_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/promise.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/system/system_error.hpp>
#include <boost/system/error_code.hpp>
#include <type_traits>
#include <tuple>
#include <utility>
#include <system_error>
#include <exception>


namespace corio::thread_unsafe{

namespace detail_{

template<typename Executor, typename NoThrow>
class resume_token_
{
public:
  using executor_type = Executor;

  explicit resume_token_(executor_type const &executor)
    : resume_token_(executor_type(executor))
  {}

  explicit resume_token_(executor_type &&executor)
    : executor_(std::move(executor))
  {}

  resume_token_(resume_token_ const &rhs) = default;

  resume_token_(resume_token_ &&rhs) = default;

  resume_token_ &operator=(resume_token_ const &) = delete;

  executor_type get_executor() const
  {
    return executor_;
  }

private:
  executor_type executor_;
}; // class resume_token_

} // namespace detail_

struct resume_t
{
  template<typename Executor>
  detail_::resume_token_<Executor, std::false_type> operator()(
    Executor &&executor,
    corio::disable_if_execution_context_t<std::decay_t<Executor> > * = nullptr,
    corio::enable_if_executor_t<std::decay_t<Executor> > * = nullptr) const
  {
    using executor_type = Executor;
    using token_type = detail_::resume_token_<executor_type, std::false_type>;
    return token_type(std::forward<Executor>(executor));
  }

  template<typename ExecutionContext>
  detail_::resume_token_<typename ExecutionContext::executor_type, std::false_type> operator()(
    ExecutionContext &context,
    corio::enable_if_execution_context_t<ExecutionContext> * = nullptr,
    corio::disable_if_executor_t<ExecutionContext> * = nullptr)
  {
    using context_type = ExecutionContext;
    using executor_type = typename context_type::executor_type;
    using token_type = detail_::resume_token_<executor_type, std::false_type>;
    return token_type(context.get_executor());
  }
}; // struct resume_t

inline constexpr resume_t resume{};

struct nothrow_resume_t
{
  template<typename Executor>
  detail_::resume_token_<Executor, std::true_type> operator()(
    Executor &&executor,
    corio::disable_if_execution_context_t<std::decay_t<Executor> > * = nullptr,
    corio::enable_if_executor_t<std::decay_t<Executor> > * = nullptr)
  {
    using executor_type = Executor;
    using token_type = detail_::resume_token_<executor_type, std::true_type>;
    return token_type(std::forward<Executor>(executor));
  }

  template<typename ExecutionContext>
  detail_::resume_token_<typename ExecutionContext::executor_type, std::true_type> operator()(
    ExecutionContext &context,
    corio::enable_if_execution_context_t<ExecutionContext> * = nullptr,
    corio::disable_if_executor_t<ExecutionContext> * = nullptr)
  {
    using context_type = ExecutionContext;
    using executor_type = typename context_type::executor_type;
    using token_type = detail_::resume_token_<executor_type, std::true_type>;
    return token_type(context.get_executor());
  }
}; // struct nothrow_resume_t

inline constexpr nothrow_resume_t nothrow_resume{};

namespace detail_{

template<typename Executor, typename NoThrow, typename... Args>
class resume_handler_;

template<typename Executor, typename NoThrow>
class resume_handler_<Executor, NoThrow>
{
public:
  using executor_type = Executor;
  static_assert(corio::is_executor_v<executor_type>);
  using no_throw_type = NoThrow;

private:
  using token_type_ = resume_token_<executor_type, no_throw_type>;
  using promise_type_ = corio::thread_unsafe::basic_promise<void, executor_type>;

public:
  using future_type = typename promise_type_::future_type;

  explicit resume_handler_(token_type_ const &token)
    : promise_(token.get_executor())
  {}

  resume_handler_(resume_handler_ const &) = delete;

  resume_handler_(resume_handler_ &&rhs) = default;

  resume_handler_ &operator=(resume_handler_ const &) = delete;

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
}; // class resume_handler_<Executor, NoThrow>

template<typename Executor, typename NoThrow, typename HeadArg, typename... TailArgs>
class resume_handler_<Executor, NoThrow, HeadArg, TailArgs...>
{
public:
  using executor_type = Executor;
  static_assert(corio::is_executor_v<executor_type>);
  using no_throw_type = NoThrow;

private:
  using token_type_ = resume_token_<executor_type, no_throw_type>;
  using may_throw_ = std::bool_constant<
    !no_throw_type::value
    && (std::is_same_v<std::decay_t<HeadArg>, std::error_code>
        || std::is_same_v<std::decay_t<HeadArg>, boost::system::error_code>)>;
  using value_type_ = std::conditional_t<
    may_throw_::value,
    std::conditional_t<
      sizeof...(TailArgs) == 0u,
      void,
      std::conditional_t<
        sizeof...(TailArgs) == 1u,
        std::tuple_element_t<0u, std::tuple<std::decay_t<TailArgs>..., void> >,
        std::tuple<std::decay_t<TailArgs>...> > >,
    std::conditional_t<
      sizeof...(TailArgs) == 0u,
      std::decay_t<HeadArg>,
      std::tuple<std::decay_t<HeadArg>, std::decay_t<TailArgs>...> > >;
  using promise_type_ = corio::thread_unsafe::promise<value_type_>;

public:
  using future_type = typename promise_type_::future_type;

  explicit resume_handler_(token_type_ const &token)
    : promise_(token.get_executor())
  {}

  resume_handler_(resume_handler_ const &) = delete;

  resume_handler_(resume_handler_ &&rhs) = default;

  resume_handler_ &operator=(resume_handler_ const &) = delete;

  executor_type get_executor() const
  {
    return promise_.get_executor();
  }

  future_type get_future()
  {
    return promise_.get_future();
  }

  void operator()(HeadArg head, TailArgs... args)
  {
    if constexpr (!no_throw_type::value) {
      if constexpr (std::is_same_v<std::decay_t<HeadArg>, std::error_code>) {
        if (!!head) {
          std::exception_ptr p = std::make_exception_ptr(std::system_error(std::forward<HeadArg>(head)));
          promise_.set_exception(std::move(p));
          return;
        }
      }
      else if (std::is_same_v<std::decay_t<HeadArg>, boost::system::error_code>) {
        if (!!head) {
          std::exception_ptr p = std::make_exception_ptr(boost::system::system_error(std::forward<HeadArg>(head)));
          promise_.set_exception(std::move(p));
          return;
        }
      }
      if constexpr (sizeof...(TailArgs) <= 1u) {
        promise_.set_value(std::forward<TailArgs>(args)...);
      }
      else {
        promise_.set_value(value_type_(std::forward<TailArgs>(args)...));
      }
    }
    else {
      if constexpr (sizeof...(TailArgs) == 0u) {
        promise_.set_value(std::forward<HeadArg>(head));
      }
      else {
        promise_.set_value(value_type_(std::forward<HeadArg>(head), std::forward<TailArgs>(args)...));
      }
    }
  }

private:
  promise_type_ promise_;
}; // class resume_handler_<Executor, NoThrow, HeadArg, TailArgs...>

} // namespace detail_

} // namespace corio::thread_unsafe

namespace boost::asio{

template<typename Executor, typename NoThrow, typename Signature>
class async_result<corio::thread_unsafe::detail_::resume_token_<Executor, NoThrow>, Signature>;

template<typename Executor, typename NoThrow, typename... Args>
class async_result<corio::thread_unsafe::detail_::resume_token_<Executor, NoThrow>, void(Args...)>
{
public:
  using completion_handler_type = corio::thread_unsafe::detail_::resume_handler_<Executor, NoThrow, Args...>;

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
}; // class async_result<corio::thread_unsafe::detail_::resume_token_, void(Args...)>

} // namespace boost::asio

#endif // !defined(CORIO_THREAD_UNSAFE_RESUME_HPP_INCLUDE_GUARD)
