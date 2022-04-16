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
#include <saga/math.hpp>

// Тестовая инфраструктура
#include "./saga_test.hpp"
#include <catch/catch.hpp>

// Тесты
TEST_CASE("square, default operation")
{
    using Value = unsigned;

    static_assert(saga::square(-2) == 4, "");
    static_assert(saga::square(2) == 4, "");

    saga_test::property_checker << [](Value const & value)
    {
        REQUIRE(saga::square(value) == value * value);
    };
}

TEST_CASE("square, custom operation")
{
    using Value = unsigned;

    static_assert(saga::square(-2, std::plus<>{}) == -4, "");
    static_assert(saga::square(2, std::plus<>{}) == 4, "");

    saga_test::property_checker << [](Value const & value)
    {
        REQUIRE(saga::square(value, std::plus<>{}) == value + value);
    };
}

TEST_CASE("power_natural: 1 is id")
{
    using Value = int;

    saga_test::property_checker << [](Value const & value)
    {
        REQUIRE(saga::power_natural(value, 1) == value);
    };
}
