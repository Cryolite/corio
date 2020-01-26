#if !defined(CORIO_THREAD_UNSAFE_COROUTINE_PROMISE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_COROUTINE_PROMISE_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/coroutine.hpp>
#include <corio/thread_unsafe/future.hpp>
#include <corio/thread_unsafe/promise.hpp>
#include <corio/thread_unsafe/coroutine_cleanup_service.hpp>
#include <corio/core/is_executor.hpp>
#include <corio/core/error.hpp>
#include <corio/util/throw.hpp>
#include <corio/util/assert.hpp>
#include <boost/asio/execution_context.hpp>
#include <boost/config.hpp>
#include <list>
#include <type_traits>
#include <utility>
#include <experimental/coroutine>
#include <exception>
#include <cstddef>


namespace corio::thread_unsafe{

struct this_executor_t
{};

inline constexpr this_executor_t this_executor{};

template<typename R, typename Executor>
class coroutine_promise;

namespace detail_{

template<typename T>
inline constexpr bool can_have_executor(void const *) noexcept
{
  return false;
}

template<typename T>
inline constexpr decltype(std::declval<T>().has_executor(), false) can_have_executor(int) noexcept
{
  return true;
}

template<typename Derived>
class coroutine_promise_return_mixin_;

template<typename R, typename Executor>
class coroutine_promise_return_mixin_<corio::thread_unsafe::coroutine_promise<R, Executor> >
{
private:
  using derived_type_ = corio::thread_unsafe::coroutine_promise<R, Executor>;

public:
  template<typename T>
  void return_value(T &&value)
  {
    static_cast<derived_type_ *>(this)->promise_.set_value(std::forward<T>(value));
  }
}; // class coroutine_promise_return_mixin_<corio::thread_unsafe<R, Executor> >

template<typename Executor>
class coroutine_promise_return_mixin_<corio::thread_unsafe::coroutine_promise<void, Executor> >
{
private:
  using derived_type_ = corio::thread_unsafe::coroutine_promise<void, Executor>;

public:
  void return_void()
  {
    static_cast<derived_type_ *>(this)->promise_.set_value();
  }
}; // class coroutine_promise_return_mixin_<corio::thread_unsafe<void, Executor> >

} // namespace detail_

template<typename R, typename Executor>
class coroutine_promise
  : public detail_::coroutine_promise_return_mixin_<coroutine_promise<R, Executor> >
{
public:
  using executor_type = Executor;
  static_assert(corio::is_executor_v<executor_type>);

private:
  using this_type_ = coroutine_promise<R, executor_type>;
  friend class detail_::coroutine_promise_return_mixin_<coroutine_promise<R, Executor> >;

public:
  using cleanup_canceller_type = std::list<corio::thread_unsafe::coroutine_cleanup_canceller>;
  using handle_type = std::experimental::coroutine_handle<this_type_>;

private:
  using promise_type_ = corio::thread_unsafe::basic_promise<R, executor_type>;
  using cleanup_service_type_ = corio::thread_unsafe::coroutine_cleanup_service;
  using future_type_ = corio::thread_unsafe::basic_future<R, executor_type>;

public:
  using coroutine_type = corio::thread_unsafe::basic_coroutine<R, executor_type>;

  coroutine_promise()
    : handle_(handle_type::from_promise(*this)),
      promise_(),
      cleanup_canceller_(),
      refcount_()
  {}

  coroutine_promise(coroutine_promise const &) = delete;

  coroutine_promise &operator=(coroutine_promise const &) = delete;

  void acquire()
  {
    ++refcount_;
  }

  std::size_t release()
  {
    CORIO_ASSERT(refcount_ > 0u);
    return --refcount_;
  }

  bool has_executor() const
  {
    CORIO_ASSERT(promise_.has_executor() || !cleanup_canceller_.valid());
    return promise_.has_executor();
  }

  executor_type get_executor() const
  {
    CORIO_ASSERT(has_executor());
    return promise_.get_executor();
  }

  void set_executor(executor_type const &executor)
  {
    CORIO_ASSERT(!has_executor());
    set_executor_(executor_type(executor));
  }

  void set_executor(executor_type const &executor, cleanup_canceller_type &cleanup_canceller)
  {
    CORIO_ASSERT(!has_executor());
    set_executor_(executor_type(executor), cleanup_canceller);
  }

  void set_executor(executor_type &&executor)
  {
    CORIO_ASSERT(!has_executor());
    promise_.set_executor(std::move(executor));
  }

  void set_executor(executor_type &&executor, cleanup_canceller_type &cleanup_canceller)
  {
    CORIO_ASSERT(!has_executor());
    promise_.set_executor(std::move(executor));
    executor_type exec = promise_.get_executor();
    auto &ctx = exec.context();
    cleanup_service_type_ &service = boost::asio::has_service<cleanup_service_type_>(ctx)
      ? boost::asio::use_service<cleanup_service_type_>(ctx)
      : boost::asio::make_service<cleanup_service_type_>(ctx);
    service.reserve(cleanup_canceller);
  }

  void register_for_cleanup(cleanup_canceller_type &cleanup_canceller) noexcept
  {
    CORIO_ASSERT(has_executor());
    CORIO_ASSERT(cleanup_canceller.size() == 1u);
    executor_type exec = promise_.get_executor();
    auto &ctx = exec.context();
    CORIO_ASSERT(boost::asio::has_service<cleanup_service_type_>(ctx));
    cleanup_service_type_ &service = boost::asio::use_service<cleanup_service_type_>(ctx);
    cleanup_canceller_ = service.register_for_cleanup(handle_, cleanup_canceller);
  }

  bool registered_for_cleanup() noexcept
  {
    return cleanup_canceller_.valid();
  }

  void cancel_cleanup() noexcept
  {
    CORIO_ASSERT(cleanup_canceller_.valid());
    cleanup_canceller_.execute();
  }

  void resume()
  {
    handle_.resume();
  }

  bool done() const noexcept
  {
    return handle_.done();
  }

  handle_type get_handle() const noexcept
  {
    return handle_;
  }

  coroutine_type get_return_object()
  {
    return coroutine_type(*this);
  }

  std::experimental::suspend_always initial_suspend() noexcept
  {
    return {};
  }

  [[nodiscard]] auto await_transform(this_executor_t)
  {
    if (BOOST_UNLIKELY(!has_executor())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::no_executor_error>();
    }

    class [[nodiscard]] awaiter
    {
    public:
      explicit awaiter(executor_type &&executor) noexcept
        : executor_(executor)
      {}

      awaiter(awaiter const &) = delete;

      awaiter &operator=(awaiter const &) = delete;

      bool await_ready()
      {
        return true;
      }

      void await_suspend(handle_type const &)
      {}

      executor_type await_resume()
      {
        return executor_;
      }

    private:
      executor_type executor_;
    }; // struct awaiter

    return awaiter(get_executor());
  }

  template<typename T>
  std::remove_cv_t<T> await_transform(T &&awaitable)
  {
    using type = std::decay_t<T>;
    if constexpr (detail_::can_have_executor<type>(0)) {
      if (has_executor()) {
        if (awaitable.has_executor()) {
          if (BOOST_UNLIKELY(get_executor() != awaitable.get_executor())) /*[[unlikely]]*/ {
            CORIO_THROW<corio::bad_executor_error>();
          }
          return std::forward<T>(awaitable);
        }
        executor_type executor = get_executor();
        awaitable.set_executor(std::move(executor));
        return std::forward<T>(awaitable);
      }

      if (BOOST_UNLIKELY(!awaitable.has_executor())) /*[[unlikely]]*/ {
        CORIO_THROW<corio::no_executor_error>();
      }

      executor_type executor = awaitable.get_executor();
      set_executor(std::move(executor));
      return std::forward<T>(awaitable);
    }

    if (BOOST_UNLIKELY(!has_executor())) /*[[unlikely]]*/ {
      CORIO_THROW<corio::no_executor_error>();
    }
    return std::forward<T>(awaitable);
  }

  void unhandled_exception() noexcept
  {
    std::exception_ptr p = std::current_exception();
    CORIO_ASSERT(p != nullptr);
    promise_.set_exception(std::move(p));
  }

  auto final_suspend() noexcept
  {
    if (cleanup_canceller_.valid()) {
      cleanup_canceller_.execute();
    }

    class awaiter
    {
    public:
      explicit awaiter(bool ready) noexcept
        : ready_(ready)
      {}

      awaiter(awaiter const &rhs) = default;

      awaiter &operator=(awaiter const &) = delete;

      bool await_ready() const noexcept
      {
        return ready_;
      }

      void await_suspend(handle_type const &) const noexcept
      {}

      void await_resume() const noexcept
      {}

    private:
      bool ready_;
    }; // class awaiter

    return awaiter(refcount_ == 0u);
  }

protected:
  handle_type handle_;
  promise_type_ promise_;
  corio::thread_unsafe::coroutine_cleanup_canceller cleanup_canceller_;
  std::size_t refcount_;
}; // class coroutine_promise

} // namespace corio::thread_unsafe

#endif // !defined(CORIO_THREAD_UNSAFE_COROUTINE_PROMISE_HPP_INCLUDE_GUARD)
