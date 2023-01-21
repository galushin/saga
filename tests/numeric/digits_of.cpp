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
#include <catch2/catch_amalgamated.hpp>
#include "../saga_test.hpp"

// Используемое при тестах
#include <saga/actions/reverse.hpp>
#include <saga/algorithm.hpp>
#include <saga/cursor/subrange.hpp>
#include <saga/cursor/to.hpp>
#include <saga/numeric/polynomial.hpp>

// Тесты
TEST_CASE("digits_of")
{
    using NotNegativeInteger = unsigned;

    saga_test::property_checker << [](NotNegativeInteger const & value
                                      , saga_test::bounded<NotNegativeInteger, 2, 100> const & base)
    {
        // Явные вычисления
        std::vector<NotNegativeInteger> digits_manual;

        for(auto num = value; num > 0; num /= base)
        {
            digits_manual.push_back(num % base);
        }

        // Курсов digits_of
        std::vector<NotNegativeInteger> digits_cursor;

        saga::copy(saga::cursor::digits_of(value, base.value())
                   , saga::back_inserter(digits_cursor));

        // Сравнение
        REQUIRE(digits_manual == digits_cursor);
    };
}

TEST_CASE("digits_of - reverse of poynomial_horner")
{
    using NotNegativeInteger = unsigned;

    saga_test::property_checker << [](NotNegativeInteger const & value
                                      , saga_test::bounded<int, 2, 100> const & base)
    {
        auto const digits = saga::cursor::digits_of(value, base.value())
                          | saga::cursor::to<std::vector>()
                          | saga::actions::reverse;

        // Цифры в заданном диапазоне
        REQUIRE(saga::all_of(saga::cursor::all(digits)
                             , [&](int const & digit){ return 0 <= digit && digit < base; }));

        // Обратное преобразование приводит к исходному числу
        auto const ans = saga::polynomial_horner(saga::cursor::all(digits)
                                                 , base, NotNegativeInteger(0));

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
