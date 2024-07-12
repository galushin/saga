/* (c) 2021-2024 Галушин Павел Викторович, galushin@gmail.com

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
#include <catch2/catch_amalgamated.hpp>

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

TEST_CASE("power_natural: regression 1138")
{
    REQUIRE(saga::power_natural(3, 5, std::plus<>{}) == 3 * 5);
}

TEST_CASE("is_square: of square")
{
    using Value = std::uint32_t;

    saga_test::property_checker << [](Value const & number)
    {
        CAPTURE(number);

        REQUIRE(saga::is_square(saga::square(static_cast<std::uint64_t>(number))));
    };
}

TEST_CASE("is_square")
{
    using Value = unsigned long;

    saga_test::property_checker << [](Value const & number)
    {
        REQUIRE(saga::is_square(number)
                == (saga::square(static_cast<Value>(std::sqrt(number))) == number));
    };
}

TEST_CASE("is_even and is_odd")
{
    using Value = long;

    saga_test::property_checker << [](Value const & number)
    {
        CAPTURE(number);
        REQUIRE(saga::is_even(number) == (number % 2 == 0));
        REQUIRE(saga::is_odd(number) == (number % 2 != 0));
    };

    static_assert(saga::is_even(4));
    static_assert(!saga::is_even(17));

    static_assert(!saga::is_odd(4));
    static_assert(saga::is_odd(17));
}

TEST_CASE("is_divisible_by")
{
    using Value1 = std::int64_t;
    using Value2 = std::uint32_t;

    static_assert(sizeof(Value1) > sizeof(Value2));

    saga_test::property_checker << [](Value1 const & lhs, Value2 const & rhs)
    {
        if(rhs != 0)
        {
            CAPTURE(lhs, rhs);
            REQUIRE(saga::is_divisible_by(lhs, rhs) == (lhs % rhs == 0));

            auto const pred = saga::is_divisible_by(rhs);

            REQUIRE(pred(lhs) == saga::is_divisible_by(lhs, rhs));
        }
    };

    static_assert(saga::is_divisible_by(4, 2));
    static_assert(!saga::is_divisible_by(17, 3));
}
