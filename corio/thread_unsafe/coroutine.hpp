#if !defined(CORIO_THREAD_UNSAFE_COROUTINE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_COROUTINE_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/future.hpp>
#include <corio/core/is_executor.hpp>
#include <corio/core/error.hpp>
#include <corio/util/throw.hpp>
#include <corio/util/assert.hpp>
#include <boost/asio/executor.hpp>
#include <boost/config.hpp>
#include <utility>
#include <experimental/coroutine>


namespace corio::thread_unsafe{

template<typename R, typename Executor>
class basic_coroutine_promise;

template<typename R, typename Executor>
class basic_coroutine
{
public:
  using executor_type = Executor;
  static_assert(corio::is_executor_v<executor_type>);
  using promise_type = basic_coroutine_promise<R, executor_type>;

private:
  using handle_type_ = std::experimental::coroutine_handle<promise_type>;

public:
  using future_type = corio::thread_unsafe::basic_future<R, executor_type>;

private:
  friend class basic_coroutine_promise<R, executor_type>;

  basic_coroutine(handle_type_ &&handle, future_type &&future) noexcept
    : handle_(std::move(handle)),
      future_(std::move(future))
  {}

public:
  basic_coroutine() = default;

  basic_coroutine(basic_coroutine const &) = delete;

  basic_coroutine(basic_coroutine &&rhs)
    : handle_(std::move(rhs.handle_)),
      future_(std::move(rhs.future_))
  {
    rhs.handle_ = nullptr;
  }

  ~basic_coroutine()
  {
    if (handle_ != nullptr) {
      handle_.destroy();
    }
  }

  void swap(basic_coroutine &rhs) noexcept
  {
    using std::swap;
    swap(handle_, rhs.handle_);
    swap(future_, rhs.future_);
  }

  friend void swap(basic_coroutine &lhs, basic_coroutine &rhs) noexcept
  {
    lhs.swap(rhs);
  }

  basic_coroutine &operator=(basic_coroutine const &) = delete;

  basic_coroutine &operator=(basic_coroutine &&rhs)
  {
    basic_coroutine(std::move(rhs)).swap(*this);
    return *this;
  }

  bool has_executor() const
  {
    if (BOOST_UNLIKELY(!future_.valid())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::no_future_state_error>();
    }
    return future_.has_executor();
  }

  void set_executor(executor_type const &executor)
  {
    set_executor(executor_type(executor));
  }

  void set_executor(executor_type &&executor)
  {
    if (BOOST_UNLIKELY(!future_.valid())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::no_future_state_error>();
    }
    future_.set_executor(std::move(executor));
  }

  executor_type get_executor() const
  {
    if (BOOST_UNLIKELY(!future_.valid())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::no_future_state_error>();
    }
    return future_.get_executor();
  }

  bool valid() const noexcept
  {
    CORIO_ASSERT((handle_ != nullptr) == future_.valid());
    return handle_ != nullptr;
  }

  void resume()
  {
    if (!valid()) {
      CORIO_THROW<corio::invalid_coroutine_error>();
    }
    if (done()) {
      CORIO_THROW<corio::coroutine_already_done_error>();
    }
    handle_.resume();
  }

  bool done() const
  {
    if (!valid()) {
      CORIO_THROW<corio::invalid_coroutine_error>();
    }
    return handle_.done();
  }

  future_type get_future() &&
  {
    return std::move(future_);
  }

private:
  handle_type_ handle_;
  future_type future_;
}; // class basic_coroutine

template<typename R>
using coroutine = basic_coroutine<R, boost::asio::executor>;

} // namespace corio::thread_unsafe

#include <corio/thread_unsafe/coroutine_promise.hpp>

#endif // !defined(CORIO_THREAD_UNSAFE_COROUTINE_HPP_INCLUDE_GUARD)
