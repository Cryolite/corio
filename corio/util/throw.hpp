#if !defined(CORIO_UTIL_THROW_HPP_INCLUDE_GUARD)
#define CORIO_UTIL_THROW_HPP_INCLUDE_GUARD

#include <corio/util/index_pack.hpp>
#include <corio/util/exception.hpp>
#include <boost/stacktrace/stacktrace.hpp>
#include <boost/throw_exception.hpp>
#include <boost/exception/enable_error_info.hpp>
#include <boost/exception/exception.hpp>
#include <sstream>
#include <ostream>
#include <ios>
#include <string>
#include <type_traits>
#include <tuple>
#include <utility>
#include <stdexcept>
#include <exception>
#include <cstddef>


namespace corio::detail_{

template<typename E, typename... Args>
[[noreturn]] std::enable_if<std::is_constructible_v<E, Args...>, void>
throw_impl_(char const *filename, int line_number, char const *function_name,
            boost::stacktrace::stacktrace &&stacktrace, Args &&... args)
{
  if (std::current_exception() != nullptr) {
    std::throw_with_nested(
      boost::enable_error_info(E(std::forward<Args>(args)...))
      << boost::throw_file(filename) << boost::throw_line(line_number)
      << boost::throw_function(function_name) << corio::stacktrace_info(std::move(stacktrace)));
  }

  throw boost::enable_error_info(E(std::forward<Args>(args)...))
    << boost::throw_file(filename) << boost::throw_line(line_number)
    << boost::throw_function(function_name) << corio::stacktrace_info(std::move(stacktrace));
}

template<typename E, typename... Args>
constexpr std::size_t throwable_with_message_impl_(std::tuple<Args...> *, std::tuple<> *)
{
  static_assert((std::is_reference_v<Args> && ...));
  return std::is_constructible_v<E, Args..., std::string &&> ? 1u : 0u;
}

template<typename E, typename... Args, typename B, typename... Brgs>
constexpr std::size_t throwable_with_message_impl_(std::tuple<Args...> *, std::tuple<B, Brgs...> *)
{
  static_assert((std::is_reference_v<Args> && ...));
  static_assert(std::is_reference_v<B>);
  static_assert((std::is_reference_v<Brgs> && ...));
  using next_head_type = std::tuple<Args..., B>;
  using next_tail_type = std::tuple<Brgs...>;
  return (throwable_with_message_impl_<E>(static_cast<next_head_type *>(nullptr),
                                          static_cast<next_tail_type *>(nullptr))
          + std::is_constructible_v<E, Args..., std::string &&, B, Brgs...>) ? 1u : 0u;
}

template<typename E, typename... Args>
constexpr bool throwable_with_message_(std::tuple<Args...> *)
{
  static_assert((std::is_reference_v<Args> && ...));
  using head_type = std::tuple<>;
  using tail_type = std::tuple<Args...>;
  return throwable_with_message_impl_<E>(
    static_cast<head_type *>(nullptr), static_cast<tail_type *>(nullptr)) == 1u;
}

template<typename E, typename... Args>
[[noreturn]] void throw_with_message_impl_(
  char const *filename, int line_number, char const *function_name, boost::stacktrace::stacktrace &&stacktrace,
  std::string &&message, std::tuple<Args...> &&args, std::tuple<> &&)
{
  static_assert((std::is_reference_v<Args> && ...));
  CORIO_INDEX_PACK(is, sizeof...(Args)){
    throw_impl_<E>(filename, line_number, function_name, std::move(stacktrace),
                   std::get<is>(std::move(args))..., std::move(message));
  };
  throw_impl_<std::logic_error>(__FILE__, __LINE__, __func__, boost::stacktrace::stacktrace(),
                                "Control flow should not reach here.");
}

template<typename E, typename... Args, typename B, typename... Brgs>
[[noreturn]] void throw_with_message_impl_(
  char const *filename, int line_number, char const *function_name, boost::stacktrace::stacktrace &&stacktrace,
  std::string &&message, std::tuple<Args...> &&head, std::tuple<B, Brgs...> &&tail)
{
  static_assert((std::is_reference_v<Args> && ...));
  static_assert(std::is_reference_v<B>);
  static_assert((std::is_reference_v<Brgs> && ...));
  if constexpr (std::is_constructible_v<E, Args..., std::string &&, B, Brgs...>) {
    CORIO_INDEX_PACK(is, sizeof...(Args)){
      CORIO_INDEX_PACK(js, sizeof...(Brgs) + 1){
        throw_impl_<E>(
          filename, line_number, function_name, std::move(stacktrace),
          std::get<is>(std::move(head))..., std::move(message), std::get<js>(std::move(tail))...);
      };
    };
  }
  else {
    CORIO_INDEX_PACK(is, sizeof...(Args)){
      using new_head_type = std::tuple<Args..., B>;
      new_head_type new_head(std::get<is>(std::move(head))..., std::get<0u>(std::move(tail)));
      CORIO_INDEX_PACK(js, sizeof...(Brgs)){
        using new_tail_type = std::tuple<Brgs...>;
        new_tail_type new_tail(std::get<js + 1>(std::move(tail))...);
        throw_with_message_impl_<E>(
          filename, line_number, function_name, std::move(stacktrace),
          std::move(message), std::move(new_head), std::move(new_tail));
      };
    };
  }
  throw_impl_<std::logic_error>(__FILE__, __LINE__, __func__, boost::stacktrace::stacktrace(),
                                "Control flow should not reach here.");
}

template<typename E, typename... Args>
[[noreturn]] std::enable_if_t<throwable_with_message_<E>(static_cast<std::tuple<Args...> *>(nullptr)), void>
throw_with_message_(
  char const *filename, int line_number, char const *function_name,
  boost::stacktrace::stacktrace &&stacktrace, std::string &&message, std::tuple<Args...> &&args)
{
  static_assert((std::is_reference_v<Args> && ...));
  throw_with_message_impl_<E>(filename, line_number, function_name, std::move(stacktrace),
                              std::move(message), std::tuple<>(), std::move(args));
}

class thrower_
{
public:
  thrower_(char const *filename, int line_number, char const *function_name,
           boost::stacktrace::stacktrace &&stacktrace) noexcept
    : filename_(filename),
      line_number_(line_number),
      function_name_(function_name),
      stacktrace_(std::move(stacktrace))
  {}

  thrower_(thrower_ const &) = delete;

  thrower_ &operator=(thrower_ const &) = delete;

  template<typename Arguments>
  [[noreturn]] void operator<<=(Arguments &&arguments) &&
  {
    static_assert(!std::is_const_v<Arguments>);
    static_assert(!std::is_volatile_v<Arguments>);
    static_assert(!std::is_reference_v<Arguments>);
    using exception_type = typename Arguments::exception_type;
    if constexpr (Arguments::has_message()) {
      throw_with_message_<exception_type>(
        filename_, line_number_, function_name_, std::move(stacktrace_),
        std::move(arguments).message(), std::move(arguments).arguments());
    }
    else {
      CORIO_INDEX_PACK(is, Arguments::size()){
        throw_impl_<exception_type>(filename_, line_number_, function_name_, std::move(stacktrace_),
                                    std::get<is>(std::move(arguments).arguments())...);
      };
    }
    throw_impl_<std::logic_error>(__FILE__, __LINE__, __func__, boost::stacktrace::stacktrace(),
                                  "Control flow should not reach here.");
  }

private:
  char const *filename_;
  int line_number_;
  char const *function_name_;
  boost::stacktrace::stacktrace stacktrace_;
}; // class thrower_

template<typename E, typename... Args>
class ostreamable_thrower_arguments_
{
private:
  static_assert((std::is_reference_v<Args> && ...));

public:
  using exception_type = E;

  static constexpr std::size_t size()
  {
    return sizeof...(Args);
  }

  static constexpr bool has_message()
  {
    return true;
  }

  explicit ostreamable_thrower_arguments_(std::tuple<Args...> &&args) noexcept
    : args_(std::move(args)),
      ss_()
  {}

  ostreamable_thrower_arguments_(ostreamable_thrower_arguments_ const &) = delete;

  ostreamable_thrower_arguments_(ostreamable_thrower_arguments_ &&rhs) = default;

  ostreamable_thrower_arguments_ &operator=(ostreamable_thrower_arguments_ const &) = delete;

  std::tuple<Args...> arguments() &&
  {
    return std::move(args_);
  }

  std::string message() &&
  {
    return std::move(ss_).str();
  }

  template<typename T>
  ostreamable_thrower_arguments_ &&operator<<(T const &value) &&
  {
    ss_ << value;
    return std::move(*this);
  }

private:
  std::tuple<Args...> args_;
  std::stringstream ss_;
}; // class ostreamable_thrower_arguments_

template<typename E, typename... Args>
class thrower_arguments_
{
private:
  static_assert((std::is_reference_v<Args> && ...));

public:
  using exception_type = E;
  using ostreamable_type = ostreamable_thrower_arguments_<E, Args...>;

  static constexpr std::size_t size()
  {
    return sizeof...(Args);
  }

  static constexpr bool has_message()
  {
    return false;
  }

  thrower_arguments_(Args... args) noexcept
    : args_(std::forward_as_tuple(std::forward<Args>(args)...))
  {}

  thrower_arguments_(thrower_arguments_ const &) = delete;

  thrower_arguments_ &operator=(thrower_arguments_ const &) = delete;

  std::tuple<Args...> arguments() &&
  {
    return std::move(args_);
  }

  template<typename T>
  ostreamable_type operator<<(T const &value) &&
  {
    return ostreamable_type(std::move(args_)) << value;
  }

  ostreamable_type operator<<(std::ostream &(*pf)(std::ostream &)) &&
  {
    return ostreamable_type(std::move(args_)) << pf;
  }

  ostreamable_type operator<<(std::ios &(*pf)(std::ios &)) &&
  {
    return ostreamable_type(std::move(args_)) << pf;
  }

  ostreamable_type operator<<(std::ios_base &(*pf)(std::ios_base &)) &&
  {
    return ostreamable_type(std::move(args_)) << pf;
  }

private:
  std::tuple<Args...> args_;
}; // class thrower_arguments_

struct thrower_arguments_maker_
{
  template<typename E, typename... Args>
  thrower_arguments_<E, Args &&...> make_(Args &&... args) const && noexcept
  {
    return thrower_arguments_<E, Args &&...>(std::forward<Args>(args)...);
  }
}; // struct thrower_arguments_maker_

} // namespace corio::detail_

#define CORIO_THROW                                                                             \
::corio::detail_::thrower_(__FILE__, __LINE__, __func__, ::boost::stacktrace::stacktrace()) <<= \
::corio::detail_::thrower_arguments_maker_().template make_                                     \
  /**/

#endif // !defined(CORIO_UTIL_THROW_HPP_INCLUDE_GUARD)
