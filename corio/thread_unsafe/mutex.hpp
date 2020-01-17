#if !defined(CORIO_THREAD_UNSAFE_MUTEX_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_MUTEX_HPP_INCLUDE_GUARD

#include <corio/core/error.hpp>
#include <corio/util/throw.hpp>
#include <corio/util/assert.hpp>
#include <boost/asio/defer.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/execution_context.hpp>
#include <mutex>
#include <deque>
#include <type_traits>
#include <functional>
#include <utility>


namespace corio::thread_unsafe{

namespace detail_{

class mutex_handler_
{
private:
  class impl_base_
  {
  public:
    virtual ~impl_base_()
    {}

    virtual void operator()() = 0;
  }; // class impl_base_

  template<typename F>
  class impl_ final
    : public impl_base_
  {
  private:
    static_assert(!std::is_const_v<F>);
    static_assert(!std::is_volatile_v<F>);
    static_assert(!std::is_reference_v<F>);

  public:
    impl_(F &&f)
      : f_(std::move(f))
    {}

    impl_(impl_ const &) = delete;

    impl_ &operator=(impl_ const &) = delete;

  private:
    virtual void operator()() override final
    {
      std::move(f_)();
    }

  private:
    F f_;
  }; // class impl_

public:
  template<typename F>
  explicit mutex_handler_(F &&f)
    : p_(new impl_<F>(std::move(f)))
  {}

  mutex_handler_(mutex_handler_ const &) = delete;

  mutex_handler_(mutex_handler_ &&rhs)
    : p_(rhs.p_)
  {
    rhs.p_ = nullptr;
  }

  mutex_handler_ &operator=(mutex_handler_ const &) = delete;

  void operator()()
  {
    if (p_ == nullptr) {
      CORIO_THROW<std::bad_function_call>();
    }
    (*p_)();
  }

  ~mutex_handler_()
  {
    if (p_ != nullptr) {
      delete p_;
    }
  }

private:
  impl_base_ *p_;
}; // class mutex_handler_

template<typename ExecutionContext>
class mutex_context_mixin_
{
protected:
  using context_type = ExecutionContext;
  static_assert(!std::is_const_v<context_type>);
  static_assert(!std::is_volatile_v<context_type>);
  static_assert(!std::is_reference_v<context_type>);
  static_assert(std::is_convertible_v<context_type &, boost::asio::execution_context &>);
  using executor_type = typename context_type::executor_type;

  explicit mutex_context_mixin_(context_type &ctx) noexcept
    : ctx_(ctx)
  {}

  mutex_context_mixin_(mutex_context_mixin_ const &) = delete;

  mutex_context_mixin_ &operator=(mutex_context_mixin_ const &) = delete;

  template<typename T>
  void set_context(T &) = delete;

  template<typename CompletionHandler>
  void post(CompletionHandler &&h)
  {
    executor_type ex = ctx_.get_executor();
    boost::asio::post(ex, std::move(h));
  }

  template<typename CompletionHandler>
  void defer(CompletionHandler &&h)
  {
    executor_type ex = ctx_.get_executor();
    boost::asio::defer(ex, std::move(h));
  }

private:
  context_type &ctx_;
}; // class mutex_context_mixin_

template<>
class mutex_context_mixin_<void>
{
private:
  class impl_base_
  {
  public:
    virtual ~impl_base_()
    {}

    virtual void post(mutex_handler_ &&h) = 0;

    virtual void defer(mutex_handler_ &&h) = 0;
  }; // class impl_base_

  template<typename ExecutionContext>
  class impl_ final
    : public impl_base_
  {
  public:
    using context_type = ExecutionContext;
    static_assert(!std::is_const_v<context_type>);
    static_assert(!std::is_volatile_v<context_type>);
    static_assert(!std::is_reference_v<context_type>);
    static_assert(std::is_convertible_v<context_type &, boost::asio::execution_context &>);
    using executor_type = typename context_type::executor_type;

    explicit impl_(context_type &ctx)
      : ctx_(ctx)
    {}

    impl_(impl_ const &) = delete;

    impl_ &operator=(impl_ const &) = delete;

  private:
    virtual void post(mutex_handler_ &&h) override final
    {
      executor_type ex = ctx_.get_executor();
      boost::asio::post(ex, std::move(h));
    }

    virtual void defer(mutex_handler_ &&h) override final
    {
      executor_type ex = ctx_.get_executor();
      boost::asio::defer(ex, std::move(h));
    }

  private:
    context_type &ctx_;
  }; // class impl_

protected:
  mutex_context_mixin_() noexcept
    : p_()
  {}

  mutex_context_mixin_(mutex_context_mixin_ const &) = delete;

  ~mutex_context_mixin_()
  {
    if (p_ != nullptr) {
      delete p_;
    }
  }

  mutex_context_mixin_ &operator=(mutex_context_mixin_ const &) = delete;

  template<typename ExecutionContext>
  void set_context(ExecutionContext &ctx)
  {
    if (p_ != nullptr) {
      CORIO_THROW<corio::context_already_set_error>("");
    }
    p_ = new impl_<ExecutionContext>(ctx);
  }

  void post(mutex_handler_ &&h)
  {
    if (p_ == nullptr) {
      CORIO_THROW<corio::no_context_error>("");
    }
    p_->post(std::move(h));
  }

  void defer(mutex_handler_ &&h)
  {
    if (p_ == nullptr) {
      CORIO_THROW<corio::no_context_error>("");
    }
    p_->defer(std::move(h));
  }

private:
  impl_base_ *p_;
}; // class mutex_context_mixin_<void>

} // namespace detail_

template<typename ExecutionContext>
class basic_mutex
  : private detail_::mutex_context_mixin_<ExecutionContext>
{
public:
  using context_type = ExecutionContext;
  static_assert(!std::is_const_v<context_type>);
  static_assert(!std::is_volatile_v<context_type>);
  static_assert(!std::is_reference_v<context_type>);

private:
  using context_mixin_type_ = detail_::mutex_context_mixin_<ExecutionContext>;
  using handler_queue_type_ = std::deque<detail_::mutex_handler_>;

public:
  basic_mutex() = default;

  template<typename T>
  explicit basic_mutex(T &ctx, std::enable_if_t<std::is_same_v<T, context_type> > * = nullptr)
    : context_mixin_type_(ctx),
      handler_queue_(),
      locked_()
  {}

  basic_mutex(basic_mutex const &) = delete;

  basic_mutex &operator=(basic_mutex const &) = delete;

  using context_mixin_type_::set_context;

  template<typename CompletionToken>
  auto async_lock(CompletionToken &&token)
  {
    using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>,  void()>;
    using completion_handler_type = typename async_result_type::completion_handler_type;
    completion_handler_type completion_handler(std::move(token));
    async_result_type async_result(completion_handler);

    if (!locked_) {
      locked_ = true;
      detail_::mutex_handler_ h(std::move(completion_handler));
      context_mixin_type_::defer(std::move(h));
    }
    else {
      detail_::mutex_handler_ h(std::move(completion_handler));
      handler_queue_.emplace_back(std::move(h));
    }

    return async_result.get();
  }

  bool try_lock()
  {
    return locked_ ? false : (locked_ = true);
  }

  void unlock()
  {
    CORIO_ASSERT(locked_);
    locked_ = false;
    if (!handler_queue_.empty()) {
      locked_ = true;
      detail_::mutex_handler_ h = std::move(handler_queue_.front());
      handler_queue_.pop_front();
      context_mixin_type_::post(std::move(h));
    }
  }

private:
  handler_queue_type_ handler_queue_;
  bool locked_ = false;
}; // class basic_mutex

} // namespace corio::thread_unsafe

#endif // !defined(CORIO_THREAD_UNSAFE_MUTEX_HPP_INCLUDE_GUARD)
