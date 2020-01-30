#if !defined(CORIO_THREAD_UNSAFE_COROUTINE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_COROUTINE_HPP_INCLUDE_GUARD

#include <corio/core/is_executor.hpp>
#include <corio/core/error.hpp>
#include <corio/util/throw.hpp>
#include <boost/asio/executor.hpp>
#include <boost/config.hpp>
#include <utility>


namespace corio::thread_unsafe{

template<typename R, typename Executor>
class coroutine_promise;

template<typename R, typename Executor>
class basic_coroutine
{
public:
  using executor_type = Executor;
  static_assert(corio::is_executor_v<executor_type>);
  using promise_type = coroutine_promise<R, executor_type>;

private:
  friend class corio::thread_unsafe::coroutine_promise<R, executor_type>;

  explicit basic_coroutine(promise_type &promise) noexcept
    : p_(&promise)
  {
    p_->acquire();
  }

public:
  basic_coroutine() noexcept
    : p_()
  {}

  basic_coroutine(basic_coroutine const &) = delete;

  basic_coroutine(basic_coroutine &&rhs) noexcept
    : p_(rhs.p_)
  {
    rhs.p_ = nullptr;
  }

  ~basic_coroutine()
  {
    if (p_ != nullptr && p_->release() == 0u) {
      if (done()) {
        p_->destroy();
      }
      else {
        p_->transfer_ownership();
      }
    }
  }

  void swap(basic_coroutine &rhs) noexcept
  {
    using std::swap;
    swap(p_, rhs.p_);
  }

  friend void swap(basic_coroutine &lhs, basic_coroutine &rhs) noexcept
  {
    lhs.swap(rhs);
  }

  basic_coroutine &operator=(basic_coroutine const &) = delete;

  basic_coroutine &operator=(basic_coroutine &&rhs) noexcept
  {
    basic_coroutine(std::move(rhs)).swap(*this);
    return *this;
  }

  executor_type get_executor() const
  {
    if (BOOST_UNLIKELY(p_ == nullptr)) /*[[unlikely]]*/ {
      CORIO_THROW<corio::invalid_coroutine_error>();
    }
    return p_->get_executor();
  }

  bool valid() const noexcept
  {
    return p_ != nullptr;
  }

  void resume()
  {
    if (BOOST_UNLIKELY(p_ == nullptr)) /*[[unlikely]]*/ {
      CORIO_THROW<corio::invalid_coroutine_error>();
    }
    if (p_->done()) {
      CORIO_THROW<corio::coroutine_already_done_error>();
    }
    p_->resume();
  }

  bool done() const
  {
    if (BOOST_UNLIKELY(p_ == nullptr)) /*[[unlikely]]*/ {
      CORIO_THROW<corio::invalid_coroutine_error>();
    }
    return p_->done();
  }

private:
  promise_type *p_;
}; // class basic_coroutine

template<typename R>
using coroutine = basic_coroutine<R, boost::asio::executor>;

} // namespace corio::thread_unsafe

#include <corio/thread_unsafe/coroutine_promise.hpp>

#endif // !defined(CORIO_THREAD_UNSAFE_COROUTINE_HPP_INCLUDE_GUARD)
