/* (c) 2019 Галушин Павел Викторович, galushin@gmail.com

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
#include <saga/optimization/local_search.hpp>

// Инфраструктура тестирования
#include "../saga_test.hpp"
#include <catch/catch.hpp>

// Вспомогательные возможноти
#include <saga/optimization/test_objectives.hpp>
#include <saga/random/iid_distribution.hpp>

template <class Objective, class Argument>
bool is_local_maximum(Objective const & objective, Argument const & arg)
{
    auto const y_arg = objective(arg);
    auto x = arg;

    for(auto i = x.begin(); i != x.end(); ++ i)
    {
        *i = !*i;

        auto const y = objective(x);

        if(y > y_arg)
        {
            return false;
        }

        *i = !*i;
    }

    return true;
}

TEST_CASE("local search (pseudoboolen, first improvement) : L1 norm")
{
    saga_test::property_checker << [](std::vector<bool> const & x_init)
    {
        auto const objective = saga::boolean_manhattan_norm;

        auto const result = saga::local_search_boolean(objective, x_init);

        CAPTURE(result.solution);
        CAPTURE(result.objective_value);

        REQUIRE(result.objective_value == objective(result.solution));
        REQUIRE(result.objective_value == 0);
    };
}

TEST_CASE("local search (pseudoboolen, first improvement) : L1 distance to some vector")
{
    using Argument = std::vector<bool>;

    saga_test::property_checker << [](Argument x_opt, Argument x_init)
    {
        auto const dim = std::min(x_opt.size(), x_init.size());
        x_opt.resize(dim);
        x_init.resize(dim);

        auto const objective = [&x_opt](Argument const & arg)
        {
            return saga::boolean_manhattan_distance(arg, x_opt);
        };

        auto const result = saga::local_search_boolean(objective, x_init);

        CAPTURE(result.solution);
        CAPTURE(result.objective_value);

        REQUIRE(result.objective_value == objective(result.solution));
        REQUIRE(result.objective_value == 0);
    };
}

TEST_CASE("local search (pseudoboolean, first improvement): number of unequal adjacent bits")
{
    saga_test::property_checker << [](std::vector<bool> const & x_init)
    {
        auto const objective = saga::count_adjacent_unequal;

        auto const result = saga::local_search_boolean(objective, x_init, std::greater<>{});

        CAPTURE(result.solution);
        CAPTURE(result.objective_value);

        REQUIRE(is_local_maximum(objective, result.solution));
    };
}

TEST_CASE("local search (pseudoboolean, first improvement): number of unequal adjacent bits, mask")
{
    using Argument = std::vector<bool>;

    saga_test::property_checker << [](Argument x_init, Argument mask)
    {
        auto const dim = std::min(x_init.size(), mask.size());
        x_init.resize(dim);
        mask.resize(dim);

        auto const objective = [&](Argument const & arg)
        {
            assert(arg.size() == mask.size());
            auto x = arg;

            for(auto i = 0*arg.size(); i+1 < arg.size(); ++ i)
            {
                x[i] = x[i] ^ mask[i];
            }

            return saga::count_adjacent_unequal(x);
        };

        auto const result = saga::local_search_boolean(objective, x_init, std::greater<>{});

        CAPTURE(result.solution);
        CAPTURE(result.objective_value);

        REQUIRE(is_local_maximum(objective, result.solution));
    };
}
