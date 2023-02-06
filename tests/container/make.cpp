/* (c) 2023 Галушин Павел Викторович, galushin@gmail.com

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
#include <saga/container/make.hpp>

// Инфраструктура тестирования
#include "../saga_test.hpp"
#include <catch2/catch_amalgamated.hpp>

// Используемые в тестах возможности
#include <list>

// Тесты
TEST_CASE("make_container : non-trivial")
{
    saga_test::property_checker
    << [](std::string const & arg1, std::string const & arg2, std::string const & arg3)
    {
        std::list<std::string> const expected{arg1, arg2, arg3};

        auto const actual = saga::make_container<std::list>(arg1, arg2, arg3);

        REQUIRE(actual == expected);
    };
}
