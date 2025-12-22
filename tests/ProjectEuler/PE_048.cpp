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
#include "./integer10.hpp"

#include <saga/cursor/transform.hpp>

// Инфраструктура тестирования
#include <catch2/catch_amalgamated.hpp>

// PE 048 - Собственные степени
namespace
{
    template <class Integer, class IntType
             , class BinaryOperation1 = std::multiplies<>
             , class BinaryOperation2 = std::plus<>>
    Integer PE_048_self_powers_sum(IntType max_num, BinaryOperation1 prod = {}
                                  , BinaryOperation2 add = {})
    {
        auto self_power = [&](IntType const & num)
        {
            assert(num >= 1);

            return saga::power_natural(Integer(num), num, prod);
        };

        auto cur = saga::cursor::indices(1, max_num + 1) | saga::cursor::transform(self_power);

        return saga::reduce(std::move(cur), {}, add);
    }
}

TEST_CASE("integer10 : mod10")
{
    saga::experimental::integer10 num("987654321");
    num.mod10(3);

    REQUIRE(num == 321);
}

TEST_CASE("PE 048")
{
    // Простой пример
    REQUIRE(::PE_048_self_powers_sum<long long>(10) == 10'405'071'317);

    // Пример, для которого недостаточно 64 бита
    using Integer = saga::experimental::integer10;

    auto const digits_needed = 10;

    auto const mult_mod = [=](Integer lhs, Integer const & rhs)
    {
        lhs *= rhs;
        lhs.mod10(digits_needed);

        return lhs;
    };

    auto const add_mod = [=](Integer lhs, Integer const & rhs)
    {
        lhs += rhs;
        lhs.mod10(digits_needed);

        return lhs;
    };

    REQUIRE(::PE_048_self_powers_sum<Integer>(1000, mult_mod, add_mod) == 9'110'846'700);
}
