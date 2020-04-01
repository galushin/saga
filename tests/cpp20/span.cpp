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

    saga_test::property_checker
    << [](std::vector<Element> const & src)
    {
        // @todo offset и n получать автоматизировано
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

// @todo Подумать, как перевести на тестирование, основанное на свойствах
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

// @todo Подумать, как перевести на тестирование, основанное на свойствах
TEST_CASE("span : initialization from std::array")
{
    using Element = int;

    std::array<Element, 6> src = {1, 2, 3, 5, 8, 13};

    saga::span<Element> const s(src);
    static_assert(noexcept(saga::span<Element>(src)), "");

    REQUIRE(s.size() == src.size());
    REQUIRE(s.data() == src.data());
}

// @todo Подумать, как перевести на тестирование, основанное на свойствах
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

// @todo Конструирование на основе valarray

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

        auto const n
            = std::uniform_int_distribution<std::size_t>(0, s.size())(saga_test::random_engine());

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

    REQUIRE(s.size() == src.size());

    for(auto const & i : saga::view::indices_of(src))
    {
        REQUIRE(s[i] == src[i]);
        REQUIRE(std::addressof(s[i]) == std::addressof(src[i]));
    }
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
    std::vector<Element> src{1, 2, 3, 5, 8, 13};

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
}

TEST_CASE("span : equality")
{
    using Element = int;
    using Span = saga::span<Element>;

    Element src1 [] = {1, 2, 3, 5, 8, 13};
    std::vector<Element> src2(saga::begin(src1), saga::end(src1));
    std::vector<Element> src3 = src2;
    src3[0] += 1;

    Span const s1(src1);
    Span const s2(src2);
    Span const s3(src3);
    Span const s4(src2.data(), src2.size() / 2);

    REQUIRE(src2 != src3);

    REQUIRE(s1 == s2);
    REQUIRE(!(s1 != s2));
    REQUIRE(s2 == s1);
    REQUIRE(!(s2 != s1));
    REQUIRE(s1 != s3);
    REQUIRE(s1 != s4);
    REQUIRE(s2 != s3);
    REQUIRE(s2 != s4);
}

TEST_CASE("span : ordering")
{
    using Element = int;
    using Span = saga::span<Element>;

    std::vector<Element> src1{1, 2, 3, 4};
    auto src3 = src1;

    Element src2 [] = {1, 2, 3, 5, 8, 13};

    Span const s1(src1);
    Span const s2(src2);
    auto const s2p = s2.first(4);
    Span const s3(src3);

    REQUIRE(s1 < s2);
    REQUIRE(s2p < s2);
    REQUIRE(s1 < s2p);

    REQUIRE(s2 > s1);

    REQUIRE(s2 >= s1);
    REQUIRE(s3 >= s1);

    REQUIRE(s1 <= s2);
    REQUIRE(s2p <= s2);
    REQUIRE(s1 <= s2p);
}
