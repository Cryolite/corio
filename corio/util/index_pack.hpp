#if !defined(CORIO_UTIL_INDEX_PACK_HPP_INCLUDE_GUARD)
#define CORIO_UTIL_INDEX_PACK_HPP_INCLUDE_GUARD

#include <utility>
#include <cstddef>


namespace corio::detail_{

template<typename>
struct index_pack_;

template<std::size_t... Is>
struct index_pack_<std::integer_sequence<std::size_t, Is...> >
{
  template<typename F>
  auto operator->*(F &&f) const
  {
    return f(std::integral_constant<std::size_t, Is>{}...);
  }
}; // struct index_pack_

} // namespace corio::detail_

#define CORIO_INDEX_PACK(NAME, SIZE)                                                         \
corio::detail_::index_pack_<std::make_index_sequence<SIZE> >{} ->* [&](auto... NAME) -> auto \
  /**/

#endif // !defined(CORIO_UTIL_INDEX_PACK_HPP_INCLUDE_GUARD)
