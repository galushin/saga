/* (c) 2021 Галушин Павел Викторович, galushin@gmail.com

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
#include <saga/numeric.hpp>

// Тестовая инфраструктура
#include "saga_test.hpp"
#include <catch/catch.hpp>

// Вспомогательные возможности, используемые в тестах
#include <saga/cursor/subrange.hpp>
#include <saga/cursor/istream_cursor.hpp>

#include <list>
#include <vector>

// Тесты
// accumulate
TEST_CASE("accumulate - default operation")
{
    using Element = int;
    using InitValue = long;

    static_assert(!std::is_same<Element, InitValue>{}, "");

    saga_test::property_checker << [](std::vector<Element> container,
                                      InitValue const & init_value)
    {
        // Так суммирование элементов массива точно не приведёт к неопределённому поведению
        std::sort(container.begin(), container.end());
        std::adjacent_difference(container.begin(), container.end(), container.begin());

        auto const values = saga_test::random_subrange_of(container);

        auto const expected = std::accumulate(values.first, values.second, init_value);

        auto cur = saga::make_subrange_cursor(values.first, values.second);
        auto const actual  = saga::accumulate(std::move(cur), init_value);

        REQUIRE(actual == expected);

        static_assert(std::is_same<InitValue const, decltype(actual)>{}, "");
    };
}

TEST_CASE("accumulate - generic operation")
{
    using Element = int;
    using InitValue = long;

    static_assert(!std::is_same<Element, InitValue>{}, "");

    saga_test::property_checker << [](std::vector<Element> const & container,
                                      InitValue const & init_value)
    {
        auto const values = saga_test::random_subrange_of(container);

        auto const min_op = [](InitValue const & lhs, InitValue const & rhs)
        {
            return std::min(lhs, rhs);
        };

        auto const expected = std::accumulate(values.first, values.second, init_value, min_op);

        auto input_range = saga::make_subrange_cursor(values.first, values.second);
        auto input = saga_test::make_istringstream_from_range(input_range);

        auto const actual
            = saga::accumulate(saga::make_istream_cursor<Element>(input), init_value, min_op);

        REQUIRE(actual == expected);

        static_assert(std::is_same<InitValue const, decltype(actual)>{}, "");
    };
}

TEST_CASE("accumulate - constexpr")
{
    constexpr int values[] = {1, 2, 3, 4, 5};

    constexpr auto const sum = saga::accumulate(saga::cursor::all(values), 1, std::multiplies<>{});

    static_assert(sum == 120, "");
}

TEST_CASE("accumulate - move only init value")
{
    saga_test::property_checker << [](std::vector<int> values, int const & init_value)
    {
        // Так суммирование элементов массива точно не приведёт к неопределённому поведению
        std::sort(values.begin(), values.end());
        std::adjacent_difference(values.begin(), values.end(), values.begin());

        auto const result_std = std::accumulate(values.begin(), values.end(), init_value);

        auto const my_plus = [](saga_test::move_only<int> lhs, int rhs)
        {
            lhs.value += rhs;
            return lhs;
        };

        auto const result_saga = saga::accumulate(saga::cursor::all(values),
                                                  saga_test::move_only<int>(init_value), my_plus);

        REQUIRE(result_saga.value == result_std);
    };
}

// inner_product
TEST_CASE("inner_product: default operations")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> lhs,
                                      std::list<bool> const & rhs,
                                      Value const & init_value)
    {
        // Так суммирование произведений точно не приведёт к неопределённому поведению
        std::sort(lhs.begin(), lhs.end());
        std::adjacent_difference(lhs.begin(), lhs.end(), lhs.begin());

        auto const n_min = std::min(lhs.size(), rhs.size());

        auto const result_std
            = std::inner_product(lhs.begin(), lhs.begin() + n_min, rhs.begin(), init_value);

        auto const result_saga
            = saga::inner_product(saga::cursor::all(lhs), saga::cursor::all(rhs), init_value);

        static_assert(std::is_same<decltype(result_saga), Value const>{}, "");

        REQUIRE(result_saga == result_std);
    };
}

TEST_CASE("inner_product: generic operations")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & lhs,
                                      std::list<Value> const & rhs,
                                      Value const & init_value)
    {
        auto op1 = [](Value const & x, Value const & y)
        {
            return std::min(x, y);
        };

        auto op2 = [](Value const & x, Value const & y)
        {
            return std::max(x, y);
        };

        auto const n_min = std::min(lhs.size(), rhs.size());

        auto const result_std = std::inner_product(lhs.begin(), lhs.begin() + n_min, rhs.begin(),
                                                   init_value, op1, op2);

        auto const result_saga = saga::inner_product(saga::cursor::all(lhs), saga::cursor::all(rhs),
                                                     init_value, op1, op2);

        static_assert(std::is_same<decltype(result_saga), Value const>{}, "");

        REQUIRE(result_saga == result_std);
    };
};

TEST_CASE("inner_product - move only init value")
{
    saga_test::property_checker << [](std::vector<int> values,
                                      std::list<bool> const & mask,
                                      int const & init_value)
    {
        // Так суммирование произведений точно не приведёт к неопределённому поведению
        std::sort(values.begin(), values.end());
        std::adjacent_difference(values.begin(), values.end(), values.begin());

        auto const n_min = std::min(values.size(), mask.size());
        auto const result_std
            = std::inner_product(values.begin(), values.begin() + n_min, mask.begin(), init_value);

        auto const my_plus = [](saga_test::move_only<int> lhs, int rhs)
        {
            lhs.value += rhs;
            return lhs;
        };

        auto const result_saga = saga::inner_product(saga::cursor::all(values),
                                                     saga::cursor::all(mask),
                                                     saga_test::move_only<int>(init_value),
                                                     my_plus, std::multiplies<>{});

        REQUIRE(result_saga.value == result_std);
    };
}

TEST_CASE("inner_product - constexpr, generic")
{
    constexpr int values[] = {1, 2, 3, 4, 5};

    constexpr auto const sum = saga::inner_product(saga::cursor::all(values),
                                                   saga::cursor::all(values), 0);

    static_assert(sum == 55, "");
}
