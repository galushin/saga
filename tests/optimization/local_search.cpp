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

template <class Argument, class Objective, class RealType>
void test_local_search_boolean(Objective const & objective, typename Argument::size_type dim,
                               RealType desired_value, RealType eps = 0)
{
    saga::iid_distribution<std::bernoulli_distribution> init_distr(dim);

    for(auto N = 100; N > 0; -- N)
    {
        auto const x_init = init_distr(saga_test::random_engine());
        auto const result = saga::local_search_boolean(objective, x_init);

        CAPTURE(result.solution);
        CAPTURE(result.objective_value);

        REQUIRE(result.objective_value == objective(result.solution));

        REQUIRE(result.objective_value == Approx(desired_value).epsilon(eps));
    }
}

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

// @todo Превратить в тест, основанный на свойстах
TEST_CASE("local search (pseudoboolen, first improvement) : L1 norm")
{
    using Argument = std::vector<bool>;
    auto const objective = saga::boolean_manhattan_norm;

    auto const dim = 20;
    test_local_search_boolean<Argument>(objective, dim, 0*dim);
}

// @todo Превратить в тест, основанный на свойстах
TEST_CASE("local search (pseudoboolen, first improvement) : L1 distance to some vector")
{
    using Argument = std::vector<bool>;

    auto const dim = 20;
    saga::iid_distribution<std::bernoulli_distribution> init_distr(dim);

    auto const x_opt = init_distr(saga_test::random_engine());
    auto const objective = [&x_opt](Argument const & arg)
    {
        return saga::boolean_manhattan_distance(arg, x_opt);
    };

    test_local_search_boolean<Argument>(objective, dim, 0*dim);
}

// @todo Превратить в тест, основанный на свойстах
TEST_CASE("local search (pseudoboolean, first improvement): number of unequal adjacent bits")
{
    auto const objective = saga::count_adjacent_unequal;

    auto const dim = 20;

    saga::iid_distribution<std::bernoulli_distribution> init_distr(dim);

    for(auto N = 100; N > 0; -- N)
    {
        auto const x_init = init_distr(saga_test::random_engine());
        auto const result = saga::local_search_boolean(objective, x_init, std::greater<>{});

        CAPTURE(result.solution);
        CAPTURE(result.objective_value);

        REQUIRE(is_local_maximum(objective, result.solution));
    }
}

// @todo Превратить в тест, основанный на свойстах
TEST_CASE("local search (pseudoboolean, first improvement): number of unequal adjacent bits, mask")
{
    using Argument = std::vector<bool>;

    auto const dim = 20;

    saga::iid_distribution<std::bernoulli_distribution> mask_distr(dim);
    auto const mask = mask_distr(saga_test::random_engine());

    // @todo Автоматизировать создание маски
    auto const objective = [&](Argument const & arg)
    {
        assert(arg.size() == mask.size());
        auto x = arg;

        for(auto i = 0*arg.size(); i+1 != arg.size(); ++ i)
        {
            x[i] = x[i] ^ mask[i];
        }

        return saga::count_adjacent_unequal(x);
    };

    saga::iid_distribution<std::bernoulli_distribution> init_distr(dim);

    for(auto N = 100; N > 0; -- N)
    {
        auto const x_init = init_distr(saga_test::random_engine());
        auto const result = saga::local_search_boolean(objective, x_init, std::greater<>{});

        CAPTURE(result.solution);
        CAPTURE(result.objective_value);

        REQUIRE(is_local_maximum(objective, result.solution));
    }
}
