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
#include <saga/functional.hpp>

// Инфраструктура тестирования
#include "./saga_test.hpp"
#include <catch/catch.hpp>

// Вспомогательные возможноти

// Тесты
namespace
{
    template <class Unsigned, class IncrementOp>
    void test_increment(Unsigned num)
    {
        static_assert(std::is_unsigned<Unsigned>{}, "Must be unsigned");

        auto fun = IncrementOp{};

        auto const num_old = num;

        auto const & result = fun(num);

        REQUIRE(num == num_old + 1);

        REQUIRE(std::addressof(result) == std::addressof(num));
        static_assert(std::is_same<decltype(fun(num)), Unsigned &>{}, "");
    }

    template <class Unsigned, class IncrementOp>
    void test_decrement(Unsigned num)
    {
        static_assert(std::is_unsigned<Unsigned>{}, "Must be unsigned");

        auto fun = IncrementOp{};

        auto const num_old = num;

        auto const & result = fun(num);

        REQUIRE(num == num_old - 1);

        REQUIRE(std::addressof(result) == std::addressof(num));
        static_assert(std::is_same<decltype(fun(num)), Unsigned &>{}, "");
    }
}
TEST_CASE("increment and decrement functional objects")
{
    // Выбран беззнаковый тип, так как знаковых типов возможно переполнение
    saga_test::property_checker
        << ::test_increment<unsigned, saga::increment<unsigned>>
        << ::test_increment<unsigned, saga::increment<>>
        << ::test_decrement<unsigned, saga::decrement<unsigned>>
        << ::test_decrement<unsigned, saga::decrement<>>;
}
