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

// Инфраструктура тестирования
#include "../saga_test.hpp"
#include <catch/catch.hpp>

// Используемые возможности
#include <saga/algorithm.hpp>
#include <saga/cursor/filter.hpp>
#include <saga/cursor/subrange.hpp>
#include <saga/pipes/filter.hpp>
#include <saga/pipes/for_each.hpp>
#include <saga/numeric.hpp>
#include <saga/view/indices.hpp>

// Тесты
// PE 001 : сумма кратных 3 или 5
namespace
{
    template <class IntType>
    constexpr IntType projectEuler_001_range_for_loop_indices(IntType n_max)
    {
        auto result = IntType(0);

        for(auto num : saga::view::indices(n_max))
        {
            if(num % 3 == 0 || num % 5 == 0)
            {
                result += num;
            }
        }

        return result;
    }

    template <class IntType>
    constexpr IntType sum_of_multiples_below(IntType d, IntType n_max)
    {
        auto const num = (n_max - 1) / d;

        if(num <= 0)
        {
            return 0;
        }

        if(num % 2 == 0)
        {
            return num / 2 * (num + 1) * d;
        }
        else
        {
            return (num + 1) / 2 * num * d;
        }
    }

    template <class IntType>
    constexpr IntType projectEuler_001_closed_form(IntType n_max)
    {
        return ::sum_of_multiples_below(3, n_max)
               + ::sum_of_multiples_below(5, n_max)
               - ::sum_of_multiples_below(15, n_max);
    }

    template <class IntType>
    IntType projectEuler_001_cursor_algorithms(IntType n_max)
    {
        std::vector<IntType> tmp;
        saga::copy_if(saga::cursor::all(saga::view::indices(n_max))
                      , saga::back_inserter(tmp)
                      , [](IntType const & num) { return num % 3 == 0 || num % 5 == 0; });

        return saga::accumulate(saga::cursor::all(tmp), IntType(0));
    }

    template <class IntType>
    IntType projectEuler_001_cursor_algorithms_filter(IntType n_max)
    {
        auto pred = [](IntType const & num) { return num % 3 == 0 || num % 5 == 0; };

        auto input = saga::cursor::filter(saga::cursor::all(saga::view::indices(n_max)), pred);

        return saga::accumulate(std::move(input), IntType(0));
    }

    template <class IntType>
    IntType projectEuler_001_smart_output_cursor(IntType n_max)
    {
        auto pred = [](IntType const & num) { return num % 3 == 0 || num % 5 == 0; };

        auto result = IntType{0};
        auto accumulator = [&result](IntType const & arg) { result += arg; };

        saga::copy(saga::cursor::all(saga::view::indices(n_max))
                   , saga::pipes::filter(saga::pipes::for_each(accumulator), pred));

        return result;
    }
}

static_assert(projectEuler_001_range_for_loop_indices(10) == 23, "");
static_assert(projectEuler_001_range_for_loop_indices(1000) == 233168, "");

static_assert(projectEuler_001_closed_form(10) == 23, "");
static_assert(projectEuler_001_closed_form(1000) == 233168, "");

TEST_CASE("ProjectEuler 001")
{
    CHECK(projectEuler_001_cursor_algorithms(10) == 23);
    CHECK(projectEuler_001_cursor_algorithms(1000) == 233168);

    CHECK(projectEuler_001_cursor_algorithms_filter(10) == 23);
    CHECK(projectEuler_001_cursor_algorithms_filter(1000) == 233168);

    CHECK(projectEuler_001_smart_output_cursor(10) == 23);
    CHECK(projectEuler_001_smart_output_cursor(1000) == 233168);
}
