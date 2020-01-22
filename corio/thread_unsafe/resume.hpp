#if !defined(CORIO_THREAD_UNSAFE_RESUME_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_RESUME_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/resume_fwd.hpp>
#include <corio/thread_unsafe/promise.hpp>
#include <corio/util/remove_rvalue_reference.hpp>
#include <boost/asio/async_result.hpp>
#include <type_traits>
#include <tuple>
#include <utility>


namespace corio::thread_unsafe{

namespace detail_{

template<typename... ArgTypes>
class resume_handler_
{
private:
  using tuple_type_ = std::tuple<std::remove_cv_t<corio::remove_rvalue_reference_t<ArgTypes> >...>;
  using promise_type_ = corio::thread_unsafe::promise<tuple_type_>;

public:
  using future_type = typename promise_type_::future_type;

  explicit resume_handler_(resume_t)
    : promise_()
  {}

  resume_handler_(resume_handler_ const &) = delete;

  resume_handler_(resume_handler_ &&rhs) = default;

  resume_handler_ &operator=(resume_handler_ const &) = delete;

  future_type get_future()
  {
    return promise_.get_future();
  }

  void operator()(ArgTypes... args)
  {
    tuple_type_ value(std::forward<ArgTypes>(args)...);
    promise_.set_value(std::move(value));
  }

private:
  promise_type_ promise_;
}; // class resume_handler_

} // namespace detail_

} // namespace corio::thread_unsafe

namespace boost::asio{

template<typename Signature>
class async_result<corio::thread_unsafe::resume_t, Signature>;

template<typename... ArgTypes>
class async_result<corio::thread_unsafe::resume_t, void(ArgTypes...)>
{
public:
  using completion_handler_type = corio::thread_unsafe::detail_::resume_handler_<ArgTypes...>;

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
