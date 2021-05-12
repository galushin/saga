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

// Тесты
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
    constexpr std::array<int, 5> values = {1, 2, 3, 4, 5};

    constexpr auto const sum = saga::accumulate(saga::cursor::all(values), 1, std::multiplies<>{});

    static_assert(sum == 120, "");
}

TEST_CASE("accumulate - move only init value")
{
    saga_test::property_checker << [](std::vector<int> const & values, int const & init_value)
    {
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
