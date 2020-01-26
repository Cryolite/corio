#if !defined(CORIO_CORE_ERROR_HPP_INCLUDE_GUARD)
#define CORIO_CORE_ERROR_HPP_INCLUDE_GUARD

#include <string>
#include <stdexcept>


namespace corio{

class no_executor_error
  : public std::logic_error
{
public:
  explicit no_executor_error()
    : std::logic_error("Attempting to perform an operation requiring"
                       " an executor before assigning a executor.")
  {}
}; // class no_executor_error

class bad_executor_error
  : public std::logic_error
{
public:
  explicit bad_executor_error()
    : std::logic_error("Attempting to perform an operation on a bad executor.")
  {}
}; // class bad_executor_error

class executor_already_assigned_error
  : public std::logic_error
{
public:
  executor_already_assigned_error()
    : std::logic_error("An executor has been already assigned to an awaitable object.")
  {}
}; // class executor_already_assigned_error

class broken_promise_error
  : public std::logic_error
{
public:
  broken_promise_error()
    : std::logic_error("A promise object was destructed before any future object was retrieved from it.")
  {}
}; // class broken_promise_error

class future_already_retrieved_error
  : public std::logic_error
{
public:
  future_already_retrieved_error()
    : std::logic_error("Attempting to retrieve a future object from a promise object"
                       " from which a future object has been already retrieved.")
  {}
}; // class future_already_retrieved_error

class no_future_state_error
  : public std::logic_error
{
public:
  no_future_state_error()
    : std::logic_error("There is no shared future state, which indicates"
                       " that a prohibited operation was made on a default-constructed or moved-from object.")
  {}
}; // class no_future_state_error

class promise_already_satisfied_error
  : public std::logic_error
{
public:
  promise_already_satisfied_error()
    : std::logic_error("Attempting to set a value or exception to an already satisfied promise object.")
  {}
}; // class promise_already_satisfied_error

class invalid_coroutine_error
  : public std::logic_error
{
public:
  invalid_coroutine_error()
    : std::logic_error("Attempt to make a prohibited operation on an invalid coroutine object,"
                       " which indicates that the object is default-constructed or moved-from.")
  {}
}; // class invalid_coroutine_error

class coroutine_already_done_error
  : public std::logic_error
{
public:
  coroutine_already_done_error()
    : std::logic_error("Attempting to resume an already done coroutine object.")
  {}
}; // class coroutine_already_done_error

} // namespace corio

#endif // !defined(CORIO_CORE_ERROR_HPP_INCLUDE_GUARD)
