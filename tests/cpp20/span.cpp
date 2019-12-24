/* (c) 2019 Галушин Павел Викторович, galushin@gmail.com

Данный файл -- часть библиотеки SAGA.

SAGA -- это свободной программное обеспечение: вы можете перераспространять ее и/или изменять ее
на условиях Стандартной общественной лицензии GNU в том виде, в каком она была опубликована Фондом
свободного программного обеспечения; либо версии 3 лицензии, либо (по вашему выбору) любой более
поздней версии.

Это программное обеспечение распространяется в надежде, что оно будет полезной, но БЕЗО ВСЯКИХ
ГАРАНТИЙ; даже без неявной гарантии ТОВАРНОГО ВИДА или ПРИГОДНОСТИ ДЛЯ ОПРЕДЕЛЕННЫХ ЦЕЛЕЙ.
Подробнее см. в Стандартной общественной лицензии GNU.

Вы должны были получить копию Стандартной общественной лицензии GNU вместе с этим программным
обеспечение. Если это не так, см. https://www.gnu.org/licenses/.
*/

#include <saga/cpp20/span.hpp>

#include <catch/catch.hpp>

// @todo Убедиться, что итераторы являются непрерывными

// Типы
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

// Конструирование, копирование, присваивание
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

    static_assert(obj_0.empty(), "");
    static_assert(obj_0.size() == 0, "");
    static_assert(obj_0.size_bytes() == 0, "");
    static_assert(obj_0.data() == nullptr, "");
    static_assert(noexcept(saga::span<Element, 0>{}), "");
    static_assert(noexcept(obj_0.size()), "");
    static_assert(noexcept(obj_0.size_bytes()), "");
    static_assert(noexcept(obj_0.data()), "");
    static_assert(noexcept(obj_0.empty()), "");

    // Статическая положительная размерность
    static_assert(!std::is_default_constructible<saga::span<Element, 7>>::value, "");
}

TEST_CASE("span : initialization from pointer and size")
{
    using Element = int;

    std::vector<Element> const src{1, 2, 3, 5, 8, 13};
    auto const offset = 1;

    for(auto n = 0*src.size(); n + offset <= src.size() / 2; ++ n)
    {
        auto const ptr = src.data() + offset;

        saga::span<Element const> const s(ptr, n);

        REQUIRE(s.empty() == (n == 0));
        REQUIRE(s.size() == n);
        REQUIRE(s.size_bytes() == n*sizeof(Element));
        REQUIRE(s.data() == ptr);
    }
}

TEST_CASE("span : initialization from range, defined by two pointers")
{
    using Element = int;

    std::vector<Element> const src{1, 2, 3, 5, 8, 13};

    auto const first = src.data() + 1;
    auto const last = src.data() + src.size() / 2;

    saga::span<Element const> const s(first, last);

    REQUIRE(s.size() == last - first);
    REQUIRE(s.data() == first);
}

TEST_CASE("span : initialization from C array")
{
    using Element = int;

    Element src[] = {1, 2, 3, 5, 8, 13};
    auto const N = sizeof(src) / sizeof(src[0]);

    saga::span<Element> const s(src);
    static_assert(noexcept(saga::span<Element>(src)), "");

    REQUIRE(s.size() == N);
    REQUIRE(s.data() == src);
}

TEST_CASE("span : initialization from std::array")
{
    using Element = int;

    std::array<Element, 6> src = {1, 2, 3, 5, 8, 13};

    saga::span<Element> const s(src);
    static_assert(noexcept(saga::span<Element>(src)), "");

    REQUIRE(s.size() == src.size());
    REQUIRE(s.data() == src.data());
}

TEST_CASE("span : initialization from const std::array")
{
    using Element = int;

    std::array<Element, 6> const src = {1, 2, 3, 5, 8, 13};

    saga::span<Element const> const s(src);
    static_assert(noexcept(saga::span<Element>(src)), "");

    REQUIRE(s.size() == src.size());
    REQUIRE(s.data() == src.data());
}

TEST_CASE("span : initialization from contiguous container (vector)")
{
    using Element = int;

    std::vector<Element> src{1, 2, 3, 5, 8, 13};

    saga::span<Element const> const s(src);

    REQUIRE(s.size() == src.size());
    REQUIRE(s.data() == src.data());
}

TEST_CASE("span : initialization from const contiguous container (vector)")
{
    using Element = int;

    std::vector<Element> const src{1, 2, 3, 5, 8, 13};

    saga::span<Element const> const s(src);

    REQUIRE(s.size() == src.size());
    REQUIRE(s.data() == src.data());
}

// @todo Конструирование на основе valarray

TEST_CASE("span : copy ctor")
{
    using Element = int;

    Element src[] = {1, 2, 3, 5, 8, 13};

    saga::span<Element> const s1(src);

    auto const s2(s1);

    static_assert(noexcept(saga::span<Element>(s1)), "");

    REQUIRE(s2.size() == s1.size());
    REQUIRE(s2.data() == s1.data());
}

TEST_CASE("span : ctor from other instantiation span")
{
    using Element = int;

    Element src[] = {1, 2, 3, 5, 8, 13};

    saga::span<Element> const s1(src);

    saga::span<Element const> const s2(s1);

    static_assert(noexcept(saga::span<Element const>(s1)), "");
    static_assert(!std::is_constructible<saga::span<double>, saga::span<int>>::value, "");

    REQUIRE(s2.size() == s1.size());
    REQUIRE(s2.data() == s1.data());
}

TEST_CASE("span : copy assign")
{
    using Element = int;

    Element src[] = {1, 2, 3, 5, 8, 13};

    saga::span<Element> const s1(src);
    saga::span<Element> s2;

    REQUIRE(s2.size() != s1.size());
    REQUIRE(s2.data() != s1.data());

    s2 = s1;

    static_assert(noexcept(s2 = s1), "");

    REQUIRE(s2.size() == s1.size());
    REQUIRE(s2.data() == s1.data());
}

// Подинтервалы
TEST_CASE("span : first and last")
{
    using Element = int;

    std::vector<Element> const src{1, 2, 3, 5, 8, 13};

    saga::span<Element const> const s(src);

    auto const n = 3;
    auto const s1 = s.first(n);
    auto const s2 = s.last(n);

    static_assert(std::is_same<decltype(s1), saga::span<Element const> const>::value, "");

    REQUIRE(s1.size() == n);
    REQUIRE(s1.data() == s.data());

    REQUIRE(s2.size() == n);
    REQUIRE(s2.data() == s.data() + (s.size() - n));
}

TEST_CASE("span : subspan")
{
    using Element = int;

    std::vector<Element> const src{1, 2, 3, 5, 8, 13};

    saga::span<Element const> const s(src);

    auto const n = src.size() / 2;
    auto const offset = 2;

    auto const s1 = s.subspan(offset, n);

    static_assert(std::is_same<decltype(s1), saga::span<Element const> const>::value, "");

    REQUIRE(s1.size() == n);
    REQUIRE(s1.data() == s.data() + offset);

    auto const s2 = s.subspan(offset, saga::dynamic_extent);

    REQUIRE(s2.size() == s.size() - offset);
    REQUIRE(s2.data() == s.data() + offset);

    auto const s3 = s.subspan(offset);

    REQUIRE(s3.size() == s.size() - offset);
    REQUIRE(s3.data() == s.data() + offset);
}

// Свойства (observers): проверяются в предыдущих тестах

// Доступ к элементам
// data проверяется в предыдущих тестах, operator() исключён в p1024r2
TEST_CASE("span : operator []")
{
     using Element = int;

    std::vector<Element> src{1, 2, 3, 5, 8, 13};

    saga::span<Element> const s(src);

    static_assert(std::is_same<decltype(s[0]), Element &>::value, "");

    for(auto i = 0*s.size(); i != s.size(); ++ i)
    {
        REQUIRE(s[i] == src[i]);
        REQUIRE(std::addressof(s[i]) == std::addressof(src[i]));
    }
}
