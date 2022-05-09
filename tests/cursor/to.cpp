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
#include <catch/catch.hpp>

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
