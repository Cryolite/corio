#if !defined(CORIO_THREAD_UNSAFE_COROUTINE_PROMISE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_COROUTINE_PROMISE_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/coroutine.hpp>
#include <corio/thread_unsafe/detail_/coroutine_cleanup_service_.hpp>
#include <corio/thread_unsafe/detail_/use_future_.hpp>
#include <corio/thread_unsafe/future.hpp>
#include <corio/thread_unsafe/promise.hpp>
#include <corio/core/use_future.hpp>
#include <corio/core/this_executor.hpp>
#include <corio/core/is_executor.hpp>
#include <corio/core/error.hpp>
#include <corio/util/exception_guard.hpp>
#include <corio/util/throw.hpp>
#include <corio/util/assert.hpp>
#include <boost/asio/execution_context.hpp>
#include <boost/config.hpp>
#include <type_traits>
#include <optional>
#include <utility>
#include <experimental/coroutine>
#include <exception>
#include <cstddef>


namespace corio::thread_unsafe{

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
    static_cast<derived_type_ *>(this)->promise_->set_value(std::forward<T>(value));
  }
}; // class coroutine_promise_return_mixin_<corio::thread_unsafe::coroutine_promise<R, Executor> >

template<typename Executor>
class coroutine_promise_return_mixin_<corio::thread_unsafe::coroutine_promise<void, Executor> >
{
private:
  using derived_type_ = corio::thread_unsafe::coroutine_promise<void, Executor>;

public:
  void return_void()
  {
    static_cast<derived_type_ *>(this)->promise_->set_value();
  }
}; // class coroutine_promise_return_mixin_<corio::thread_unsafe::coroutine_promise<void, Executor> >

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
  friend class detail_::coroutine_promise_return_mixin_<this_type_>;
  using raw_handle_type_ = std::experimental::coroutine_handle<this_type_>;
  using handle_type_ = corio::thread_unsafe::detail_::coroutine_handle_;
  using promise_type_ = corio::thread_unsafe::basic_promise<R, executor_type>;

public:
  using coroutine_type = corio::thread_unsafe::basic_coroutine<R, executor_type>;
  using future_type = corio::thread_unsafe::basic_future<R, executor_type>;

  coroutine_promise()
    : handle_(),
      promise_(),
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

  coroutine_type get_return_object()
  {
    CORIO_ASSERT(!handle_.valid());
    CORIO_ASSERT(!promise_.has_value());
    CORIO_ASSERT(refcount_ == 0u);
    return coroutine_type(*this);
  }

  template<typename OtherExecutor>
  corio::thread_unsafe::detail_::coroutine_handle_ set_executor(OtherExecutor &&executor)
  {
    CORIO_ASSERT(!handle_.valid());
    CORIO_ASSERT(!promise_.has_value());
    CORIO_ASSERT(refcount_ > 0u);
    promise_.emplace(std::forward<OtherExecutor>(executor));
    CORIO_EXCEPTION_GUARD(){
      promise_ = std::nullopt;
    };
    executor_type e = promise_->get_executor();
    auto &context = e.context();
    using service_type = corio::thread_unsafe::detail_::coroutine_cleanup_service_;
    service_type &service = boost::asio::has_service<service_type>(context)
      ? boost::asio::use_service<service_type>(context)
      : boost::asio::make_service<service_type>(context);
    raw_handle_type_ raw_handle = raw_handle_type_::from_promise(*this);
    handle_ = service.get_handle(std::move(raw_handle));
    return handle_;
  }

  executor_type get_executor() const
  {
    CORIO_ASSERT(handle_.valid());
    CORIO_ASSERT(!handle_.destroyed());
    CORIO_ASSERT(!handle_.done());
    CORIO_ASSERT(promise_.has_value());
    CORIO_ASSERT(refcount_ > 0u);
    return promise_->get_executor();
  }

  future_type get_future()
  {
    CORIO_ASSERT(handle_.valid());
    CORIO_ASSERT(!handle_.destroyed());
    CORIO_ASSERT(!handle_.done());
    CORIO_ASSERT(promise_.has_value());
    CORIO_ASSERT(refcount_ > 0u);
    return promise_->get_future();
  }

  std::experimental::suspend_always initial_suspend() noexcept
  {
    CORIO_ASSERT(!handle_.valid());
    CORIO_ASSERT(!promise_.has_value());
    CORIO_ASSERT(refcount_ > 0u);
    return {};
  }

  [[nodiscard]] auto await_transform(corio::this_executor_t)
  {
    CORIO_ASSERT(handle_.valid());
    CORIO_ASSERT(!handle_.destroyed());
    CORIO_ASSERT(!handle_.done());
    CORIO_ASSERT(promise_.has_value());

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

      void await_suspend(raw_handle_type_ const &)
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

  template<typename NoThrow>
  [[nodiscard]] auto await_transform(corio::use_future_t<NoThrow>)
  {
    CORIO_ASSERT(handle_.valid());
    CORIO_ASSERT(!handle_.destroyed());
    CORIO_ASSERT(!handle_.done());
    CORIO_ASSERT(promise_.has_value());
    using nothrow_type = NoThrow;
    executor_type executor = get_executor();
    using token_type = corio::thread_unsafe::detail_::use_future_token_<executor_type, nothrow_type>;
    return token_type(std::move(executor));
  }

  template<typename T>
  std::decay_t<T> await_transform(T &&awaitable)
  {
    CORIO_ASSERT(handle_.valid());
    CORIO_ASSERT(!handle_.destroyed());
    CORIO_ASSERT(!handle_.done());
    CORIO_ASSERT(promise_.has_value());
    using type = std::decay_t<T>;
    if constexpr (detail_::can_have_executor<type>(0)) {
      if (BOOST_UNLIKELY(get_executor() != awaitable.get_executor())) /*[[unlikely]]*/ {
        CORIO_THROW<corio::bad_executor_error>();
      }
    }
    return std::forward<T>(awaitable);
  }

  void unhandled_exception() noexcept
  {
    CORIO_ASSERT(handle_.valid());
    CORIO_ASSERT(!handle_.destroyed());
    CORIO_ASSERT(!handle_.done());
    CORIO_ASSERT(promise_.has_value());
    std::exception_ptr p = std::current_exception();
    CORIO_ASSERT(p != nullptr);
    promise_->set_exception(std::move(p));
  }

  auto final_suspend() noexcept
  {
    CORIO_ASSERT(handle_.valid());
    CORIO_ASSERT(!handle_.destroyed());
    CORIO_ASSERT(!handle_.done());
    CORIO_ASSERT(promise_.has_value());

    if (refcount_ == 0u) {
      handle_.notify_flow_off();
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

      void await_suspend(raw_handle_type_ const &) const noexcept
      {}

      void await_resume() const noexcept
      {}

    private:
      bool ready_;
    }; // class awaiter

    return awaiter(refcount_ == 0u);
  }

protected:
  handle_type_ handle_;
  std::optional<promise_type_> promise_;
  std::size_t refcount_;
}; // class coroutine_promise

} // namespace corio::thread_unsafe

#endif // !defined(CORIO_THREAD_UNSAFE_COROUTINE_PROMISE_HPP_INCLUDE_GUARD)
