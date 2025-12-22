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
#include <saga/numeric.hpp>
#include <saga/numeric/primes_cursor.hpp>

// Инфраструктура тестирования
#include <catch2/catch_amalgamated.hpp>

// PE 069 - Максимум функции Эйлера
namespace
{
    template <class Quotient, class IntType>
    IntType PE_069_scan(IntType const n_max)
    {
        auto const phi = saga::euler_phi_below(n_max);
        assert(phi.size() == static_cast<size_t>(n_max));
        assert(phi[0] == 0);

        std::vector<Quotient> n_over_phi;

        saga::transform(saga::cursor::indices(IntType(1), n_max)
                       ,saga::cursor::drop_front_n(saga::cursor::all(phi), 1)
                       ,saga::back_inserter(n_over_phi)
                       ,std::divides<Quotient>{});

        return saga::max_element(saga::cursor::all(n_over_phi)).dropped_front().size() + 1;
    }

    template <class IntType>
    IntType PE_069_smart(IntType const n_max)
    {
        auto cur = saga::primes_cursor<IntType>();

        auto result = IntType(1);

        for(;; ++ cur)
        {
            auto new_result = result * cur->back();

            if(new_result > n_max)
            {
                break;
            }

            result = std::move(new_result);
        }

        return result;
    }
}

TEST_CASE("PE 069")
{
    REQUIRE(PE_069_scan<double>(10) == 6);
    REQUIRE(PE_069_scan<double>(1'000'000) == 510'510);

    REQUIRE(PE_069_smart(10) == 6);
    REQUIRE(PE_069_smart(1'000'000) == 510'510);
}
