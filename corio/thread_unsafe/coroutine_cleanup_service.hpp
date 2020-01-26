#if !defined(CORIO_THREAD_UNSAFE_COROUTINE_CLEANUP_SERVICE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_COROUTINE_CLEANUP_SERVICE_HPP_INCLUDE_GUARD

#include <corio/util/throw.hpp>
#include <corio/util/assert.hpp>
#include <boost/asio/execution_context.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <list>
#include <utility>
#include <stdexcept>
#include <experimental/coroutine>
#include <cstddef>


namespace corio::thread_unsafe{

namespace detail_{

class coroutine_cleanup_canceller_impl_;

} // namespace detail_

class coroutine_cleanup_canceller
{
private:
  using impl_type_ = detail_::coroutine_cleanup_canceller_impl_;

public:
  using handle_type = std::experimental::coroutine_handle<>;

  coroutine_cleanup_canceller();

  coroutine_cleanup_canceller(coroutine_cleanup_canceller const &rhs) noexcept;

  coroutine_cleanup_canceller(coroutine_cleanup_canceller &&rhs) noexcept
    : p_(rhs.p_)
  {
    rhs.p_ = nullptr;
  }

  ~coroutine_cleanup_canceller();

  void swap(coroutine_cleanup_canceller &rhs) noexcept
  {
    using std::swap;
    swap(p_, rhs.p_);
  }

  friend void swap(coroutine_cleanup_canceller &lhs, coroutine_cleanup_canceller &rhs) noexcept
  {
    lhs.swap(rhs);
  }

  coroutine_cleanup_canceller &operator=(coroutine_cleanup_canceller const &rhs)
  {
    coroutine_cleanup_canceller(rhs).swap(*this);
    return *this;
  }

  coroutine_cleanup_canceller &operator=(coroutine_cleanup_canceller &&rhs) noexcept
  {
    coroutine_cleanup_canceller(std::move(rhs)).swap(*this);
    return *this;
  }

  bool valid() const noexcept;

  handle_type get_handle() const noexcept;

  void execute() noexcept;

private:
  friend class coroutine_cleanup_service;
  using list_type = std::list<coroutine_cleanup_canceller>;
  using iterator_type = list_type::iterator;

  void assign(handle_type const &handle, list_type &list,
              list_type &reserved_list, iterator_type const &iterator);

private:
  impl_type_ *p_ = nullptr;
}; // class coroutine_cleanup_canceller

class coroutine_cleanup_service
  : public boost::asio::execution_context::service
{
public:
  using context_type = boost::asio::execution_context;
  using handle_type = std::experimental::coroutine_handle<>;
  using list_type = std::list<coroutine_cleanup_canceller>;

private:
  using iterator_type_ = list_type::iterator;

public:
  inline static boost::asio::execution_context::id id{};

  explicit coroutine_cleanup_service(context_type &ctx) noexcept
    : boost::asio::execution_context::service(ctx),
      list_(),
      reserved_list_()
  {}

  coroutine_cleanup_service(coroutine_cleanup_service const &) = delete;

  coroutine_cleanup_service &operator=(coroutine_cleanup_service const &) = delete;

  void reserve(list_type &list)
  {
    if (BOOST_UNLIKELY(!list.empty())) /*[[unlikely]]*/ {
      CORIO_THROW<std::invalid_argument>(
        "A non-empty list is passed to `coroutine_cleanup_service::reserve'.");
    }
    if (!reserved_list_.empty()) {
      list.splice(list.cbegin(), reserved_list_, reserved_list_.cbegin());
    }
    else {
      list.emplace_front();
    }
    CORIO_ASSERT(!list.front().valid());
  }

  coroutine_cleanup_canceller register_for_cleanup(handle_type const &handle, list_type &list) noexcept
  {
    CORIO_ASSERT(list.size() == 1u);
    list_.splice(list_.cbegin(), list);
    iterator_type_ iter = list_.begin();
    iter->assign(handle, list_, reserved_list_, iter);
    return *iter;
  }

private:
  virtual void shutdown() noexcept override
  {
    if (!list_.empty()) {
      iterator_type_ iter = list_.begin();
      iterator_type_ jter = iter;
      ++jter;
      for (;;) {
        CORIO_ASSERT(iter->valid());
        // The following line (the copy constructor of the type
        // `std::list<coroutine_cleanup_canceller>`) is assumed not to throw an
        // exception although the C++2a specification does not explicitly
        // guarantee so.
        handle_type h = iter->get_handle();
        CORIO_ASSERT(h != nullptr);
        // The following line (`std::list<coroutine_cleanup_canceller>::destroy`)
        // is assumed not to throw an exception although the C++2a
        // specification does not explicitly guarantee so.
        h.destroy();
        iter->execute();
        if (jter == list_.cend()) {
          break;
        }
        iter = jter++;
      }
      CORIO_ASSERT(list_.empty());
    }
    CORIO_ASSERT(
      std::all_of(reserved_list_.cbegin(), reserved_list_.cend(),
                  [](auto const &canceller) -> bool { return !canceller.valid(); }));
    reserved_list_.clear();
  }

private:
  list_type list_;
  list_type reserved_list_;
}; // class coroutine_cleanup_service

class coroutine_cleanup_service;

namespace detail_{

class coroutine_cleanup_canceller_impl_
{
public:
  using handle_type = std::experimental::coroutine_handle<>;
  using list_type = std::list<coroutine_cleanup_canceller>;
  using iterator_type = list_type::iterator;

  coroutine_cleanup_canceller_impl_() noexcept
    : handle_(),
      p_list_(),
      p_reserved_list_(),
      // The following line (the default constructor of the type
      // `std::list<coroutine_cleanup_canceller>`) is assumed not to throw an
      // exception although the C++2a specification does not explicitly
      // guarantee so.
      iterator_(),
      refcount_()
  {}

  coroutine_cleanup_canceller_impl_(coroutine_cleanup_canceller_impl_ const &) = delete;

  coroutine_cleanup_canceller_impl_ &operator=(coroutine_cleanup_canceller_impl_ const &) = delete;

  void acquire()
  {
    ++refcount_;
  }

  std::size_t release()
  {
    CORIO_ASSERT(refcount_ > 0u);
    return --refcount_;
  }

  bool valid() const noexcept
  {
    CORIO_ASSERT((p_list_ != nullptr) == (p_reserved_list_ != nullptr));
    CORIO_ASSERT(refcount_ > 0u);
    return p_list_ != nullptr;
  }

  handle_type get_handle() const
  {
    CORIO_ASSERT(valid());
    // The following line (the copy constructor of the type
    // `std::list<coroutine_cleanup_canceller>`) is assumed not to throw an
    // exception although the C++2a specification does not explicitly guarantee
    // so.
    return handle_;
  }

  void execute() noexcept
  {
    handle_ = nullptr;
    // The following line (`std::list<coroutine_cleanup_canceller>::splice`)
    // is assumed not to throw an exception although the C++2a specification
    // does not explicitly guarantee so.
    p_reserved_list_->splice(p_reserved_list_->cbegin(), *p_list_, iterator_);
    p_list_ = nullptr;
    p_reserved_list_ = nullptr;
    // The following line (the default and copy constructors of the type
    // `std::list<coroutine_cleanup_canceller>`) is assumed not to throw an
    // exception although the C++2a specification does not explicitly guarantee
    // so.
    iterator_ = iterator_type();
  }

  void assign(handle_type const &handle, list_type &list,
              list_type &reserved_list, iterator_type const &iterator)
  {
    CORIO_ASSERT(handle_ == nullptr);
    CORIO_ASSERT(p_list_ == nullptr);
    CORIO_ASSERT(p_reserved_list_ == nullptr);
    CORIO_ASSERT(refcount_ > 0u);
    handle_ = handle;
    p_list_ = &list;
    p_reserved_list_ = &reserved_list;
    iterator_ = iterator;
  }

private:
  handle_type handle_;
  list_type *p_list_;
  list_type *p_reserved_list_;
  iterator_type iterator_;
  std::size_t refcount_;
}; // class coroutine_cleanup_canceller_impl_

} // namespace detail_

inline coroutine_cleanup_canceller::coroutine_cleanup_canceller()
  : p_(new impl_type_())
{
  p_->acquire();
}

inline coroutine_cleanup_canceller::coroutine_cleanup_canceller(
  coroutine_cleanup_canceller const &rhs) noexcept
  : p_(rhs.p_)
{
  if (p_ != nullptr) {
    p_->acquire();
  }
}

inline coroutine_cleanup_canceller::~coroutine_cleanup_canceller()
{
  if (p_ != nullptr && p_->release() == 0u) {
    delete p_;
  }
}

inline bool coroutine_cleanup_canceller::valid() const noexcept
{
  return p_ != nullptr && p_->valid();
}

inline auto coroutine_cleanup_canceller::get_handle() const noexcept -> handle_type
{
  CORIO_ASSERT(valid());
  // The following line (the copy constructor of the type
  // `std::list<coroutine_cleanup_canceller>`) is assumed not to throw an
  // exception although the C++2a specification does not explicitly guarantee
  // so.
  return p_->get_handle();
}

inline void coroutine_cleanup_canceller::execute() noexcept
{
  CORIO_ASSERT(valid());
  p_->execute();
}

inline void coroutine_cleanup_canceller::assign(
  handle_type const &handle, list_type &list, list_type &reserved_list, iterator_type const &iterator)
{
  CORIO_ASSERT(!valid());
  if (p_ == nullptr) {
    p_ = new impl_type_();
    p_->acquire();
  }
  p_->assign(handle, list, reserved_list, iterator);
}

} // namespace corio::thread_unsafe

#endif // !defined(CORIO_THREAD_UNSAFE_COROUTINE_CLEANUP_SERVICE_HPP_INCLUDE_GUARD)
