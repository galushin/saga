/* (c) 2020 Галушин Павел Викторович, galushin@gmail.com

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

#include <saga/optimization/ga.hpp>

#include <catch/catch.hpp>

#include "../random_engine.hpp"
#include <valarray>

TEST_CASE("GA pseudoboolean : initializing population")
{
    auto const dim = 20;

    auto const population_size = 100;

    using Genotype = std::valarray<bool>;

    for(auto n = 0; n < 2*population_size; n += population_size / 3)
    {
        std::vector<Genotype> population(n);

        saga::ga_boolen_initialize_population(population, population_size, dim,
                                              saga_test::random_engine());

        REQUIRE(population.size() == population_size);

        for(auto const & each : population)
        {
            REQUIRE(each.size() == dim);
        }
    }
}
