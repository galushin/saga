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
#include <saga/numeric/digits_of.hpp>

//Инфраструктура тестирования
#include <catch/catch.hpp>
#include "../saga_test.hpp"

// Используемое при тестах
#include <saga/numeric/polynomial.hpp>
#include <saga/cursor/subrange.hpp>

// Тесты
// @todo Что функция должна делать, если ей передать отрицательное число?
// @todo Что функция должна делать, base < 2?
// @todo Тест значения по умолчанию для digits_of и digits_cursor
TEST_CASE("digits_of")
{
    using Integer = int;
    saga_test::property_checker << [](Integer num)
    {
        // @todo Механизм игнорирования или задания того, что Integer должен быть положительным
        if(num < 0)
        {
            return;
        }

        auto const base = saga_test::random_uniform<Integer>(2, 100);

        std::vector<int> digits;

        for(auto cur = saga::cursor::digits_of(num, base); !!cur; ++ cur)
        {
            digits.push_back(*cur);
        }

        // @todo ural::reverse(digits)
        std::reverse(digits.begin(), digits.end());

        // Цифры в заданном диапазоне
        for(auto const & digit : digits)
        {
            REQUIRE(0 <= digit);
            REQUIRE(digit < base);
        }

        // Обратное преобразование приводит к исходному числу
        auto const ans = saga::polynomial_horner(saga::cursor::all(digits), base, Integer(0));
        REQUIRE(ans == num);

    };
}

// @todo digits_of - основание по умолчанию - 10;
