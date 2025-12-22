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

// Инфраструктура тестирования
#include "../saga_test.hpp"
#include <catch2/catch_amalgamated.hpp>

// PE 070 - Перестановка функции Эйлера
/* Чтобы n/phi(n) было как можно меньше, phi(p) должно быть как можно больше.
Чем больше простых множителей, тем меньше phi(p).
Для p простого phi(p)=p-1 не может быть перестановкой
Рассмотрим всевозможные пары
*/
TEST_CASE("PE 070")
{
    auto const n_max = 10'000'000;

    auto const primes = saga::primes_below(n_max / 2 + 1);

    auto best_num = 2;
    auto best_phi = 1;

    for(auto pos1 : saga::cursor::indices_of(primes))
    for(auto pos2 : saga::cursor::indices(0, pos1))
    {
        if(primes[pos2] > n_max / primes[pos1])
        {
            break;
        }

        auto const num = primes[pos1] * primes[pos2];
        auto const phi = (primes[pos1] - 1) * (primes[pos2] - 1);

        auto const num_s = std::to_string(num);
        auto const phi_s = std::to_string(phi);

        if(saga::is_permutation(saga::cursor::all(num_s), saga::cursor::all(phi_s))
           && (double(num) / phi < double(best_num) / best_phi))
        {
            best_num = num;
            best_phi = phi;
        }
    }

    REQUIRE(best_num == 8319823);
}
