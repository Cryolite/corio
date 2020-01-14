#if !defined(CORIO_UTIL_EXCEPTION_HPP_INCLUDE_GUARD)
#define CORIO_UTIL_EXCEPTION_HPP_INCLUDE_GUARD

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-inline"
#include <boost/stacktrace/stacktrace.hpp>
#pragma clang diagnostic pop
#include <boost/stacktrace/frame.hpp>
#include <boost/exception/info.hpp>


namespace corio{

struct stacktrace_error_info_t
{};

using stacktrace_info = boost::error_info<stacktrace_error_info_t, boost::stacktrace::stacktrace>;

namespace detail_{

class terminate_handler_setter_t_
{
public:
  terminate_handler_setter_t_() noexcept;
}; // class terminate_handler_setter_

inline terminate_handler_setter_t_ terminate_handler_setter_;

} // namespace detail_

} // namespace corio

#endif // !defined(CORIO_UTIL_EXCEPTION_HPP_INCLUDE_GUARD)
