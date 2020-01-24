#if !defined(CORIO_THREAD_UNSAFE_COROUTINE_PROMISE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_COROUTINE_PROMISE_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/coroutine.hpp>
#include <corio/thread_unsafe/future.hpp>
#include <corio/thread_unsafe/promise.hpp>
#include <corio/thread_unsafe/coroutine_cleanup_service.hpp>
#include <corio/core/enable_if_executor.hpp>
#include <corio/core/is_executor.hpp>
#include <corio/core/enable_if_execution_context.hpp>
#include <corio/core/error.hpp>
#include <corio/util/throw.hpp>
#include <corio/util/assert.hpp>
#include <boost/asio/execution_context.hpp>
#include <type_traits>
#include <utility>
#include <experimental/coroutine>
#include <exception>


namespace corio::thread_unsafe{

struct this_executor_t
{};

inline constexpr this_executor_t this_executor{};

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

template<typename R, typename Executor>
class coroutine_promise_mixin_
{
protected:
  using executor_type = Executor;
  static_assert(corio::is_executor_v<executor_type>);
  using promise_type_ = corio::thread_unsafe::basic_promise<R, executor_type>;
  using future_type_ = corio::thread_unsafe::basic_future<R, executor_type>;

  coroutine_promise_mixin_()
    : promise_(),
      ownership_(),
      refcount_()
  {}

  coroutine_promise_mixin_(coroutine_promise_mixin_ const &) = delete;

  coroutine_promise_mixin_ &operator=(coroutine_promise_mixin_ const &) = delete;

  void acquire()
  {
    ++refcount_;
  }

  void release()
  {
    if (--refcount_ == 0u) {
      ownership_ = corio::thread_unsafe::coroutine_ownership();
    }
  }

  bool has_executor() const
  {
    return promise_.has_executor();
  }

  executor_type get_executor() const
  {
    return promise_.get_executor();
  }

  std::experimental::suspend_always initial_suspend() noexcept
  {
    return {};
  }

  auto await_transform(this_executor_t)
  {
    if (!has_executor()) {
      CORIO_THROW<corio::no_executor_error>();
    }

    class awaiter
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

      void await_suspend(std::experimental::coroutine_handle<>)
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

  void unhandled_exception() noexcept
  {
    std::exception_ptr p = std::current_exception();
    CORIO_ASSERT(p != nullptr);
    promise_.set_exception(std::move(p));
  }

  std::experimental::suspend_always final_suspend() noexcept
  {
    release();
    return {};
  }

protected:
  promise_type_ promise_;
  corio::thread_unsafe::coroutine_ownership ownership_;
  std::size_t refcount_;
}; // class coroutine_promise_mixin_

} // namespace detail_

template<typename R, typename Executor>
class basic_coroutine_promise
  : private detail_::coroutine_promise_mixin_<R, Executor>
{
public:
  using executor_type = Executor;
  static_assert(corio::is_executor_v<executor_type>);

private:
  using mixin_type_ = detail_::coroutine_promise_mixin_<R, executor_type>;
  using typename mixin_type_::promise_type_;
  using mixin_type_::promise_;
  using mixin_type_::ownership_;
  using mixin_type_::refcount_;
  using this_type_ = basic_coroutine_promise<R, executor_type>;
  using handle_type_ = std::experimental::coroutine_handle<this_type_>;
  using typename mixin_type_::future_type_;

public:
  using coroutine_type = corio::thread_unsafe::basic_coroutine<R, executor_type>;

  basic_coroutine_promise() = default;

  using mixin_type_::acquire;

  using mixin_type_::release;

  using mixin_type_::has_executor;

  void set_executor(executor_type const &executor)
  {
    set_executor_(executor_type(executor));
  }

  void set_executor(executor_type &&executor)
  {
    promise_.set_executor(std::move(executor));
    executor_type exec = promise_.get_executor();
    auto &ctx = exec.context();
    using service_type = corio::thread_unsafe::coroutine_cleanup_service;
    service_type &service = boost::asio::has_service<service_type>(ctx)
      ? boost::asio::use_service<service_type>(ctx)
      : boost::asio::make_service<service_type>(ctx, exec);
    handle_type_ h = handle_type_::from_promise(*this);
    ownership_ = service.register_for_cleanup(std::move(h));
    acquire();
  }

  using mixin_type_::get_executor;

  coroutine_type get_return_object()
  {
    return coroutine_type(*this);
  }

  using mixin_type_::initial_suspend;

  using mixin_type_::await_transform;

  template<typename T>
  std::remove_cv_t<T> await_transform(T &&awaitable)
  {
    using type = std::decay_t<T>;
    if constexpr (detail_::can_have_executor<type>(0)) {
      if (has_executor()) {
        if (awaitable.has_executor()) {
          if (get_executor() != awaitable.get_executor()) {
            CORIO_THROW<corio::bad_executor_error>();
          }
          return std::forward<T>(awaitable);
        }
        awaitable.set_executor(get_executor());
        return std::forward<T>(awaitable);
      }

      if (!awaitable.has_executor()) {
        CORIO_THROW<corio::no_executor_error>();
      }

      set_executor(awaitable.get_executor());
      return std::forward<T>(awaitable);
    }

    if (!has_executor()) {
      CORIO_THROW<corio::no_executor_error>();
    }
    return std::forward<T>(awaitable);
  }

  template<typename T>
  void return_value(T &&value)
  {
    promise_.set_value(std::forward<T>(value));
  }

  using mixin_type_::unhandled_exception;

  using mixin_type_::final_suspend;
}; // class basic_coroutine_promise

template<typename Executor>
class basic_coroutine_promise<void, Executor>
  : private detail_::coroutine_promise_mixin_<void, Executor>
{
public:
  using executor_type = Executor;
  static_assert(corio::is_executor_v<executor_type>);

private:
  using mixin_type_ = detail_::coroutine_promise_mixin_<void, executor_type>;
  using typename mixin_type_::promise_type_;
  using mixin_type_::promise_;
  using this_type_ = basic_coroutine_promise<void, executor_type>;
  using handle_type_ = std::experimental::coroutine_handle<this_type_>;
  using typename mixin_type_::future_type_;

public:
  using coroutine_type = corio::thread_unsafe::basic_coroutine<void, executor_type>;

  basic_coroutine_promise() = default;

  using mixin_type_::acquire;

  using mixin_type_::release;

  using mixin_type_::has_executor;

  void set_executor(executor_type const &executor)
  {
    set_executor_(executor_type(executor));
  }

  void set_executor(executor_type &&executor)
  {
    promise_.set_executor(std::move(executor));
    executor_type exec = promise_.get_executor();
    auto &ctx = exec.context();
    using service_type = corio::thread_unsafe::coroutine_cleanup_service;
    service_type &service = boost::asio::has_service<service_type>(ctx)
      ? boost::asio::use_service<service_type>(ctx)
      : boost::asio::make_service<service_type>(ctx, exec);
    handle_type_ h = handle_type_::from_promise(*this);
    service.register_for_cleanup(std::move(h));
    acquire();
  }

  using mixin_type_::get_executor;

  coroutine_type get_return_object()
  {
    return coroutine_type(*this);
  }

  using mixin_type_::initial_suspend;

  using mixin_type_::await_transform;

  template<typename T>
  std::remove_cv_t<T> await_transform(T &&awaitable)
  {
    using type = std::decay_t<T>;
    if constexpr (detail_::can_have_executor<type>(0)) {
      if (has_executor()) {
        if (awaitable.has_executor()) {
          if (get_executor() != awaitable.get_executor()) {
            CORIO_THROW<corio::bad_executor_error>();
          }
          return std::forward<T>(awaitable);
        }
        awaitable.set_executor(get_executor());
        return std::forward<T>(awaitable);
      }

      if (!awaitable.has_executor()) {
        CORIO_THROW<corio::no_executor_error>();
      }

      set_executor(awaitable.get_executor());
      return std::forward<T>(awaitable);
    }

    if (!has_executor()) {
      CORIO_THROW<corio::no_executor_error>();
    }
    return std::forward<T>(awaitable);
  }

  void return_void()
  {
    promise_.set_value();
  }

  using mixin_type_::unhandled_exception;

  using mixin_type_::final_suspend;
}; // class basic_coroutine_promise<void, Executor>

} // namespace corio::thread_unsafe

#endif // !defined(CORIO_THREAD_UNSAFE_COROUTINE_PROMISE_HPP_INCLUDE_GUARD)
