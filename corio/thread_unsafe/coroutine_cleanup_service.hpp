#if !defined(CORIO_THREAD_UNSAFE_COROUTINE_CLEANUP_SERVICE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_COROUTINE_CLEANUP_SERVICE_HPP_INCLUDE_GUARD

#include <corio/util/throw.hpp>
#include <corio/util/assert.hpp>
#include <boost/asio/defer.hpp>
#include <boost/asio/execution_context.hpp>
#include <boost/asio/executor.hpp>
#include <list>
#include <memory>
#include <optional>
#include <tuple>
#include <utility>
#include <stdexcept>
#include <experimental/coroutine>


namespace corio::thread_unsafe{

class coroutine_ownership;

namespace detail_{

class coroutine_ownership_impl_
{
public:
  using handle_type = std::experimental::coroutine_handle<>;
  using value_type = std::tuple<handle_type, std::shared_ptr<coroutine_ownership_impl_> >;
  using list_type = std::list<value_type>;
  using iterator_type = list_type::iterator;

  coroutine_ownership_impl_(
    list_type &list, list_type &reserved_list, iterator_type const &iterator)
    : p_list_(&list),
      p_reserved_list_(&reserved_list),
      iterator_(iterator)
  {}

  coroutine_ownership_impl_(coroutine_ownership_impl_ const &) = delete;

  coroutine_ownership_impl_ &operator=(coroutine_ownership_impl_ const &) = delete;

private:
  friend class corio::thread_unsafe::coroutine_ownership;

  handle_type get_handle() const
  {
    return std::get<0u>(*iterator_);
  }

  void release()
  {
    CORIO_ASSERT((p_list_ != nullptr) == (p_reserved_list_ != nullptr));
    if (p_list_ != nullptr) {
      std::get<0u>(*iterator_) = nullptr;
      std::get<1u>(*iterator_).reset();
      p_reserved_list_->splice(p_reserved_list_->cbegin(), *p_list_, iterator_);
      p_list_ = nullptr;
      p_reserved_list_ = nullptr;
      iterator_ = iterator_type();
    }
  }

public:
  ~coroutine_ownership_impl_()
  {
    release();
  }

private:
  list_type *p_list_;
  list_type *p_reserved_list_;
  iterator_type iterator_;
}; // class coroutine_ownership_impl_

}; // namespace detail_

class coroutine_cleanup_service;

class coroutine_ownership
{
private:
  using executor_type = boost::asio::executor;
  using handle_type = std::experimental::coroutine_handle<>;
  friend class corio::thread_unsafe::coroutine_cleanup_service;

  coroutine_ownership(
    executor_type const &executor,
    std::weak_ptr<detail_::coroutine_ownership_impl_> p) noexcept
    : executor_(executor),
      p_(std::move(p))
  {
    CORIO_ASSERT(!p_.expired());
  }

public:
  coroutine_ownership() = default;

  coroutine_ownership(coroutine_ownership const &) = delete;

  coroutine_ownership(coroutine_ownership &&rhs) = default;

  coroutine_ownership &operator=(coroutine_ownership const &) = delete;

  coroutine_ownership &operator=(coroutine_ownership &&rhs) = default;

  ~coroutine_ownership()
  {
    if (std::shared_ptr<detail_::coroutine_ownership_impl_> p = p_.lock()) {
      handle_type h = p->get_handle();
      CORIO_ASSERT(h != nullptr);
      boost::asio::defer(executor_.value(), [h]() mutable -> void{ h.destroy(); });
      p->release();
    }
  }

private:
  std::optional<boost::asio::executor> executor_;
  std::weak_ptr<detail_::coroutine_ownership_impl_> p_;
}; // class coroutine_ownership

class coroutine_cleanup_service
  : public boost::asio::execution_context::service
{
public:
  using executor_type = boost::asio::executor;
  using context_type = boost::asio::execution_context;
  using handle_type = std::experimental::coroutine_handle<>;

private:
  using value_type_ = std::tuple<handle_type, std::shared_ptr<detail_::coroutine_ownership_impl_> >;
  using list_type_ = std::list<value_type_>;
  using iterator_type_ = list_type_::iterator;

public:
  inline static boost::asio::execution_context::id id{};

  explicit coroutine_cleanup_service(context_type &ctx) noexcept
    : boost::asio::execution_context::service(ctx),
      exec_(),
      list_(),
      reserved_list_()
  {}

  coroutine_cleanup_service(context_type &ctx, executor_type const &exec)
    : boost::asio::execution_context::service(ctx),
      exec_(exec),
      list_(),
      reserved_list_()
  {}

  coroutine_cleanup_service(coroutine_cleanup_service const &) = delete;

  coroutine_cleanup_service &operator=(coroutine_cleanup_service const &) = delete;

  coroutine_ownership register_for_cleanup(handle_type const &h)
  {
    CORIO_ASSERT(exec_.has_value());
    if (h == nullptr) {
      CORIO_THROW<std::invalid_argument>(
        "A null pointer of the type `std::coroutine_handle<>' is passed to"
        " `coroutine_cleanup_service::register_for_cleanup'.");
    }
    if (!reserved_list_.empty()) {
      CORIO_ASSERT(std::get<0u>(reserved_list_.front()) == nullptr);
      CORIO_ASSERT(std::get<1u>(reserved_list_.front()) == nullptr);
      list_.splice(list_.cbegin(), reserved_list_, reserved_list_.cbegin());
    }
    else {
      list_.emplace_front(nullptr, nullptr);
    }
    iterator_type_ iter = list_.begin();
    auto p = std::make_shared<detail_::coroutine_ownership_impl_>(list_, reserved_list_, iter);
    std::get<0u>(*iter) = h;
    std::get<1u>(*iter) = p;
    return coroutine_ownership(exec_.value(), std::move(p));
  }

private:
  virtual void shutdown() noexcept override
  {
    CORIO_ASSERT(exec_.has_value());
    if (!list_.empty()) {
      iterator_type_ jter = list_.begin();
      iterator_type_ iter = jter++;
      for (; jter != list_.end(); iter = jter++) {
        auto &[h, p] = *iter;
        CORIO_ASSERT(h != nullptr);
        CORIO_ASSERT(!!p);
        h.destroy();
        h = nullptr;
        p.reset();
      }
    }
    CORIO_ASSERT(list_.empty());
    reserved_list_.clear();
  }

private:
  std::optional<executor_type> exec_;
  list_type_ list_;
  list_type_ reserved_list_;
}; // class coroutine_cleanup_service

} // namespace corio::thread_unsafe

#endif // !defined(CORIO_THREAD_UNSAFE_COROUTINE_CLEANUP_SERVICE_HPP_INCLUDE_GUARD)
