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

#include <saga/cpp20/span.hpp>
#include <saga/optimization/ga.hpp>
#include <saga/optimization/test_objectives.hpp>

#include <catch/catch.hpp>

#include "../random_engine.hpp"
#include <valarray>

namespace
{
    struct compare_by_objective_value
    {
        template <class Individual>
        bool operator()(Individual const & x, Individual const & y) const
        {
            return x.objective_value < y.objective_value;
        }
    };

    template <class Crossover, class Selection>
    void test_ga_boolean_manhattan_distance_min(Selection selection)
    {
        auto const dim = 20;
        using Argument = std::valarray<bool>;

        saga::iid_distribution<std::bernoulli_distribution, Argument> init_distr(dim);

        auto const x_opt = init_distr(saga_test::random_engine());
        auto const objective = [&x_opt](Argument const & arg) -> double
        {
            return saga::boolean_manhattan_distance(arg, x_opt);
        };

        auto const problem = saga::make_optimization_problem_boolean(objective, dim);

        using Genotype = std::valarray<bool>;
        saga::GA_settings<Genotype, Crossover, Selection> settings;
        settings.population_size = 200;
        settings.max_iterations = 200;
        settings.selection = selection;

        auto const population
            = saga::genetic_algorithm_boolean(problem, settings, saga_test::random_engine());

        auto const best = std::min_element(population.begin(), population.end(),
                                           ::compare_by_objective_value{});

        CAPTURE(x_opt);
        CAPTURE(best->solution);
        CAPTURE(best->objective_value);

        REQUIRE(saga::equal(saga::cursor::all(best->solution), saga::cursor::all(x_opt)));
    }

    template <class Crossover, class Selection>
    void test_ga_boolean_manhattan_distance_max(Selection selection)
    {
        auto const dim = 20;
        using Argument = std::valarray<bool>;

        saga::iid_distribution<std::bernoulli_distribution, Argument> init_distr(dim);

        auto const x_opt = init_distr(saga_test::random_engine());
        auto const objective = [&x_opt](Argument const & arg) -> double
        {
            return arg.size() - saga::boolean_manhattan_distance(arg, x_opt);
        };

        auto const problem
            = saga::make_optimization_problem_boolean(objective, dim, std::greater<>{});

        using Genotype = std::valarray<bool>;
        saga::GA_settings<Genotype, Crossover, Selection> settings;
        settings.population_size = 200;
        settings.max_iterations = 200;
        settings.selection = selection;

        auto const population
            = saga::genetic_algorithm_boolean(problem, settings, saga_test::random_engine());

        auto const best = std::max_element(population.begin(), population.end(),
                                           ::compare_by_objective_value{});

        CAPTURE(x_opt);
        CAPTURE(best->solution);
        CAPTURE(best->objective_value);

        REQUIRE(saga::equal(saga::cursor::all(best->solution), saga::cursor::all(x_opt)));
    }
}

TEST_CASE("GA boolean : manhattan distance, tournament selection")
{
    for(auto tournament = 2; tournament < 5; ++ tournament)
    {
        {
            saga::selection_tournament selection(tournament);

            ::test_ga_boolean_manhattan_distance_min<saga::ga_boolean_crossover_uniform_fn>(selection);
            ::test_ga_boolean_manhattan_distance_min<saga::ga_boolean_crossover_one_point_fn>(selection);
            ::test_ga_boolean_manhattan_distance_min<saga::ga_boolean_crossover_two_point_fn>(selection);

            ::test_ga_boolean_manhattan_distance_max<saga::ga_boolean_crossover_uniform_fn>(selection);
            ::test_ga_boolean_manhattan_distance_max<saga::ga_boolean_crossover_one_point_fn>(selection);
            ::test_ga_boolean_manhattan_distance_max<saga::ga_boolean_crossover_two_point_fn>(selection);
        }

        {
            saga::selection_tournament selection(tournament, false);

            ::test_ga_boolean_manhattan_distance_min<saga::ga_boolean_crossover_uniform_fn>(selection);
            ::test_ga_boolean_manhattan_distance_min<saga::ga_boolean_crossover_one_point_fn>(selection);
            ::test_ga_boolean_manhattan_distance_min<saga::ga_boolean_crossover_two_point_fn>(selection);

            ::test_ga_boolean_manhattan_distance_max<saga::ga_boolean_crossover_uniform_fn>(selection);
            ::test_ga_boolean_manhattan_distance_max<saga::ga_boolean_crossover_one_point_fn>(selection);
            ::test_ga_boolean_manhattan_distance_max<saga::ga_boolean_crossover_two_point_fn>(selection);
        }
    }
}

TEST_CASE("GA boolean : manhattan distance, ranking selection")
{
    saga::selection_ranking selection;

    ::test_ga_boolean_manhattan_distance_min<saga::ga_boolean_crossover_uniform_fn>(selection);
    ::test_ga_boolean_manhattan_distance_min<saga::ga_boolean_crossover_one_point_fn>(selection);
    ::test_ga_boolean_manhattan_distance_min<saga::ga_boolean_crossover_two_point_fn>(selection);

    ::test_ga_boolean_manhattan_distance_max<saga::ga_boolean_crossover_uniform_fn>(selection);
    ::test_ga_boolean_manhattan_distance_max<saga::ga_boolean_crossover_one_point_fn>(selection);
    ::test_ga_boolean_manhattan_distance_max<saga::ga_boolean_crossover_two_point_fn>(selection);
}

TEST_CASE("GA boolean : manhattan distance, proportional selection")
{
    saga::selection_proportional selection;

    ::test_ga_boolean_manhattan_distance_min<saga::ga_boolean_crossover_uniform_fn>(selection);
    ::test_ga_boolean_manhattan_distance_min<saga::ga_boolean_crossover_one_point_fn>(selection);
    ::test_ga_boolean_manhattan_distance_min<saga::ga_boolean_crossover_two_point_fn>(selection);

    ::test_ga_boolean_manhattan_distance_max<saga::ga_boolean_crossover_uniform_fn>(selection);
    ::test_ga_boolean_manhattan_distance_max<saga::ga_boolean_crossover_one_point_fn>(selection);
    ::test_ga_boolean_manhattan_distance_max<saga::ga_boolean_crossover_two_point_fn>(selection);
}

TEST_CASE("GA boolean : manhattan distance, selection tournament - type-erased")
{
    saga::any_selection<saga_test::random_engine_type>
        selection(saga::selection_tournament(2));

    ::test_ga_boolean_manhattan_distance_min<saga::ga_boolean_crossover_uniform_fn>(selection);
    ::test_ga_boolean_manhattan_distance_min<saga::ga_boolean_crossover_one_point_fn>(selection);
    ::test_ga_boolean_manhattan_distance_min<saga::ga_boolean_crossover_two_point_fn>(selection);

    ::test_ga_boolean_manhattan_distance_max<saga::ga_boolean_crossover_uniform_fn>(selection);
    ::test_ga_boolean_manhattan_distance_max<saga::ga_boolean_crossover_one_point_fn>(selection);
    ::test_ga_boolean_manhattan_distance_max<saga::ga_boolean_crossover_two_point_fn>(selection);
}
