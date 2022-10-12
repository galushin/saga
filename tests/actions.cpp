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

// Тестируемый файл
#include <saga/actions/sort.hpp>
#include <saga/actions/reverse.hpp>

// Инфраструктура тестирования
#include "./saga_test.hpp"
#include <catch2/catch_amalgamated.hpp>

// Используемые файлы
#include <saga/algorithm.hpp>
#include <saga/cursor/subrange.hpp>

#include <list>

// Тесты
// reverse
TEST_CASE("actions::reverse : function")
{
    saga_test::property_checker << [](std::list<int> const & src_old)
    {
        auto src = src_old;

        auto const actual = saga::actions::reverse(src);

        auto const expected = [&]
        {
            auto result = src_old;
            saga::reverse(saga::cursor::all(result));
            return result;
        }();

        REQUIRE(actual == expected);
    };
}

TEST_CASE("actions::reverse : pipes")
{
    saga_test::property_checker << [](std::list<int> const & src_old)
    {
        auto src = src_old;

        auto const actual = std::move(src) | saga::actions::reverse;

        auto const expected = [&]
        {
            auto result = src_old;
            saga::reverse(saga::cursor::all(result));
            return result;
        }();

        REQUIRE(actual == expected);
    };
}

TEST_CASE("actions::reverse : pipe-assign")
{
    saga_test::property_checker << [](std::list<int> const & src_old)
    {
        auto actual = src_old;

        actual |= saga::actions::reverse;

        auto const expected = [&]
        {
            auto result = src_old;
            saga::reverse(saga::cursor::all(result));
            return result;
        }();

        REQUIRE(actual == expected);
    };
}

// sort
namespace
{
    template <class Container, class... Args>
    void test_actions_sort(Container const & src_old, Args... args)
    {
        auto src = src_old;

        auto const actual = saga::actions::sort(std::move(src), args...);

        auto const expected = [&]
        {
            auto result = src_old;
            saga::sort(saga::cursor::all(result), args...);
            return result;
        }();

        REQUIRE(actual == expected);
    }
}

TEST_CASE("actions::sort : function")
{
    saga_test::property_checker << [](std::vector<int> const & src_old)
    {
        ::test_actions_sort(src_old);
        ::test_actions_sort(src_old, std::greater<>{});
    };
}

TEST_CASE("actions::sort : default compare, pipes")
{
    saga_test::property_checker << [](std::vector<int> const & src_old)
    {
        auto src = src_old;

        auto const actual = std::move(src) | saga::actions::sort;

        auto const expected = [&]
        {
            auto result = src_old;
            saga::sort(saga::cursor::all(result));
            return result;
        }();

        REQUIRE(actual == expected);
    };
}

TEST_CASE("actions::sort : custom compare, pipes")
{
    saga_test::property_checker << [](std::vector<int> const & src_old)
    {
        auto const cmp = std::greater<>{};

        auto src = src_old;

        auto const actual = std::move(src) | saga::actions::sort(cmp);

        auto const expected = [&]
        {
            auto result = src_old;
            saga::sort(saga::cursor::all(result), cmp);
            return result;
        }();

        REQUIRE(actual == expected);
    };
}

TEST_CASE("actions::sort : default compare, pipes-assign")
{
    saga_test::property_checker << [](std::vector<int> const & src_old)
    {
        auto actual = src_old;
        actual |= saga::actions::sort;

        auto const expected = [&]
        {
            auto result = src_old;
            saga::sort(saga::cursor::all(result));
            return result;
        }();

        REQUIRE(actual == expected);
    };
}

TEST_CASE("actions::sort : custom compare, pipe-assign")
{
    saga_test::property_checker << [](std::vector<int> const & src_old)
    {
        auto const cmp = std::greater<>{};

        auto actual = src_old;

        actual |= saga::actions::sort(cmp);

        auto const expected = [&]
        {
            auto result = src_old;
            saga::sort(saga::cursor::all(result), cmp);
            return result;
        }();

        REQUIRE(actual == expected);
    };
}
