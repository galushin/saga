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
#include <saga/utility/exchange.hpp>

// Инфраструктура тестирования
#include "../saga_test.hpp"
#include <catch2/catch_amalgamated.hpp>

// Используемые в тестах
#include <vector>

// Проверки
static_assert(noexcept(saga::exchange(std::declval<int&>(), std::declval<int>())));

namespace
{
    template <bool noexcept_move_ctor, bool noexcept_move_assign>
    struct Helper
    {
        Helper(Helper &&) noexcept(noexcept_move_ctor);

        void operator=(int) noexcept(noexcept_move_assign);
    };
}

static_assert(noexcept(saga::exchange(std::declval<Helper<true, true>&>(), std::declval<int>())));
static_assert(!noexcept(saga::exchange(std::declval<Helper<true, false>&>(), std::declval<int>())));
static_assert(!noexcept(saga::exchange(std::declval<Helper<false, true>&>(), std::declval<int>())));
static_assert(!noexcept(saga::exchange(std::declval<Helper<false, false>&>(), std::declval<int>())));

TEST_CASE("exhange")
{
    using Value = long;

    saga_test::property_checker << [](Value const & old_value, Value const & new_value)
    {
        auto obj = old_value;

        auto const result = saga::exchange(obj, new_value);

        REQUIRE(result == old_value);
        REQUIRE(obj == new_value);
    };
}
