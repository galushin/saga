#include <saga/cpp20/span.hpp>

#include <catch/catch.hpp>

// @todo Убедиться, что итераторы являются непрерывными

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

    using Iter = Span::iterator;
    using CIter = Span::const_iterator;
    static_assert(std::is_same<std::iterator_traits<Iter>::iterator_category,
                               std::random_access_iterator_tag>::value, "");
    static_assert(std::is_same<std::iterator_traits<CIter>::iterator_category,
                               std::random_access_iterator_tag>::value, "");
    static_assert(std::is_same<Span::reverse_iterator,
                               std::reverse_iterator<Iter>>::value, "");
    static_assert(std::is_same<Span::const_reverse_iterator,
                               std::reverse_iterator<CIter>>::value, "");
    static_assert(Span::extent == n_elements, "");
    static_assert(std::is_same<decltype(Span::extent),
                               Span::index_type const>::value, "");
}

TEST_CASE("span: types and static constants, dynamic extent")
{
    using Value = double;
    using Element = Value const;
    using Span_default = saga::span<Element>;
    using Span = saga::span<Element, saga::dynamic_extent>;

    static_assert(std::is_same<Span, Span_default>::value, "");

    static_assert(std::is_same<Span::element_type, Element>::value, "");
    static_assert(std::is_same<Span::value_type, Value>::value, "");
    static_assert(std::is_same<Span::index_type, ptrdiff_t>::value, "");
    static_assert(std::is_same<Span::difference_type, ptrdiff_t>::value, "");
    static_assert(std::is_same<Span::pointer, Element*>::value, "");
    static_assert(std::is_same<Span::reference, Element&>::value, "");

    using Iter = Span::iterator;
    using CIter = Span::const_iterator;
    static_assert(std::is_same<std::iterator_traits<Iter>::iterator_category,
                               std::random_access_iterator_tag>::value, "");
    static_assert(std::is_same<std::iterator_traits<CIter>::iterator_category,
                               std::random_access_iterator_tag>::value, "");
    static_assert(std::is_same<Span::reverse_iterator,
                               std::reverse_iterator<Iter>>::value, "");
    static_assert(std::is_same<Span::const_reverse_iterator,
                               std::reverse_iterator<CIter>>::value, "");
    static_assert(Span::extent == saga::dynamic_extent, "");
    static_assert(std::is_same<decltype(Span::extent),
                               Span::index_type const>::value, "");
}

TEST_CASE("span : default constructor")
{
    using Element = int;

    // Динамическая размерность
    const constexpr saga::span<Element> obj{};

    static_assert(obj.size() == 0, "");
    static_assert(obj.data() == nullptr, "");
    static_assert(noexcept(saga::span<Element>{}), "");

    // Нулевая размерность
    const constexpr saga::span<Element, 0> obj_0{};

    static_assert(obj_0.size() == 0, "");
    static_assert(obj_0.data() == nullptr, "");
    static_assert(noexcept(saga::span<Element, 0>{}), "");

    // Статическая положительная размерность
    static_assert(!std::is_default_constructible<saga::span<Element, 7>>::value, "");
}

// @todo Аналог для span со статическим размером
TEST_CASE("span : initialization from pointer and size")
{
    using Element = int;

    std::vector<Element> const src{1, 2, 3, 5, 8, 13};

    auto const n = src.size() / 2;
    auto const ptr = src.data() + 1;

    saga::span<Element const> const s(ptr, n);

    REQUIRE(s.size() == n);
    REQUIRE(s.data() == ptr);
}

