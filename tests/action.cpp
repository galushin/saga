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
#include <saga/action/unique.hpp>
#include <saga/action/sort.hpp>
#include <saga/action/reverse.hpp>

// Инфраструктура тестирования
#include "./saga_test.hpp"
#include <catch2/catch_amalgamated.hpp>

// Используемые файлы
#include <saga/algorithm.hpp>
#include <saga/cursor/subrange.hpp>

#include <list>

// Тесты
// reverse
TEST_CASE("action::reverse : function")
{
    saga_test::property_checker << [](std::list<int> const & src_old)
    {
        auto src = src_old;

        auto const actual = saga::action::reverse(src);

        auto const expected = [&]
        {
            auto result = src_old;
            saga::reverse(saga::cursor::all(result));
            return result;
        }();

        REQUIRE(actual == expected);
    };
}

TEST_CASE("action::reverse : pipes")
{
    saga_test::property_checker << [](std::list<int> const & src_old)
    {
        auto src = src_old;

        auto const actual = std::move(src) | saga::action::reverse;

        auto const expected = [&]
        {
            auto result = src_old;
            saga::reverse(saga::cursor::all(result));
            return result;
        }();

        REQUIRE(actual == expected);
    };
}

TEST_CASE("action::reverse : pipe-assign")
{
    saga_test::property_checker << [](std::list<int> const & src_old)
    {
        auto actual = src_old;

        actual |= saga::action::reverse;

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
    void test_action_sort(Container const & src_old, Args... args)
    {
        auto src = src_old;

        auto const actual = saga::action::sort(std::move(src), args...);

        auto const expected = [&]
        {
            auto result = src_old;
            saga::sort(saga::cursor::all(result), args...);
            return result;
        }();

        REQUIRE(actual == expected);
    }
}

TEST_CASE("action::sort : function")
{
    saga_test::property_checker << [](std::vector<int> const & src_old)
    {
        ::test_action_sort(src_old);
        ::test_action_sort(src_old, std::greater<>{});
    };
}

TEST_CASE("action::sort : default compare, pipes")
{
    saga_test::property_checker << [](std::vector<int> const & src_old)
    {
        auto src = src_old;

        auto const actual = std::move(src) | saga::action::sort;

        auto const expected = [&]
        {
            auto result = src_old;
            saga::sort(saga::cursor::all(result));
            return result;
        }();

        REQUIRE(actual == expected);
    };
}

TEST_CASE("action::sort : custom compare, pipes")
{
    saga_test::property_checker << [](std::vector<int> const & src_old)
    {
        auto const cmp = std::greater<>{};

        auto src = src_old;

        auto const actual = std::move(src) | saga::action::sort(cmp);

        auto const expected = [&]
        {
            auto result = src_old;
            saga::sort(saga::cursor::all(result), cmp);
            return result;
        }();

        REQUIRE(actual == expected);
    };
}

TEST_CASE("action::sort : default compare, pipes-assign")
{
    saga_test::property_checker << [](std::vector<int> const & src_old)
    {
        auto actual = src_old;
        actual |= saga::action::sort;

        auto const expected = [&]
        {
            auto result = src_old;
            saga::sort(saga::cursor::all(result));
            return result;
        }();

        REQUIRE(actual == expected);
    };
}

TEST_CASE("action::sort : custom compare, pipe-assign")
{
    saga_test::property_checker << [](std::vector<int> const & src_old)
    {
        auto const cmp = std::greater<>{};

        auto actual = src_old;

        actual |= saga::action::sort(cmp);

        auto const expected = [&]
        {
            auto result = src_old;
            saga::sort(saga::cursor::all(result), cmp);
            return result;
        }();

        REQUIRE(actual == expected);
    };
}

// unique
namespace
{
    template <class Container, class... Args>
    void test_action_unique_function(Container const & src_old, Args... args)
    {
        auto src = src_old;

        auto const actual = saga::action::unique(std::move(src), args...);

        auto const expected = [&]
        {
            auto result = src_old;
            result.erase(std::unique(result.begin(), result.end(), args...), result.end());
            return result;
        }();

        REQUIRE(actual == expected);
    }
}

TEST_CASE("action::unique : function")
{
    using Value = int;
    using Binary_predicate = std::function<bool(Value const &, Value const &)>;

    saga_test::property_checker
        << [](std::vector<Value> const & src_old, Binary_predicate const & bin_pred)
    {
        ::test_action_unique_function(src_old);
        ::test_action_unique_function(src_old, bin_pred);
    };
}

TEST_CASE("action::unique : default compare, pipes")
{
    using Value = std::string;
    using Container = std::vector<Value>;

    saga_test::property_checker << [](Container const & src)
    {
        auto const actual = Container(src) | saga::action::unique;

        auto const expected = saga::action::unique(Container(src));

        REQUIRE(actual == expected);
    };
}

TEST_CASE("action::unique : custom compare, pipes")
{
    using Value = int;
    using Container = std::vector<Value>;
    using BinaryPredicate = std::function<bool(Value const &, Value const &)>;

    saga_test::property_checker << [](Container const & src, BinaryPredicate bin_pred)
    {
        auto const actual = Container(src) | saga::action::unique(bin_pred);

        auto const expected = saga::action::unique(Container(src), bin_pred);

        REQUIRE(actual == expected);
    };
}

TEST_CASE("action::unique : default compare, pipes-assign")
{
    using Value = std::string;
    using Container = std::vector<Value>;

    saga_test::property_checker << [](Container const & src)
    {
        auto actual = src;
        actual |= saga::action::unique;

        auto const expected = saga::action::unique(Container(src));

        REQUIRE(actual == expected);
    };
}

TEST_CASE("action::unique : custom compare, pipes-assign")
{
    using Value = long;
    using Container = std::vector<Value>;
    using BinaryPredicate = std::function<bool(Value const &, Value const &)>;

    saga_test::property_checker << [](Container const & src, BinaryPredicate bin_pred)
    {
        auto actual = src;
        actual |= saga::action::unique(bin_pred);

        auto const expected = saga::action::unique(Container(src), bin_pred);

        REQUIRE(actual == expected);
    };
}
