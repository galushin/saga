/* (c) 2020-2021 Галушин Павел Викторович, galushin@gmail.com

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
#include <saga/numeric/polynomial.hpp>

// Тестовая инфраструктура
#include "./../saga_test.hpp"
#include <catch/catch.hpp>

// Вспомогательные файлы
#include <saga/cursor/subrange.hpp>

// Тесты
TEST_CASE("polynomial_horner for binary code")
{
    using NotNegativeInteger = unsigned;

    saga_test::property_checker << [](NotNegativeInteger value)
    {
        auto const base = 2;

        std::vector<bool> bits;

        for(auto num = value; num > 0; num /= base)
        {
            bits.push_back(num % base);
        }

        std::reverse(bits.begin(), bits.end());

        auto const result
            = saga::polynomial_horner(saga::cursor::all(bits), base, NotNegativeInteger(0));

        REQUIRE(result == value);
    };
}
