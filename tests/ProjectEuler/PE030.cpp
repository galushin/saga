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
#include <catch/catch.hpp>

// Используемые возможности
#include <saga/cursor/filter.hpp>
#include <saga/cursor/indices.hpp>
#include <saga/cursor/transform.hpp>
#include <saga/math.hpp>
#include <saga/numeric.hpp>
#include <saga/numeric/digits_of.hpp>

// PE 030: Суммы степеней цифр
namespace
{
    template <class IntType>
    IntType digits_powers_sum(IntType num, IntType power)
    {
        auto fun = [=](IntType arg) { return saga::power_natural(arg, power); };

        return saga::reduce(saga::cursor::transform(saga::cursor::digits_of(num), fun));
    }

    template <class IntType>
    IntType PE_030(IntType power)
    {
        auto cur = saga::cursor::indices(10, (power + 1) * saga::power_natural(9, power));

        auto pred = [=](IntType num) { return num == ::digits_powers_sum(num, power); };

        return saga::reduce(saga::cursor::filter(std::move(cur), pred));
    }
}

TEST_CASE("PE 030")
{
    REQUIRE(::digits_powers_sum(1634, 4) == 1634);
    REQUIRE(::digits_powers_sum(8208, 4) == 8208);
    REQUIRE(::digits_powers_sum(9474, 4) == 9474);

    REQUIRE(::PE_030(4) == 19316);
    REQUIRE(::PE_030(5) == 443839);
}
