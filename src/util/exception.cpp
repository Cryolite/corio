#include <corio/util/exception.hpp>
#include <corio/util/scope_guard.hpp>
#include <boost/stacktrace/stacktrace.hpp>
#include <boost/exception/get_error_info.hpp>
#include <boost/exception/exception.hpp>
#include <iostream>
#include <string>
#include <exception>
#include <typeinfo>
#include <cstdlib>
#include <cstddef>


namespace corio::detail_{

namespace{

std::string demangle_(char const *mangled_name)
{
  int status = 0;
  char *p = abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status);
  if (status != 0) {
    throw std::runtime_error("Failed to call `abi::__cxa_demangle'.");
  }
  CORIO_SCOPE_GUARD{
    std::free(p);
  };
  return std::string(p);
}

[[noreturn]] void terminate_handler_() noexcept
try{
  std::exception_ptr p = std::current_exception();
  if (p == nullptr) {
    std::abort();
  }

  std::cerr << "Program terminates due to an uncaught exception.";

  std::string indent;
  while (p != nullptr) {
    char const * const *p_filename = nullptr;
    int const *p_line_number = nullptr;
    char const * const *p_function = nullptr;
    boost::stacktrace::stacktrace const *p_stacktrace = nullptr;
    try {
      std::rethrow_exception(p);
    }
    catch (boost::exception const &e) {
      p_filename = boost::get_error_info<boost::throw_file>(e);
      p_line_number = boost::get_error_info<boost::throw_line>(e);
      p_function = boost::get_error_info<boost::throw_function>(e);
      p_stacktrace = boost::get_error_info<corio::stacktrace_info>(e);
    }
    catch (...) {
    }

    std::type_info const *p_type_info = nullptr;
    char const *what = nullptr;
    std::exception_ptr nested_exception = nullptr;
    try {
      std::rethrow_exception(p);
    }
    catch (std::exception const &e) {
      p_type_info = &typeid(e);
      what = e.what();
      try {
        std::rethrow_if_nested(e);
      }
      catch (...) {
        nested_exception = std::current_exception();
      }
    }
    catch (...) {
    }

    bool has_prefix = false;

    if (p_filename != nullptr && *p_filename != nullptr) {
      std::cerr << '\n' << indent << *p_filename;
      has_prefix = true;
    }

    if (p_line_number != nullptr) {
      if (has_prefix) {
        std::cerr << ':';
      }
      else {
        std::cerr << '\n' << indent;
      }
      std::cerr << *p_line_number;
      has_prefix = true;
    }

    if (p_function != nullptr && *p_function != nullptr) {
      if (has_prefix) {
        std::cerr << ": ";
      }
      else {
        std::cerr << '\n' << indent;
      }
      std::cerr << *p_function;
      has_prefix = true;
    }

    if (p_type_info != nullptr) {
      if (has_prefix) {
        std::cerr << ": ";
      }
      else {
        std::cerr << '\n' << indent;
      }
      std::cerr << demangle_(p_type_info->name());
      has_prefix = true;
    }

    if (what != nullptr) {
      if (has_prefix) {
        std::cerr << ": ";
      }
      else {
        std::cerr << '\n' << indent;
      }
      std::cerr << what;
    }

    if (p_stacktrace != nullptr) {
      if (p_stacktrace->size() > 0u) {
        std::cerr << '\n' << indent << "Stack trace:";
        for (auto const &frame : *p_stacktrace) {
          std::cerr << '\n' << indent << "  " << frame;
        }
      }
    }

    if (nested_exception != nullptr) {
      std::cerr << '\n' << indent << "Nested exception:";
      indent += "  ";
      p = nested_exception;
    }
    else {
      p = nullptr;
    }
  }

  std::abort();
}
catch (...) {
  std::abort();
}

} // namespace *unnamed*

terminate_handler_setter_t_::terminate_handler_setter_t_() noexcept
{
  std::set_terminate(&terminate_handler_);
}

} // namespace corio::detail_
