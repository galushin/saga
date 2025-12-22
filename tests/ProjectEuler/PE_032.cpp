/* (c) 2022-2025 Галушин Павел Викторович, galushin@gmail.com

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

// Используемые возможности
#include <saga/flat_set.hpp>
#include <saga/numeric.hpp>

// Инфраструктура тестирования
#include <catch2/catch_amalgamated.hpp>

// PE 032: Панцифирные произведения
TEST_CASE("PE 032")
{
    using IntType = long;
    auto const min_mult = saga::power_natural(IntType(10), 2);
    auto const max_mult = saga::power_natural(IntType(10), 4);

    saga::flat_set<IntType> obj;

    for(auto lhs : saga::cursor::indices(min_mult, max_mult))
    {
        auto const lhs_str = std::to_string(lhs);

        for(auto rhs : saga::cursor::indices(1, lhs))
        {
            auto rhs_str = std::to_string(rhs);

            auto prod = lhs * rhs;

            auto str = lhs_str + rhs_str + std::to_string(prod);

            if(str.size() > 9)
            {
                break;
            }

            str |= saga::action::sort;

            if(str == "123456789")
            {
                // @todo Убрать первый аргумент?
                obj.insert(obj.end(), std::move(prod));
            }
        }
    }

    REQUIRE(saga::reduce(saga::cursor::all(obj)) == 45228);
}
