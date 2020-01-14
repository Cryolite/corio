#if !defined(CORIO_UTIL_ASSERT_HPP_INCLUDE_GUARD)
#define CORIO_UTIL_ASSERT_HPP_INCLUDE_GUARD

#if defined(CORIO_ENABLE_ASSERT)

#include <boost/stacktrace/stacktrace.hpp>
#include <boost/stacktrace/frame.hpp>
#include <sstream>
#include <iostream>
#include <ios>
#include <type_traits>
#include <utility>
#include <cstdlib>


namespace corio::detail_{

template<typename Assertion>
class ostreamable_assertion_
{
private:
  static_assert(!std::is_const_v<Assertion>);
  static_assert(!std::is_volatile_v<Assertion>);
  static_assert(!std::is_reference_v<Assertion>);

public:
  explicit ostreamable_assertion_(Assertion &&assertion) noexcept
    : assertion_(std::move(assertion)),
      ss_()
  {}

  ostreamable_assertion_(ostreamable_assertion_ const &) = delete;

  ostreamable_assertion_(ostreamable_assertion_ &&) = default;

  ostreamable_assertion_ &operator=(ostreamable_assertion_ const &) = delete;

  template<typename T>
  ostreamable_assertion_ &&operator<<(T const &value) &&
  {
    ss_ << value;
    return std::move(*this);
  }

  ~ostreamable_assertion_() noexcept(false)
  {
    if (!assertion_.condition()) {
      std::cerr << assertion_.file_name() << ':' << assertion_.line_number()
                << ": " << assertion_.function_name() << ": Assertion ";
      if constexpr (Assertion::has_expression()) {
        std::cerr << '`' << assertion_.expression() << "' ";
      }
      std::cerr << "failed.";
      std::string message = std::move(ss_).str();
      if (!message.empty()) {
        std::cerr << "\nMessage: " << message;
      }
      if (assertion_.stacktrace().size() > 0u) {
        std::cerr << "\nStack trace:";
        for (auto const &frame : assertion_.stacktrace()) {
          std::cerr << "\n  " << frame;
        }
      }
      std::abort();
    }
  }

private:
  Assertion assertion_;
  std::stringstream ss_;
}; // class ostreamable_assertion_

template<typename Assertion>
ostreamable_assertion_(Assertion &&) -> ostreamable_assertion_<Assertion>;

class expression_assertion_
{
private:
  using ostreamable_type_ = ostreamable_assertion_<expression_assertion_>;

public:
  expression_assertion_(
    bool condition, char const *file_name, int line_number, char const *function_name,
    char const *expression, boost::stacktrace::stacktrace &&stacktrace) noexcept
    : condition_(condition),
      file_name_(file_name),
      line_number_(line_number),
      function_name_(function_name),
      expression_(expression),
      stacktrace_(std::move(stacktrace))
  {}

  expression_assertion_(expression_assertion_ const &) = delete;

  expression_assertion_(expression_assertion_ &&rhs) noexcept
    : condition_(rhs.condition_),
      file_name_(rhs.file_name_),
      line_number_(rhs.line_number_),
      function_name_(rhs.function_name_),
      expression_(rhs.expression_),
      stacktrace_(std::move(rhs.stacktrace_))
  {
    rhs.condition_ = true;
  }

  expression_assertion_ &operator=(expression_assertion_ const &) = delete;

  bool condition() const noexcept
  {
    return condition_;
  }

  char const *file_name() const noexcept
  {
    return file_name_;
  }

  int line_number() const noexcept
  {
    return line_number_;
  }

  char const *function_name() const noexcept
  {
    return function_name_;
  }

  static constexpr bool has_expression()
  {
    return true;
  }

  char const *expression() const noexcept
  {
    return expression_;
  }

  boost::stacktrace::stacktrace const &stacktrace() const noexcept
  {
    return stacktrace_;
  }

  template<typename T>
  ostreamable_type_ operator<<(T const &value) &&
  {
    return ostreamable_type_(std::move(*this)) << value;
  }

  ostreamable_type_ operator<<(std::ostream &(*pf)(std::ostream &)) &&
  {
    return ostreamable_type_(std::move(*this)) << pf;
  }

  ostreamable_type_ operator<<(std::ios &(*pf)(std::ios &)) &&
  {
    return ostreamable_type_(std::move(*this)) << pf;
  }

  ostreamable_type_ operator<<(std::ios_base &(*pf)(std::ios_base &)) &&
  {
    return ostreamable_type_(std::move(*this)) << pf;
  }

  ~expression_assertion_() noexcept(false)
  {
    if (!condition_) {
      std::cerr << file_name_ << ':' << line_number_ << ": " << function_name_ << ": "
                << "Assertion `" << expression_ << "' failed.";
      if (stacktrace_.size() > 0u) {
        std::cerr << "\nStack trace:";
        for (auto const &frame : stacktrace_) {
          std::cerr << "\n  " << frame;
        }
      }
      std::abort();
    }
  }

private:
  bool condition_;
  char const *file_name_;
  int line_number_;
  char const *function_name_;
  char const *expression_;
  boost::stacktrace::stacktrace stacktrace_;
}; // class expression_assertion_

class block_assertion_
{
private:
  using ostreamable_type_ = ostreamable_assertion_<block_assertion_>;

public:
  block_assertion_(char const *file_name, int line_number, char const *function_name,
                   boost::stacktrace::stacktrace &&stacktrace)
    : condition_(true),
      file_name_(file_name),
      line_number_(line_number),
      function_name_(function_name),
      stacktrace_(std::move(stacktrace))
  {}

  block_assertion_(block_assertion_ const &) = delete;

  block_assertion_(block_assertion_ &&rhs) noexcept
    : condition_(rhs.condition_),
      file_name_(rhs.file_name_),
      line_number_(rhs.line_number_),
      function_name_(rhs.function_name_),
      stacktrace_(std::move(rhs.stacktrace_))
  {
    rhs.condition_ = true;
  }

  template<typename F>
  block_assertion_ &&operator->*(F &&f) &&
  {
    condition_ = f();
    return std::move(*this);
  }

  bool condition() const noexcept
  {
    return condition_;
  }

  char const *file_name() const noexcept
  {
    return file_name_;
  }

  int line_number() const noexcept
  {
    return line_number_;
  }

  char const *function_name() const noexcept
  {
    return function_name_;
  }

  static constexpr bool has_expression()
  {
    return false;
  }

  boost::stacktrace::stacktrace const &stacktrace() const noexcept
  {
    return stacktrace_;
  }

  template<typename T>
  ostreamable_type_ operator<<(T const &value) &&
  {
    return ostreamable_type_(std::move(*this)) << value;
  }

  ostreamable_type_ operator<<(std::ostream &(*pf)(std::ostream &))
  {
    return ostreamable_type_(std::move(*this)) << pf;
  }

  ostreamable_type_ operator<<(std::ios &(*pf)(std::ios &))
  {
    return ostreamable_type_(std::move(*this)) << pf;
  }

  ostreamable_type_ operator<<(std::ios_base &(*pf)(std::ios_base &))
  {
    return ostreamable_type_(std::move(*this)) << pf;
  }

  ~block_assertion_() noexcept(false)
  {
    if (!condition_) {
      std::cerr << file_name_ << ':' << line_number_ << ": " << function_name_ << ": " << "Assertion failed.";
      if (stacktrace_.size() > 0u) {
        std::cerr << "\nStack trace:";
        for (auto const &frame : stacktrace_) {
          std::cerr << "\n  " << frame;
        }
      }
      std::abort();
    }
  }

private:
  bool condition_;
  char const *file_name_;
  int line_number_;
  char const *function_name_;
  boost::stacktrace::stacktrace stacktrace_;
}; // class block_assertion_

} // namespace corio::detail_

#define CORIO_ASSERT_EXPRESSION_(...)                                                         \
::corio::detail_::expression_assertion_(                                                      \
  (__VA_ARGS__), __FILE__, __LINE__, __func__, #__VA_ARGS__, boost::stacktrace::stacktrace()) \
  /**/

#define CORIO_ASSERT_()                                                            \
::corio::detail_::block_assertion_(                                                \
  __FILE__, __LINE__, __func__, boost::stacktrace::stacktrace()) ->* [&]() -> bool \
  /**/

#define CORIO_ASSERT(...) CORIO_ASSERT_ ## __VA_OPT__(EXPRESSION_) (__VA_ARGS__)



#else // defined(CORIO_ENABLE_ASSERT)



#include <corio/param/expression.hpp>
#include <corio/param/function.hpp>
#include <corio/param/line.hpp>
#include <corio/param/file.hpp>
#include <iostream>


namespace corio::detail_{

class dummy_expression_assertion_
{
public:
  constexpr dummy_expression_assertion_() noexcept
  {}

  dummy_expression_assertion_(dummy_expression_assertion_ const &) = delete;

  dummy_expression_assertion_ &operator=(dummy_expression_assertion_ const &) = delete;

  dummy_expression_assertion_ const &operator<<(corio::param::file_t) const noexcept
  {
    return *this;
  }

  dummy_expression_assertion_ const &operator<<(corio::param::line_t) const noexcept
  {
    return *this;
  }

  dummy_expression_assertion_ const &operator<<(corio::param::function_t) const noexcept
  {
    return *this;
  }

  dummy_expression_assertion_ const &operator<<(corio::param::expression_t) const noexcept
  {
    return *this;
  }

  template<typename T>
  dummy_expression_assertion_ const &operator<<(T const &) const noexcept
  {
    return *this;
  }

  dummy_expression_assertion_ const &operator<<(std::ostream &(*)(std::ostream &)) const noexcept
  {
    return *this;
  }

  dummy_expression_assertion_ const &operator<<(std::ios &(*)(std::ios &)) const noexcept
  {
    return *this;
  }

  dummy_expression_assertion_ const &operator<<(std::ios_base &(*)(std::ios_base &)) const noexcept
  {
    return *this;
  }
}; // class dummy_expression_assertion_

class dummy_block_assertion_
{
public:
  constexpr dummy_block_assertion_() noexcept
  {}

  dummy_block_assertion_(dummy_block_assertion_ const &) = delete;

  dummy_block_assertion_ &operator=(dummy_block_assertion_ const &) = delete;

  template<typename F>
  dummy_block_assertion_ const &operator->*(F &&) const noexcept
  {
    return *this;
  }

  dummy_block_assertion_ const &operator<<(corio::param::file_t) const noexcept
  {
    return *this;
  }

  dummy_block_assertion_ const &operator<<(corio::param::line_t) const noexcept
  {
    return *this;
  }

  dummy_block_assertion_ const &operator<<(corio::param::function_t) const noexcept
  {
    return *this;
  }

  dummy_block_assertion_ const &operator<<(corio::param::expression_t) const noexcept
  {
    return *this;
  }

  template<typename T>
  dummy_block_assertion_ const &operator<<(T const &) const noexcept
  {
    return *this;
  }

  dummy_block_assertion_ const &operator<<(std::ostream &(*)(std::ostream &)) const noexcept
  {
    return *this;
  }

  dummy_block_assertion_ const &operator<<(std::ios &(*)(std::ios &)) const noexcept
  {
    return *this;
  }

  dummy_block_assertion_ const &operator<<(std::ios_base &(*)(std::ios_base &)) const noexcept
  {
    return *this;
  }
}; // class dummy_block_assertion_

} // namespace corio::detail_

#define CORIO_ASSERT_EXPRESSION_(...)           \
::corio::detail_::dummy_expression_assertion_{} \
  /**/

#define CORIO_ASSERT_()                                    \
::corio::detail_::dummy_block_assertion_{} ->* [&] -> void \
  /**/

#define CORIO_ASSERT(...) CORIO_ASSERT_ ## __VA_OPT__(EXPRESSION_) (__VA_ARGS__)

#endif // defined(CORIO_ENABLE_ASSERT)

#endif // !defined(CORIO_UTIL_ASSERT_HPP_INCLUDE_GUARD)
