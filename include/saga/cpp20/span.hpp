#ifndef Z_SAGA_CPP20_SPAN_HPP_INCLUDED
#define Z_SAGA_CPP20_SPAN_HPP_INCLUDED

/** @file saga/cpp20/span.hpp
 @brief Реализация std::span из C++20, описанный в:
  www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/n4741.pdf
  www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1024r0.pdf
*/

#include <cstddef>

namespace saga
{
    template <class ElementType, std::ptrdiff_t Extent>
    class span;
}
// namespace saga

#endif
// Z_SAGA_CPP20_SPAN_HPP_INCLUDED
