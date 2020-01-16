#if !defined(CORIO_THREAD_UNSAFE_FUTURE_HPP_INCLUDE_GUARD)
#define CORIO_THREAD_UNSAFE_FUTURE_HPP_INCLUDE_GUARD

#include <corio/thread_unsafe/detail_/shared_future_state_.hpp>


namespace corio::thread_unsafe{

namespace detail_{

template<typename R, typename ExecutionContext>
class promise_base_;

} // namespace detail_

template<typename R, typename ExecutionContext>
class basic_future
{
public:
  using context_type = ExecutionContext;

private:
  using state_type_ = corio::thread_unsafe::detail_::shared_future_state_<R, context_type>;

  friend class corio::thread_unsafe::detail_::promise_base_<R, context_type>;

  explicit basic_future(state_type_ const &state)
  {}
};

} // namespace corio::thread_unsafe

#endif // !defined(CORIO_THREAD_UNSAFE_FUTURE_HPP_INCLUDE_GUARD)
