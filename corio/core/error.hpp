#if !defined(CORIO_CORE_ERROR_HPP_INCLUDE_GUARD)
#define CORIO_CORE_ERROR_HPP_INCLUDE_GUARD

#include <string>
#include <stdexcept>


namespace corio{

class context_already_set_error
  : public std::runtime_error
{
public:
  explicit context_already_set_error(std::string const &what)
    : std::runtime_error(what)
  {}
}; // class context_already_set_error

class no_context_error
  : public std::runtime_error
{
public:
  explicit no_context_error(std::string const &what)
    : std::runtime_error(what)
  {}
}; // class no_context_error

class broken_promise_error
  : public std::runtime_error
{
public:
  explicit broken_promise_error(std::string const &what)
    : std::runtime_error(what)
  {}
}; // class broken_promise_error

class future_already_retrieved_error
  : public std::runtime_error
{
public:
  explicit future_already_retrieved_error(std::string const &what)
    : std::runtime_error(what)
  {}
}; // class future_already_retrieved_error

class no_future_state_error
  : public std::runtime_error
{
public:
  explicit no_future_state_error(std::string const &what)
    : std::runtime_error(what)
  {}
}; // class no_future_state_error

class promise_already_satisfied_error
  : public std::runtime_error
{
public:
  explicit promise_already_satisfied_error(std::string const &what)
    : std::runtime_error(what)
  {}
}; // class promise_already_satisfied_error

} // namespace corio

#endif // !defined(CORIO_CORE_ERROR_HPP_INCLUDE_GUARD)
