#if !defined(CORIO_THREAD_UNSAFE_DETAIL_COROUTINE_CLEANUP_SERVICE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_DETAIL_COROUTINE_CLEANUP_SERVICE_HPP_INCLUDE_GUARD

#include <corio/core/error.hpp>
#include <corio/util/is_empty.hpp>
#include <corio/util/throw.hpp>
#include <corio/util/assert.hpp>
#include <boost/asio/execution_context.hpp>
#include <boost/config.hpp>
#include <list>
#include <memory>
#include <utility>
#include <experimental/coroutine>


namespace corio::thread_unsafe::detail_{

class coroutine_handle_impl_
{
public:
  using list_type = std::list<std::shared_ptr<coroutine_handle_impl_> >;
  using iterator_type = list_type::iterator;
  using handle_type = std::experimental::coroutine_handle<>;

  coroutine_handle_impl_() noexcept
    : p_active_list_(),
      p_reserved_list_(),
      iterator_(),
      handle_()
  {}

  void assign(list_type &active_list, list_type &reserved_list,
              iterator_type const &iterator, handle_type &&handle) noexcept
  {
    CORIO_ASSERT(p_active_list_ == nullptr);
    CORIO_ASSERT(p_reserved_list_ == nullptr);
    CORIO_ASSERT(handle_ == nullptr);
    p_active_list_ = &active_list;
    p_reserved_list_ = &reserved_list;
    iterator_ = iterator;
    handle_ = std::move(handle);
  }

  coroutine_handle_impl_(coroutine_handle_impl_ const &) = delete;

  coroutine_handle_impl_ &operator=(coroutine_handle_impl_ const &) = delete;

private:
  void unregister_with_service_() noexcept
  {
    p_reserved_list_->splice(p_reserved_list_->cbegin(), *p_active_list_, iterator_);
  }

public:
  void destroy() noexcept
  {
    CORIO_ASSERT(p_active_list_ != nullptr);
    CORIO_ASSERT(p_reserved_list_ != nullptr);
    CORIO_ASSERT(handle_ != nullptr);
    unregister_with_service_();
    handle_.destroy();
    handle_ = nullptr;
  }

  void notify_flow_off() noexcept
  {
    CORIO_ASSERT(p_active_list_ != nullptr);
    CORIO_ASSERT(p_reserved_list_ != nullptr);
    CORIO_ASSERT(handle_ != nullptr);
    unregister_with_service_();
    handle_ = nullptr;
  }

  void notify_shutdown() noexcept
  {
    CORIO_ASSERT(p_active_list_ != nullptr);
    CORIO_ASSERT(p_reserved_list_ != nullptr);
    CORIO_ASSERT(handle_ != nullptr);
    p_active_list_ = nullptr;
    p_reserved_list_ = nullptr;
    handle_.destroy();
    handle_ = nullptr;
  }

  void resume()
  {
    CORIO_ASSERT(p_active_list_ != nullptr);
    CORIO_ASSERT(p_reserved_list_ != nullptr);
    CORIO_ASSERT(handle_ != nullptr);
    handle_.resume();
  }

  bool done() const noexcept
  {
    CORIO_ASSERT(p_active_list_ != nullptr);
    CORIO_ASSERT(p_reserved_list_ != nullptr);
    CORIO_ASSERT(handle_ != nullptr);
    return handle_.done();
  }

private:
  list_type *p_active_list_;
  list_type *p_reserved_list_;
  iterator_type iterator_;
  handle_type handle_;
}; // class coroutine_handle_impl_

class coroutine_handle_
{
public:
  coroutine_handle_() noexcept
    : p_()
  {}

  explicit coroutine_handle_(std::shared_ptr<coroutine_handle_impl_> const &p) noexcept
    : p_(p)
  {
    CORIO_ASSERT(p != nullptr);
    CORIO_ASSERT(!corio::is_empty(p));
  }

  void resume()
  {
    if (BOOST_UNLIKELY(corio::is_empty(p_))) /*[[unlikely]]*/ {
      CORIO_THROW<corio::invalid_coroutine_handle_error>();
    }
    std::shared_ptr<coroutine_handle_impl_> p = p_.lock();
    if (BOOST_UNLIKELY(p.get() == nullptr)) /*[[unlikely]]*/ {
      CORIO_THROW<corio::coroutine_already_destroyed_error>();
    }
    p->resume();
  }

  void destroy() noexcept
  {
    CORIO_ASSERT(!corio::is_empty(p_));
    std::shared_ptr<coroutine_handle_impl_> p = p_.lock();
    CORIO_ASSERT(p.get() != nullptr);
    p->destroy();
    p.reset();
  }

  void notify_flow_off() noexcept
  {
    CORIO_ASSERT(!corio::is_empty(p_));
    std::shared_ptr<coroutine_handle_impl_> p = p_.lock();
    CORIO_ASSERT(p.get() != nullptr);
    p->notify_flow_off();
    p.reset();
  }

  bool valid() const noexcept
  {
    return !corio::is_empty(p_);
  }

  bool done() const noexcept
  {
    std::shared_ptr<coroutine_handle_impl_> p = p_.lock();
    return p.get() != nullptr && p->done();
  }

  bool destroyed() const noexcept
  {
    return valid() && p_.expired();
  }

private:
  std::weak_ptr<coroutine_handle_impl_> p_;
}; // class coroutine_handle_

class coroutine_cleanup_service_
  : public boost::asio::execution_context::service
{
private:
  using list_type_ = std::list<std::shared_ptr<coroutine_handle_impl_> >;

public:
  using handle_type = std::experimental::coroutine_handle<>;

  inline static boost::asio::execution_context::id id{};

  explicit coroutine_cleanup_service_(boost::asio::execution_context &context)
    : boost::asio::execution_context::service(context),
      active_list_(),
      reserved_list_()
  {}

  coroutine_cleanup_service_(coroutine_cleanup_service_ const &) = delete;

  coroutine_cleanup_service_ &operator=(coroutine_cleanup_service_ const &) = delete;

  coroutine_handle_ get_handle(handle_type &&handle)
  {
    if (reserved_list_.empty()) {
      auto p = std::make_shared<coroutine_handle_impl_>();
      reserved_list_.emplace_front(std::move(p));
    }
    using iterator_type = list_type_::iterator;
    iterator_type iterator = reserved_list_.begin();
    reserved_list_.front()->assign(active_list_, reserved_list_, iterator, std::move(handle));
    active_list_.splice(active_list_.cbegin(), reserved_list_, reserved_list_.cbegin());
    return coroutine_handle_(active_list_.front());
  }

private:
  virtual void shutdown() noexcept override
  {
    for (auto &p : active_list_) {
      CORIO_ASSERT(!corio::is_empty(p));
      CORIO_ASSERT(p.get() != nullptr);
      p->notify_shutdown();
    }
    active_list_.clear();
    reserved_list_.clear();
  }

private:
  list_type_ active_list_;
  list_type_ reserved_list_;
}; // class coroutine_cleanup_service_

} // namespace corio::thread_unsafe::detail_

#endif // !defined(CORIO_THREAD_UNSAFE_DETAIL_COROUTINE_CLEANUP_SERVICE_HPP_INCLUDE_GUARD)
