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
#include "saga_test.hpp"

// Тестовая инфраструктура
#include <catch/catch.hpp>

// Вспомогательные файлы

// Тесты
TEST_CASE("saga_test::detail::function_input_iterator: operator++ without value")
{
    int value = 0;

    auto gen = [&]() { return value++; };

    auto iter = saga_test::detail::make_function_input_iterator(gen, 0);

    ++ iter;

    REQUIRE(*iter == 1);
}

TEST_CASE("saga_test::bounded - positive")
{
    constexpr auto const x_min = int(13);
    constexpr auto const x_max = int(42);

    using Bounded = saga_test::bounded<int, x_min, x_max>;

    static_assert(Bounded::min() == x_min, "");
    static_assert(Bounded::max() == x_max, "");

    saga_test::property_checker << [](Bounded const & value)
    {
        REQUIRE(Bounded::min() <= value);
        REQUIRE(value <= Bounded::max());
    };

    REQUIRE_THROWS_AS(Bounded(x_max + 1), std::out_of_range);
    REQUIRE_THROWS_AS(Bounded(x_min - 20), std::out_of_range);
}
