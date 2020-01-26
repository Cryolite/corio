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

struct nothrow_resume_t
{};

struct resume_t
{
  constexpr nothrow_resume_t operator()(std::nothrow_t) const
  {
    return {};
  }
};

inline constexpr resume_t resume{};

namespace detail_{

template<typename NoThrow, typename... ArgTypes>
class resume_handler_;

template<typename NoThrow>
class resume_handler_<NoThrow>
{
private:
  using promise_type_ = corio::thread_unsafe::promise<void>;

public:
  using future_type = typename promise_type_::future_type;

  explicit resume_handler_(resume_t)
    : promise_()
  {
    static_assert(!NoThrow::value);
  }

  explicit resume_handler_(nothrow_resume_t)
    : promise_()
  {
    static_assert(NoThrow::value);
  }

  resume_handler_(resume_handler_ const &) = delete;

  resume_handler_(resume_handler_ &&rhs) = default;

  resume_handler_ &operator=(resume_handler_ const &) = delete;

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
}; // class resume_handler_<NoThrow>

template<typename NoThrow, typename T, typename... ArgTypes>
class resume_handler_<NoThrow, T, ArgTypes...>
{
private:
  using may_throw_ = std::bool_constant<
    !NoThrow::value
    && (std::is_same_v<std::decay_t<T>, std::error_code>
        || std::is_same_v<std::decay_t<T>, boost::system::error_code>)>;
  using value_type_ = std::conditional_t<
    may_throw_::value,
    std::conditional_t<
      sizeof...(ArgTypes) == 0u,
      void,
      std::conditional_t<
        sizeof...(ArgTypes) == 1u,
        std::tuple_element_t<0u, std::tuple<std::decay_t<ArgTypes>..., void> >,
        std::tuple<std::decay_t<ArgTypes>...> > >,
    std::conditional_t<
      sizeof...(ArgTypes) == 0u,
      std::decay_t<T>,
      std::tuple<std::decay_t<T>, std::decay_t<ArgTypes>...> > >;
  using promise_type_ = corio::thread_unsafe::promise<value_type_>;

public:
  using future_type = typename promise_type_::future_type;

  explicit resume_handler_(resume_t)
    : promise_()
  {
    static_assert(!NoThrow::value);
  }

  explicit resume_handler_(nothrow_resume_t)
    : promise_()
  {
    static_assert(NoThrow::value);
  }

  resume_handler_(resume_handler_ const &) = delete;

  resume_handler_(resume_handler_ &&rhs) = default;

  resume_handler_ &operator=(resume_handler_ const &) = delete;

  future_type get_future()
  {
    return promise_.get_future();
  }

  void operator()(T head, ArgTypes... args)
  {
    if constexpr (!NoThrow::value) {
      if constexpr (std::is_same_v<std::decay_t<T>, std::error_code>) {
        if (!!head) {
          std::exception_ptr p = std::make_exception_ptr(std::system_error(std::forward<T>(head)));
          promise_.set_exception(std::move(p));
          return;
        }
      }
      else if (std::is_same_v<std::decay_t<T>, boost::system::error_code>) {
        if (!!head) {
          std::exception_ptr p = std::make_exception_ptr(boost::system::system_error(std::forward<T>(head)));
          promise_.set_exception(std::move(p));
          return;
        }
      }
      if constexpr (sizeof...(ArgTypes) <= 1u) {
        promise_.set_value(std::forward<ArgTypes>(args)...);
      }
      else {
        promise_.set_value(value_type_(std::forward<ArgTypes>(args)...));
      }
    }
    else {
      if constexpr (sizeof...(ArgTypes) == 0u) {
        promise_.set_value(std::forward<T>(head));
      }
      else {
        promise_.set_value(value_type_(std::forward<T>(head), std::forward<ArgTypes>(args)...));
      }
    }
  }

private:
  promise_type_ promise_;
}; // class resume_handler_<NoThrow, T, ArgTypes...>

} // namespace detail_

} // namespace corio::thread_unsafe

namespace boost::asio{

template<typename Signature>
class async_result<corio::thread_unsafe::resume_t, Signature>;

template<typename... ArgTypes>
class async_result<corio::thread_unsafe::resume_t, void(ArgTypes...)>
{
public:
  using completion_handler_type = corio::thread_unsafe::detail_::resume_handler_<std::false_type, ArgTypes...>;

  using return_type = typename completion_handler_type::future_type;

  explicit async_result(completion_handler_type &h)
    : future_(h.get_future())
  {}

  async_result(async_result const &) = delete;

  async_result &operator=(async_result const &) = delete;

  template<typename Initiation, typename CompletionToken, typename... Args>
  static return_type initiate(Initiation &&initiation, CompletionToken &&token, Args &&... args)
  {
    completion_handler_type completion_handler(std::forward<CompletionToken>(token));
    async_result r(completion_handler);
    std::forward<Initiation>(initiation)(std::move(completion_handler), std::forward<Args>(args)...);
    return r.get();
  }

  return_type get()
  {
    return std::move(future_);
  }

private:
  return_type future_;
}; // class async_result<corio::thread_unsafe::resume_t, void(ArgTypes...)>

template<typename... ArgTypes>
class async_result<corio::thread_unsafe::nothrow_resume_t, void(ArgTypes...)>
{
public:
  using completion_handler_type = corio::thread_unsafe::detail_::resume_handler_<std::true_type, ArgTypes...>;

  using return_type = typename completion_handler_type::future_type;

  explicit async_result(completion_handler_type &h)
    : future_(h.get_future())
  {}

  async_result(async_result const &) = delete;

  async_result &operator=(async_result const &) = delete;

  template<typename Initiation, typename CompletionToken, typename... Args>
  static return_type initiate(Initiation &&initiation, CompletionToken &&token, Args &&... args)
  {
    completion_handler_type completion_handler(std::forward<CompletionToken>(token));
    async_result r(completion_handler);
    std::forward<Initiation>(initiation)(std::move(completion_handler), std::forward<Args>(args)...);
    return r.get();
  }

  return_type get()
  {
    return std::move(future_);
  }

private:
  return_type future_;
}; // class async_result<corio::thread_unsafe::resume_t, void(ArgTypes...)>

} // namespace boost::asio

#endif // !defined(CORIO_THREAD_UNSAFE_RESUME_HPP_INCLUDE_GUARD)
