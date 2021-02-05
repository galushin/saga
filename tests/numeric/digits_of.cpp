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
TEST_CASE("digits_of")
{
    using Integer = int;
    saga_test::property_checker << [](Integer num)
    {
        // @todo Механизм задания того, что Integer должен быть положительным
        if(num < 0)
        {
            return;
        }

        auto const base = saga_test::random_uniform<Integer>(2, 100);

        // @todo Заменить на алгоритм copy или механизм преобразования курсора в контейнер
        std::vector<int> digits;

        for(auto cur = saga::cursor::digits_of(num, base); !!cur; ++ cur)
        {
            digits.push_back(*cur);
        }

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

TEST_CASE("digits_of: default value")
{
    using Integer = int;
    saga_test::property_checker << [](Integer num)
    {
        // @todo Механизм задания того, что Integer должен быть положительным
        if(num < 0)
        {
            return;
        }

        auto cur = saga::cursor::digits_of(num);
        auto cur_10 = saga::cursor::digits_of(num, 10);

        // Проверка
        // @todo Заменить на алгоритм
        for(; !!cur && !!cur_10; ++ cur, ++ cur_10)
        {
            REQUIRE(*cur == *cur_10);
        }

        REQUIRE(!cur);
        REQUIRE(!cur_10);
    };
}

TEST_CASE("digits_cursor: default value")
{
    using Integer = int;
    saga_test::property_checker << [](Integer num)
    {
        // @todo Механизм задания того, что Integer должен быть положительным
        if(num < 0)
        {
            return;
        }

        auto cur = saga::digits_cursor<Integer>(num);
        auto cur_10 = saga::cursor::digits_of(num, 10);

        // Проверка
        // @todo Заменить на алгоритм
        for(; !!cur && !!cur_10; ++ cur, ++ cur_10)
        {
            REQUIRE(*cur == *cur_10);
        }

        REQUIRE(!cur);
        REQUIRE(!cur_10);
    };
}
