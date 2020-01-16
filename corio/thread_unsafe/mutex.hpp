#if !defined(CORIO_THREAD_UNSAFE_MUTEX_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_MUTEX_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/detail_/mutex_service_.hpp>
#include <corio/util/assert.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/execution_context.hpp>
#include <type_traits>
#include <utility>


namespace corio::thread_unsafe{

template<typename ExecutionContext>
class basic_mutex
{
public:
  using context_type = ExecutionContext;

private:
  static_assert(!std::is_const_v<context_type>);
  static_assert(!std::is_volatile_v<context_type>);
  static_assert(!std::is_reference_v<context_type>);
  static_assert(std::is_convertible_v<context_type &, boost::asio::execution_context &>);

  using service_type_ = corio::thread_unsafe::detail_::mutex_service_<context_type>;

public:
  explicit basic_mutex(ExecutionContext &ctx)
    : ctx_(ctx),
      service_(boost::asio::has_service<service_type_>(ctx_)
               ? boost::asio::use_service<service_type_>(ctx_)
               : boost::asio::make_service<service_type_>(ctx_, ctx_)),
      locked_()
  {}

  basic_mutex(basic_mutex const &) = delete;

  basic_mutex &operator=(basic_mutex const &) = delete;

  template<typename CompletionToken>
  auto async_lock(CompletionToken &&token)
  {
    using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void()>;
    using completion_handler_type = typename async_result_type::completion_handler_type;
    completion_handler_type completion_handler(std::move(token));
    async_result_type async_result(completion_handler);
    service_.async_lock(locked_, std::move(completion_handler));
    return async_result.get();
  }

  bool try_lock()
  {
    return locked_ ? false : (locked_ = true);
  }

  void unlock()
  {
    CORIO_ASSERT(locked_);
    service_.unlock(locked_);
  }

private:
  context_type &ctx_;
  service_type_ &service_;
  bool locked_;
}; // class basic_mutex

} // namespace corio::thread_unsafe

#endif // !defined(CORIO_THREAD_UNSAFE_MUTEX_HPP_INCLUDE_GUARD)
