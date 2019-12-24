#include <saga/cpp20/span.hpp>

#include <catch/catch.hpp>

TEST_CASE("span: types and static constants, static extent")
{
    using Value = double;
    using Element = Value const;
    constexpr auto const n_elements = 13;
    using Span = saga::span<Element, n_elements>;

    static_assert(std::is_same<Span::element_type, Element>::value, "");
    static_assert(std::is_same<Span::value_type, Value>::value, "");
    static_assert(std::is_same<Span::index_type, ptrdiff_t>::value, "");
    static_assert(std::is_same<Span::difference_type, ptrdiff_t>::value, "");
    static_assert(std::is_same<Span::pointer, Element*>::value, "");
    static_assert(std::is_same<Span::reference, Element&>::value, "");
    static_assert(std::is_same<Span::iterator, Element&>::value, "");
    static_assert(std::is_same<Span::const_iterator, Element&>::value, "");
    static_assert(std::is_same<Span::reverse_iterator, Element&>::value, "");
    static_assert(std::is_same<Span::const_reverse_iterator, Element&>::value, "");
    static_assert(Span::extent == n_elements, "");
    static_assert(std::is_same<decltype(Span::extent), index_type>::value, "");
}

// @todo ѕроверка типов и статических констант дл€ dynamic_extent
