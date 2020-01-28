#if !defined(CORIO_THREAD_UNSAFE_COROUTINE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_COROUTINE_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/future.hpp>
#include <corio/thread_unsafe/coroutine_cleanup_service.hpp>
#include <corio/core/is_executor.hpp>
#include <corio/core/error.hpp>
#include <corio/util/throw.hpp>
#include <corio/util/assert.hpp>
#include <boost/asio/executor.hpp>
#include <boost/config.hpp>
#include <list>
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
  using future_type = corio::thread_unsafe::basic_future<R, executor_type>;

private:
  using handle_type_ = std::experimental::coroutine_handle<promise_type>;
  using cleanup_canceller_type_ = std::list<corio::thread_unsafe::coroutine_cleanup_canceller>;

  friend class corio::thread_unsafe::coroutine_promise<R, executor_type>;

  explicit basic_coroutine(promise_type &promise) noexcept
    : p_(&promise),
      cleanup_canceller_()
  {
    p_->acquire();
    promise.reserve_cleanup_canceller(cleanup_canceller_);
  }

public:
  basic_coroutine() noexcept
    : p_(),
      cleanup_canceller_()
  {}

  basic_coroutine(basic_coroutine const &) = delete;

  basic_coroutine(basic_coroutine &&rhs) noexcept
    : p_(rhs.p_),
      cleanup_canceller_(std::move(rhs.cleanup_canceller_))
  {
    rhs.p_ = nullptr;
  }

  ~basic_coroutine()
  {
    CORIO_ASSERT((p_ != nullptr) == (cleanup_canceller_.size() == 1u));
    CORIO_ASSERT(cleanup_canceller_.size() == 0u || cleanup_canceller_.size() == 1u);
    if (p_ != nullptr && p_->release() == 0u) {
      if (done() || (cleanup_canceller_.size() == 0u && !p_->registered_for_cleanup())) {
        if (p_->registered_for_cleanup()) {
          p_->cancel_cleanup();
        }
        handle_type_ h = p_->get_handle();
        h.destroy();
      }
      else if (cleanup_canceller_.size() == 1u) {
        p_->register_for_cleanup(cleanup_canceller_);
      }
    }
  }

  void swap(basic_coroutine &rhs) noexcept
  {
    using std::swap;
    swap(p_, rhs.p_);
    swap(cleanup_canceller_, rhs.cleanup_canceller);
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
  cleanup_canceller_type_ cleanup_canceller_;
}; // class basic_coroutine

template<typename R>
using coroutine = basic_coroutine<R, boost::asio::executor>;

} // namespace corio::thread_unsafe

#include <corio/thread_unsafe/coroutine_promise.hpp>

#endif // !defined(CORIO_THREAD_UNSAFE_COROUTINE_HPP_INCLUDE_GUARD)
