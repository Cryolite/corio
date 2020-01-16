#if !defined(CORIO_THREAD_UNSAFE_CONTEXTLESS_MUTEX_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_CONTEXTLESS_MUTEX_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/detail_/mutex_service_.hpp>
#include <corio/core/error.hpp>
#include <corio/util/throw.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/execution_context.hpp>
#include <type_traits>
#include <functional>
#include <utility>


namespace corio::thread_unsafe::contextless{

class mutex
{
private:
  class impl_base_
  {
  public:
    virtual ~impl_base_()
    {}

    virtual void shutdown() noexcept = 0;

    virtual void async_lock(bool &locked, std::function<void()> &&h) = 0;

    virtual void unlock(bool &locked) = 0;
  }; // class impl_base

  template<typename MutexService>
  class impl_
    : public impl_base_
  {
  public:
    explicit impl_(MutexService &service) noexcept
      : service_(service)
    {}

    impl_(impl_ &) = delete;

  private:
    virtual void shutdown() noexcept override
    {
      service_.shutdown();
    }

    virtual void async_lock(bool &locked, std::function<void()> &&h) override
    {
      service_.async_lock(locked, std::move(h));
    }

    virtual void unlock(bool &locked) override
    {
      service_.unlock(locked);
    }

  private:
    MutexService &service_;
  }; // class impl_

public:
  mutex() noexcept
    : p_(),
      locked_()
  {}

  mutex(mutex const &) = delete;

  ~mutex()
  {
    delete p_;
  }

  mutex &operator=(mutex const &) = delete;

  virtual void shutdown() noexcept
  {
    if (p_ == nullptr) {
      return;
    }
    p_->shutdown();
  }

  template<typename ExecutionContext>
  void set_context(ExecutionContext &ctx)
  {
    if (p_ != nullptr) {
      CORIO_THROW<corio::context_already_set_error>("");
    }
    using context_type_ = ExecutionContext;
    using service_type_ = corio::thread_unsafe::detail_::mutex_service_<context_type_>;
    service_type_ &service = boost::asio::has_service<service_type_>(ctx)
      ? boost::asio::use_service<service_type_>(ctx) : boost::asio::make_service<service_type_>(ctx, ctx);
    p_ = new impl_<service_type_>(service);
  }

  bool has_context() const noexcept
  {
    return p_ != nullptr;
  }

  template<typename CompletionToken>
  auto async_lock(CompletionToken &&token)
  {
    if (p_ == nullptr) {
      CORIO_THROW<corio::no_context_error>("");
    }
    using async_result_type = boost::asio::async_result<std::decay_t<CompletionToken>, void()>;
    using completion_handler_type = typename async_result_type::completion_handler_type;
    completion_handler_type completion_handler(std::move(token));
    async_result_type async_result(completion_handler);
    p_->async_lock(locked_, std::move(completion_handler));
    return async_result.get();
  }

  bool try_lock()
  {
    if (p_ == nullptr) {
      CORIO_THROW<corio::no_context_error>("");
    }
    return locked_ ? false : (locked_ = true);
  }

  void unlock()
  {
    if (p_ == nullptr) {
      CORIO_THROW<corio::no_context_error>("");
    }
    p_->unlock(locked_);
  }

private:
  impl_base_ *p_;
  bool locked_;
}; // class mutex

} // namespace corio::thread_unsafe::contextless

#endif // !defined(CORIO_THREAD_UNSAFE_CONTEXTLESS_MUTEX_HPP_INCLUDE_GUARD)
