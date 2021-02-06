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
#include <saga/algorithm.hpp>
#include <saga/numeric/polynomial.hpp>
#include <saga/cursor/subrange.hpp>

// Тесты
TEST_CASE("digits_of")
{
    using NotNegativeInteger = unsigned;

    saga_test::property_checker << [](NotNegativeInteger const & value)
    {
        auto const base = saga_test::random_uniform<NotNegativeInteger>(2, 100);

        // Явные вычисления
        std::vector<NotNegativeInteger> digits_manual;

        for(auto num = value; num > 0; num /= base)
        {
            digits_manual.push_back(num % base);
        }

        // Курсов digits_of
        std::vector<NotNegativeInteger> digits_cursor;

        for(auto cur = saga::cursor::digits_of(value, base); !!cur; ++ cur)
        {
            digits_cursor.push_back(*cur);
        }

        // Сравнение
        REQUIRE(digits_manual == digits_cursor);
    };
}

TEST_CASE("digits_of - reverse of poynomial_horner")
{
    using NotNegativeInteger = unsigned;

    saga_test::property_checker << [](NotNegativeInteger const & value)
    {
        auto const base = saga_test::random_uniform<NotNegativeInteger>(2, 100);

        // @todo Заменить на алгоритм copy или механизм преобразования курсора в контейнер
        std::vector<int> digits;

        for(auto cur = saga::cursor::digits_of(value, base); !!cur; ++ cur)
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
        auto const ans
            = saga::polynomial_horner(saga::cursor::all(digits), base, NotNegativeInteger(0));

        REQUIRE(ans == value);

    };
}

TEST_CASE("digits_of: default value")
{
    using NotNegativeInteger = unsigned;

    saga_test::property_checker << [](NotNegativeInteger const & value)
    {
        auto cur = saga::cursor::digits_of(value);
        auto cur_10 = saga::cursor::digits_of(value, 10u);

        REQUIRE(saga::equal(cur, cur_10));
    };
}

TEST_CASE("digits_cursor: default value")
{
    using NotNegativeInteger = unsigned;
    saga_test::property_checker << [](NotNegativeInteger const & value)
    {
        auto cur = saga::digits_cursor<NotNegativeInteger>(value);
        auto cur_10 = saga::cursor::digits_of(value, 10u);

        REQUIRE(saga::equal(cur, cur_10));
    };
}
