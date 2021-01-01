/* (c) 2019-2021 Галушин Павел Викторович, galushin@gmail.com

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

#include <saga/iterator.hpp>

#include <catch/catch.hpp>

#include <saga/view/indices.hpp>
#include "../saga_test.hpp"

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
    static_assert(std::is_same<Span::index_type, std::ptrdiff_t>::value, "");
    static_assert(std::is_same<Span::difference_type, std::ptrdiff_t>::value, "");
    static_assert(std::is_same<Span::pointer, Element*>::value, "");
    static_assert(std::is_same<Span::reference, Element&>::value, "");

    struct span_structure
    {
        Span::pointer ptr;
        Span::index_type size;
    };

    static_assert(sizeof(Span) <= sizeof(span_structure), "");
    static_assert(sizeof(saga::span<int, 42>) == sizeof(int *), "");

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

    saga_test::property_checker
    << [](std::vector<Element> const & src)
    {
        auto const offset = saga_test::random_position_of(src);
        auto const num = saga_test::random_uniform(0*src.size(), src.size() - offset);

        auto const ptr = src.data() + offset;
        saga::span<Element const> const s(ptr, num);

        REQUIRE(s.empty() == (num == 0));
        REQUIRE(s.size() == num);
        REQUIRE(s.size_bytes() == num*sizeof(Element));
        REQUIRE(s.data() == ptr);
    };
}

TEST_CASE("span : initialization from range, defined by two pointers")
{
    using Element = int;

    saga_test::property_checker
    << [](std::vector<Element> const & src)
    {
        auto const first = src.data() + !src.empty();
        auto const last = src.data() + src.size() / 2;

        saga::span<Element const> const s(first, last);

        REQUIRE(s.size() == last - first);
        REQUIRE(s.data() == first);
    };
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

    static_assert(!std::is_constructible<saga::span<int>, double(&)[10]>{}, "");
}

TEST_CASE("span (fixed extent) : initialization from C array")
{
    using Element = int;

    Element src[] = {1, 2, 3, 5, 8, 13};
    constexpr auto const N = sizeof(src) / sizeof(src[0]);

    saga::span<Element, N> const s(src);
    static_assert(noexcept(saga::span<Element>(src)), "");

    REQUIRE(s.size() == N);
    REQUIRE(s.data() == src);

    constexpr auto const M = N+1;
    static_assert(M != N, "");

    static_assert(!std::is_constructible<saga::span<Element, M>, Element(&)[N]>{}, "");
    static_assert(!std::is_constructible<saga::span<int, N>, double(&)[N]>{}, "");
}

TEST_CASE("span : initialization from std::array")
{
    using Element = int;

    saga_test::property_checker << [](std::array<Element, 6> src)
    {
        saga::span<Element> const s(src);
        static_assert(noexcept(saga::span<Element>(src)), "");

        REQUIRE(s.size() == src.size());
        REQUIRE(s.data() == src.data());
    };
}

TEST_CASE("span : initialization from const std::array")
{
    using Element = int;

    saga_test::property_checker << [](std::array<Element, 6> const & src)
    {
        saga::span<Element const> const s(src);
        static_assert(noexcept(saga::span<Element const>(src)), "");

        REQUIRE(s.size() == src.size());
        REQUIRE(s.data() == src.data());
    };
}

TEST_CASE("span : initialization from contiguous container (vector)")
{
    using Element = int;

    saga_test::property_checker
    << [](std::vector<Element> src)
    {
        saga::span<Element> const s(src);

        REQUIRE(s.size() == src.size());
        REQUIRE(s.data() == src.data());
    };
}

TEST_CASE("span : initialization from const contiguous container (vector)")
{
    using Element = int;

    saga_test::property_checker
    << [](std::vector<Element> const src)
    {
        saga::span<Element const> const s(src);

        REQUIRE(s.size() == src.size());
        REQUIRE(s.data() == src.data());
    };
}

TEST_CASE("span : ctor from const string")
{
    saga_test::property_checker
    << [](std::string const src)
    {
        saga::span<char const> const s(src);

        REQUIRE(s.size() == src.size());
        REQUIRE(s.data() == src.data());
    };
}

#if __cplusplus >= 201703L
TEST_CASE("span : ctor from const string - C++17")
{
    saga_test::property_checker
    << [](std::string src)
    {
        saga::span<char> const s(src);

        REQUIRE(s.size() == src.size());
        REQUIRE(s.data() == src.data());
    };
}
#endif

static_assert(!std::is_constructible<saga::span<int>, std::vector<double> &>{}, "");
static_assert(!std::is_constructible<saga::span<int>, std::vector<int> const &>{}, "");
static_assert(!std::is_constructible<saga::span<int const>, std::vector<double> const &>{}, "");

TEST_CASE("span : copy ctor")
{
    using Element = int;

    saga_test::property_checker
    << [](std::vector<Element> src)
    {
        saga::span<Element> const s1(src);

        auto const s2(s1);

        static_assert(noexcept(saga::span<Element>(s1)), "");

        REQUIRE(s2.size() == s1.size());
        REQUIRE(s2.data() == s1.data());
    };
}

TEST_CASE("span : ctor from other instantiation span")
{
    static_assert(!std::is_constructible<saga::span<double>, saga::span<int>>::value, "");

    using Element = int;

    saga_test::property_checker
    << [](std::vector<Element> src)
    {
        saga::span<Element> const s1(src);

        saga::span<Element const> const s2(s1);

        static_assert(noexcept(saga::span<Element const>(s1)), "");

        REQUIRE(s2.size() == s1.size());
        REQUIRE(s2.data() == s1.data());
    };
}

TEST_CASE("span : copy assign")
{
    using Element = int;

    saga_test::property_checker
    << [](std::vector<Element> src)
    {
        saga::span<Element> const s1(src);
        saga::span<Element> s2;

        s2 = s1;

        static_assert(noexcept(s2 = s1), "");

        REQUIRE(s2.size() == s1.size());
        REQUIRE(s2.data() == s1.data());
    };
}

// Подинтервалы
TEST_CASE("span : first and last")
{
    using Element = int;

    saga_test::property_checker
    << [](std::vector<Element> const & src)
    {
        saga::span<Element const> const s(src);

        auto const n = saga_test::random_position_of(s);

        auto const s1 = s.first(n);
        auto const s2 = s.last(n);

        static_assert(std::is_same<decltype(s1), saga::span<Element const> const>::value, "");

        REQUIRE(s1.size() == n);
        REQUIRE(s1.data() == s.data());

        REQUIRE(s2.size() == n);
        REQUIRE(s2.data() == s.data() + (s.size() - n));
    };
}

TEST_CASE("span : subspan")
{
    using Element = int;

    saga_test::property_checker
    << [](std::vector<Element> const & src)
    {
        saga::span<Element const> const s_all(src);

        auto const offset = saga_test::random_position_of(s_all);

        auto const s_part = s_all.subspan(offset, saga::dynamic_extent);
        static_assert(std::is_same<decltype(s_part), decltype(s_all)>::value, "");

        REQUIRE(s_part.size() == s_all.size() - offset);
        REQUIRE(s_part.data() == s_all.data() + offset);
    }
    << [](std::vector<Element> const & src)
    {
        saga::span<Element const> const s_all(src);

        auto const offset = saga_test::random_position_of(s_all);

        auto const s_part = s_all.subspan(offset);
        static_assert(std::is_same<decltype(s_part), decltype(s_all)>::value, "");

        REQUIRE(s_part.size() == s_all.size() - offset);
        REQUIRE(s_part.data() == s_all.data() + offset);
    }
    << [](std::vector<Element> const & src)
    {
        saga::span<Element const> const s_all(src);

        auto const offset = saga_test::random_position_of(s_all);
        auto const num = saga_test::random_uniform(0*s_all.size(), s_all.size() - offset);

        auto const s_part = s_all.subspan(offset, num);
        static_assert(std::is_same<decltype(s_part), decltype(s_all)>::value, "");

        REQUIRE(s_part.size() == num);
        REQUIRE(s_part.data() == s_all.data() + offset);
    };
}

// Свойства (observers): проверяются в предыдущих тестах

// Доступ к элементам
// data проверяется в предыдущих тестах, operator() исключён в p1024r2
TEST_CASE("span : operator []")
{
    using Element = int;

    saga_test::property_checker
    << [](std::vector<Element> src)
    {
        saga::span<Element> const s(src);

        static_assert(std::is_same<decltype(s[0]), Element &>::value, "");

        REQUIRE(s.size() == src.size());

        for(auto const & i : saga::view::indices_of(src))
        {
            REQUIRE(s[i] == src[i]);
            REQUIRE(std::addressof(s[i]) == std::addressof(src[i]));
        }
    };
}

// Итераторы
TEST_CASE("span : iterators of empty")
{
    using Element = int;
    using Span = saga::span<Element>;
    Span const s{};

    REQUIRE(s.begin() == s.end());
    REQUIRE(s.cbegin() == s.cend());
    REQUIRE(s.rbegin() == s.rend());
    REQUIRE(s.crbegin() == s.crend());

    static_assert(std::is_same<decltype(s.begin()), Span::iterator>::value, "");
    static_assert(std::is_same<decltype(s.end()), Span::iterator>::value, "");
    static_assert(std::is_same<decltype(s.cbegin()), Span::const_iterator>::value, "");
    static_assert(std::is_same<decltype(s.cend()), Span::const_iterator>::value, "");
    static_assert(std::is_same<decltype(s.rbegin()), Span::reverse_iterator>::value, "");
    static_assert(std::is_same<decltype(s.rend()), Span::reverse_iterator>::value, "");
    static_assert(std::is_same<decltype(s.crbegin()), Span::const_reverse_iterator>::value, "");
    static_assert(std::is_same<decltype(s.crend()), Span::const_reverse_iterator>::value, "");

    static_assert(noexcept(s.begin()), "");
    static_assert(noexcept(s.end()), "");
    static_assert(noexcept(s.cbegin()), "");
    static_assert(noexcept(s.cend()), "");
    static_assert(noexcept(s.rbegin()), "");
    static_assert(noexcept(s.rend()), "");
    static_assert(noexcept(s.crbegin()), "");
    static_assert(noexcept(s.crend()), "");
}

TEST_CASE("span : iterators of not-empty")
{
    using Element = int;
    saga_test::property_checker
    << [](std::vector<Element> src)
    {
        using Span = saga::span<Element>;
        Span const s(src);

        REQUIRE(std::addressof(*s.begin()) == std::addressof(src.front()));
        REQUIRE(std::addressof(*s.cbegin()) == std::addressof(src.front()));
        REQUIRE(std::addressof(*s.rbegin()) == std::addressof(src.back()));
        REQUIRE(std::addressof(*s.crbegin()) == std::addressof(src.back()));

        REQUIRE(s.rbegin() == Span::reverse_iterator(s.end()));
        REQUIRE(s.rend() == Span::reverse_iterator(s.begin()));
        REQUIRE(s.crbegin() == Span::const_reverse_iterator(s.cend()));
        REQUIRE(s.crend() == Span::const_reverse_iterator(s.cbegin()));

        REQUIRE(s.end() == s.begin() + s.size());
        REQUIRE(s.cend() == s.cbegin() + s.size());
        REQUIRE(s.rend() == s.rbegin() + s.size());
        REQUIRE(s.crend() == s.crbegin() + s.size());
    };
}

TEST_CASE("span : equality (same)")
{
    using Element = int;

    saga_test::property_checker
    << [](std::vector<Element> const & src1)
    {
        auto const src2 = src1;
        using Span = saga::span<Element const>;

        Span const s1(src1);
        Span const s2(src2);
        REQUIRE(s1 == s1);
        REQUIRE(s2 == s2);

        REQUIRE(s1 == s2);
        REQUIRE(!(s1 != s2));
    };
}

TEST_CASE("span : equality")
{
    using Element = int;

    saga_test::property_checker
    << [](std::vector<Element> const & src1, std::vector<Element> const & src2)
    {
        using Span = saga::span<Element const>;

        Span const s1(src1);
        Span const s2(src2);

        REQUIRE(s1 == s1);
        REQUIRE(s2 == s2);

        REQUIRE((s1 == s2) == (src1 == src2));
        REQUIRE((s1 != s2) == !(s1 == s2));
    };
}

TEST_CASE("span : ordering")
{
    using Element = int;

    saga_test::property_checker
    << [](std::vector<Element> const & src1, std::vector<Element> const & src2)
    {
        using Span = saga::span<Element const>;
        Span const s1(src1);
        Span const s2(src2);

        REQUIRE((s1 < s2) == (src1 < src2));
        REQUIRE((s1 <= s2) == (src1 <= src2));
        REQUIRE((s1 > s2) == (src1 > src2));
        REQUIRE((s1 >= s2) == (src1 >= src2));
    };
}

TEST_CASE("span : ordering prefix")
{
    using Element = int;

    saga_test::property_checker
    << [](std::vector<Element> const & src)
    {
        using Span = saga::span<Element const>;

        Span const s1(src);
        auto s2 = s1.first(s1.size() / 2);

        REQUIRE((s2 < s1) == !src.empty());
    };
}
