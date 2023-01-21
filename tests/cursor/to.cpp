/* (c) 2022 Галушин Павел Викторович, galushin@gmail.com

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

// Тестируемый заголовочный файл
#include <saga/cursor/to.hpp>

// Инфраструктура тестирования
#include "../saga_test.hpp"
#include <catch2/catch_amalgamated.hpp>

// Вспомогательные файлы
#include <saga/algorithm.hpp>
#include <saga/cursor/subrange.hpp>
#include <saga/cursor/istream_cursor.hpp>
#include <saga/cursor/indices.hpp>

#include <vector>
#include <forward_list>

TEST_CASE("cursor::to : vector<T>, minimal")
{
    using Value = int;
    using Container = std::vector<Value>;

    saga_test::property_checker << [](Container const & values)
    {
        auto src_in = saga_test::make_istringstream_from_range(values);

        auto container = saga::cursor::to<Container>(saga::make_istream_cursor<Value>(src_in));

        static_assert(std::is_same<decltype(container), Container>{}, "");

        CAPTURE(values, container);

        REQUIRE(saga::equal(saga::cursor::all(container), saga::cursor::all(values)));
    };
}

TEST_CASE("cursor::to : vector<T>")
{
    using Value = int;
    using Container = std::vector<Value>;

    saga_test::property_checker << [](saga_test::container_size<Value> num)
    {
        auto const src = saga::cursor::indices(num.value);

        auto container = saga::cursor::to<Container>(src);

        static_assert(std::is_same<decltype(container), Container>{}, "");

        CAPTURE(src, container);

        REQUIRE(saga::equal(saga::cursor::all(container), src));
    };
}

TEST_CASE("cursor::to : set<T>, compare")
{
    using Value = long;

    saga_test::property_checker
    << [](std::vector<Value> const & src, saga_test::strict_weak_order<Value> const & cmp)
    {
        using Container = std::set<Value, saga_test::strict_weak_order<Value>>;

        Container const expected(src.begin(), src.end(), cmp);

        auto const actual = saga::cursor::to<Container>(saga::cursor::all(src), cmp);

        REQUIRE(actual == expected);
    };
}

TEST_CASE("cursor::to : set, compare, template")
{
    using Value = long;

    saga_test::property_checker
    << [](std::vector<Value> const & src, saga_test::strict_weak_order<Value> const & cmp)
    {
        using Container = std::set<Value, saga_test::strict_weak_order<Value>>;

        Container const expected(src.begin(), src.end(), cmp);

        auto const actual = saga::cursor::to<std::set>(saga::cursor::all(src), cmp);

        REQUIRE(actual == expected);
    };
}

TEST_CASE("cursor::to : explicit type conversion")
{
    using Value = int;
    using OtherValue = long;
    using Container = std::vector<OtherValue>;

    saga_test::property_checker << [](saga_test::container_size<Value> num)
    {
        auto const src = saga::cursor::indices(num.value);

        auto container = saga::cursor::to<Container>(src);

        static_assert(std::is_same<decltype(container), Container>{}, "");

        REQUIRE(saga::equal(saga::cursor::all(container), src));
    };
}

TEST_CASE("cursor::to : forward_list<T>")
{
    using Value = int;
    using Container = std::forward_list<Value>;

    saga_test::property_checker << [](saga_test::container_size<Value> num)
    {
        auto const src = saga::cursor::indices(num.value);

        auto container = saga::cursor::to<Container>(src);

        static_assert(std::is_same<decltype(container), Container>{}, "");

        CAPTURE(src, container);

        REQUIRE(saga::equal(saga::cursor::all(container), src));
    };
}

TEST_CASE("cursor::to : vector")
{
    using Value = int;

    saga_test::property_checker << [](saga_test::container_size<Value> num)
    {
        auto const src = saga::cursor::indices(num.value);

        auto container = saga::cursor::to<std::vector>(src);

        static_assert(std::is_same<decltype(container), std::vector<Value>>{}, "");

        CAPTURE(src, container);

        REQUIRE(saga::equal(saga::cursor::all(container), src));
    };
}

TEST_CASE("cursor::to : forward_list")
{
    using Value = int;

    saga_test::property_checker << [](saga_test::container_size<Value> num)
    {
        auto const src = saga::cursor::indices(num.value);

        auto container = saga::cursor::to<std::forward_list>(src);

        static_assert(std::is_same<decltype(container), std::forward_list<Value>>{}, "");

        CAPTURE(src, container);

        REQUIRE(saga::equal(saga::cursor::all(container), src));
    };
}

TEST_CASE("cursor::to : vector to map")
{
    using Type1 = int;
    using Type2 = std::string;

    saga_test::property_checker << [](std::vector<std::pair<Type1, Type2>> const & src)
    {
        auto const actual = saga::cursor::all(src)
                          | saga::cursor::to<std::map>();

        std::map<Type1, Type2> const expected(src.begin(), src.end());

        REQUIRE(actual == expected);
    };
}

TEST_CASE("cursor::to : map to vector")
{
    using Type1 = int;
    using Type2 = std::string;

    saga_test::property_checker << [](std::map<Type1, Type2> const & src)
    {
        auto const actual = saga::cursor::all(src)
                          | saga::cursor::to<std::vector>();

        std::vector<std::pair<Type1 const, Type2>> const expected(src.begin(), src.end());

        REQUIRE(actual == expected);
    };
}

TEST_CASE("cursor::to : vector<T>, pipe")
{
    using Value = int;
    using Container = std::vector<Value>;

    saga_test::property_checker << [](saga_test::container_size<Value> num)
    {
        auto const src = saga::cursor::indices(num.value);
        auto container = src | saga::cursor::to<Container>();

        static_assert(std::is_same<decltype(container), Container>{}, "");

        CAPTURE(src, container);

        REQUIRE(saga::equal(saga::cursor::all(container), src));
    };
}

TEST_CASE("cursor::to : forward_list<T>, pipe")
{
    using Value = int;
    using Container = std::forward_list<Value>;

    saga_test::property_checker << [](saga_test::container_size<Value> num)
    {
        auto const src = saga::cursor::indices(num.value);
        auto container = src | saga::cursor::to<Container>();

        static_assert(std::is_same<decltype(container), Container>{}, "");

        CAPTURE(src, container);

        REQUIRE(saga::equal(saga::cursor::all(container), src));
    };
}

TEST_CASE("cursor::to : vector, pipe")
{
    using Value = int;

    saga_test::property_checker << [](saga_test::container_size<Value> num)
    {
        auto const src = saga::cursor::indices(num.value);
        auto container = src | saga::cursor::to<std::vector>();

        static_assert(std::is_same<decltype(container), std::vector<Value>>{}, "");

        CAPTURE(src, container);

        REQUIRE(saga::equal(saga::cursor::all(container), src));
    };
}

TEST_CASE("cursor::to : forward_list, pipe")
{
    using Value = int;

    saga_test::property_checker << [](saga_test::container_size<Value> num)
    {
        auto const src = saga::cursor::indices(num.value);
        auto container = src | saga::cursor::to<std::forward_list>();

        static_assert(std::is_same<decltype(container), std::forward_list<Value>>{}, "");

        CAPTURE(src, container);

        REQUIRE(saga::equal(saga::cursor::all(container), src));
    };
}

TEST_CASE("cursor::to : set<T>, compare, pipe")
{
    using Value = long;

    saga_test::property_checker
    << [](std::vector<Value> const & src, saga_test::strict_weak_order<Value> const & cmp)
    {
        using Container = std::set<Value, saga_test::strict_weak_order<Value>>;

        Container const expected(src.begin(), src.end(), cmp);

        auto const actual = saga::cursor::all(src)
                          | saga::cursor::to<Container>(cmp);

        REQUIRE(actual == expected);
    };
}

TEST_CASE("cursor::to : set, compare, template, pipe")
{
    using Value = long;

    saga_test::property_checker
    << [](std::vector<Value> const & src, saga_test::strict_weak_order<Value> const & cmp)
    {
        using Container = std::set<Value, saga_test::strict_weak_order<Value>>;

        Container const expected(src.begin(), src.end(), cmp);

        auto const actual = saga::cursor::all(src)
                          | saga::cursor::to<std::set>(cmp);

        REQUIRE(actual == expected);
    };
}

#include <list>

TEST_CASE("cursor::to : nested ranges")
{
    saga_test::property_checker << [](std::list<std::forward_list<int>> const & src)
    {
        auto const dest = saga::cursor::to<std::vector<std::vector<int>>>(saga::cursor::all(src));

        CAPTURE(src, dest);

        auto const eq_containers = [](auto const & lhs, auto const & rhs)
        {
            return saga::equal(saga::cursor::all(lhs), saga::cursor::all(rhs));
        };

        REQUIRE(saga::equal(saga::cursor::all(dest), saga::cursor::all(src), eq_containers));
    };
}
