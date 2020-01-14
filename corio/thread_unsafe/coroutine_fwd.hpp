#if !defined(CORIO_THREAD_UNSAFE_COROUTINE_FWD_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_COROUTINE_FWD_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/detail_/shared_coroutine_state_.hpp>
#include <experimental/coroutine>


namespace corio::thread_unsafe{

template<typename R>
class promise;

namespace detail_{

template<typename R>
class coroutine_base_
{
public:
  using promise_type = promise<R>;

protected:
  using handle_type_ = std::experimental::coroutine_handle<promise_type>;
  using state_type_ = corio::thread_unsafe::detail_::shared_coroutine_state_<R>;

  coroutine_base_(handle_type_ &&handle, state_type_ *p) noexcept;

  coroutine_base_() noexcept;

  coroutine_base_(coroutine_base_ const &) = delete;

  coroutine_base_(coroutine_base_ &&rhs) noexcept;

  ~coroutine_base_();

  void swap(coroutine_base_ &rhs) noexcept;

  coroutine_base_ &operator=(coroutine_base_ const &) = delete;

  coroutine_base_ &operator=(coroutine_base_ &&rhs) noexcept;

  void operator()();

  bool done() const;

protected:
  handle_type_ handle_;
  state_type_ *p_;
}; // class coroutine_base_

} // namespace detail_

template<typename R>
class coroutine
  : protected detail_::coroutine_base_<R>
{
private:
  using base_type_ = detail_::coroutine_base_<R>;

public:
  using typename base_type_::promise_type;

private:
  using typename base_type_::handle_type_;
  using typename base_type_::state_type_;

  friend class promise<R>;

  coroutine(handle_type_ &&handle, state_type_ *p) noexcept;

public:
  friend void swap(coroutine &lhs, coroutine &rhs) noexcept
  {
    lhs.swap(rhs);
  }

  using base_type_::operator();

  using base_type_::done;
}; // class coroutine

template<typename R>
class coroutine<R &>
  : protected detail_::coroutine_base_<R &>
{
private:
  using base_type_ = detail_::coroutine_base_<R &>;

public:
  using typename base_type_::promise_type;

private:
  using typename base_type_::handle_type_;
  using typename base_type_::state_type_;

  friend class promise<R &>;

  coroutine(handle_type_ &&handle, state_type_ *p) noexcept;

public:
  friend void swap(coroutine &lhs, coroutine &rhs) noexcept
  {
    lhs.swap(rhs);
  }

  using base_type_::operator();

  using base_type_::done;
}; // class coroutine<R &>

template<>
class coroutine<void>
  : protected detail_::coroutine_base_<void>
{
private:
  using base_type_ = detail_::coroutine_base_<void>;

public:
  using typename base_type_::promise_type;

private:
  using typename base_type_::handle_type_;
  using typename base_type_::state_type_;

  friend class promise<void>;

  coroutine(handle_type_ &&handle, state_type_ *p) noexcept;

public:
  friend void swap(coroutine &lhs, coroutine &rhs) noexcept
  {
    lhs.swap(rhs);
  }

  using base_type_::operator();

  using base_type_::done;
}; // class coroutine<void>

} // namespace corio::thread_unsafe

#endif // !defined(CORIO_THREAD_UNSAFE_COROUTINE_FWD_HPP_INCLUDE_GUARD)
