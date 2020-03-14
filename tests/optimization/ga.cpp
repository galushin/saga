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

#include <saga/cpp20/span.hpp>
#include <saga/optimization/test_objectives.hpp>

TEST_CASE("GA boolean : minimize manhattan distance, uniform crossover")
{
    auto const dim = 20;
    using Argument = std::valarray<bool>;

    for(auto T = 50; T > 0; -- T)
    {
        saga::iid_distribution<std::bernoulli_distribution, Argument> init_distr(dim);

        auto const x_opt = init_distr(saga_test::random_engine());
        auto const objective = [&x_opt](Argument const & arg)
        {
            return arg.size() - saga::boolean_manhattan_distance(arg, x_opt);
        };

        auto const problem = saga::make_optimization_problem_boolean(objective, dim);

        using Genotype = std::valarray<bool>;
        saga::GA_settings<Genotype, saga::ga_boolean_crossover_uniform_fn> settings;
        settings.population_size = 200;
        settings.max_iterations = 200;

        auto const population
            = saga::genetic_algorithm(problem, settings, saga_test::random_engine());

        std::vector<double> obj_values;
        obj_values.reserve(settings.population_size);

        std::transform(saga::begin(population), saga::end(population),
                       std::back_inserter(obj_values), objective);

        auto const best = std::max_element(obj_values.begin(), obj_values.end())
                        - obj_values.begin();

        CAPTURE(x_opt);
        CAPTURE(population[best]);
        CAPTURE(obj_values[best]);

        REQUIRE(saga::equal(saga::cursor::all(population[best]), saga::cursor::all(x_opt)));
    }
}

TEST_CASE("GA boolean : minimize manhattan distance, one point crossover")
{
    auto const dim = 20;
    using Argument = std::valarray<bool>;

    for(auto T = 50; T > 0; -- T)
    {
        saga::iid_distribution<std::bernoulli_distribution, Argument> init_distr(dim);

        auto const x_opt = init_distr(saga_test::random_engine());
        auto const objective = [&x_opt](Argument const & arg)
        {
            return arg.size() - saga::boolean_manhattan_distance(arg, x_opt);
        };

        auto const problem = saga::make_optimization_problem_boolean(objective, dim);

        using Genotype = std::valarray<bool>;
        saga::GA_settings<Genotype, saga::ga_boolean_crossover_one_point_fn> settings;
        settings.population_size = 200;
        settings.max_iterations = 200;

        auto const population
            = saga::genetic_algorithm(problem, settings, saga_test::random_engine());

        std::vector<double> obj_values;
        obj_values.reserve(settings.population_size);

        std::transform(saga::begin(population), saga::end(population),
                       std::back_inserter(obj_values), objective);

        auto const best = std::max_element(obj_values.begin(), obj_values.end())
                        - obj_values.begin();

        CAPTURE(x_opt);
        CAPTURE(population[best]);
        CAPTURE(obj_values[best]);

        REQUIRE(saga::equal(saga::cursor::all(population[best]), saga::cursor::all(x_opt)));
    }
}

TEST_CASE("GA boolean : minimize manhattan distance, two point crossover")
{
    auto const dim = 20;
    using Argument = std::valarray<bool>;

    for(auto T = 50; T > 0; -- T)
    {
        saga::iid_distribution<std::bernoulli_distribution, Argument> init_distr(dim);

        auto const x_opt = init_distr(saga_test::random_engine());
        auto const objective = [&x_opt](Argument const & arg)
        {
            return arg.size() - saga::boolean_manhattan_distance(arg, x_opt);
        };

        auto const problem = saga::make_optimization_problem_boolean(objective, dim);

        using Genotype = std::valarray<bool>;
        saga::GA_settings<Genotype, saga::ga_boolean_crossover_two_point_fn> settings;
        settings.population_size = 200;
        settings.max_iterations = 200;

        auto const population
            = saga::genetic_algorithm(problem, settings, saga_test::random_engine());

        std::vector<double> obj_values;
        obj_values.reserve(settings.population_size);

        std::transform(saga::begin(population), saga::end(population),
                       std::back_inserter(obj_values), objective);

        auto const best = std::max_element(obj_values.begin(), obj_values.end())
                        - obj_values.begin();

        CAPTURE(x_opt);
        CAPTURE(population[best]);
        CAPTURE(obj_values[best]);

        REQUIRE(saga::equal(saga::cursor::all(population[best]), saga::cursor::all(x_opt)));
    }
}
