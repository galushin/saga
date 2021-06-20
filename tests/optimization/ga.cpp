/* (c) 2020-2021 Галушин Павел Викторович, galushin@gmail.com

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
#include <saga/optimization/ga.hpp>

//Инфраструктура тестирования
#include <catch/catch.hpp>
#include "../saga_test.hpp"

// Используемое при тестах
#include <saga/cpp20/span.hpp>
#include <saga/numeric/digits_of.hpp>
#include <saga/optimization/test_objectives.hpp>
#include <saga/view/indices.hpp>

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
    for(auto tournament : saga::view::indices(2, 5))
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

TEST_CASE("tournament_selection_distribution : equal to copy")
{
    saga_test::property_checker << [](bool repeat, std::vector<double> const & obj_values)
    {
        if(obj_values.empty())
        {
            return;
        }

        bool(*cmp)(double const & x, double const & y)
            = [](double const & x, double const & y) { return x < y; };

        auto const tournament_size = saga_test::random_uniform(1, obj_values.size());

        saga::selection_tournament selection(tournament_size, repeat);

        auto distr1 = selection.build_distribution(obj_values, cmp);
        auto distr2 = distr1;

        REQUIRE(distr2 == distr1);
    };
}

TEST_CASE("binary code")
{
    using NotNegativeInteger = unsigned;

    saga_test::property_checker << [](NotNegativeInteger value)
    {
        auto const base = 2;

        std::vector<bool> bits;

        for(auto num = value; num > 0; num /= base)
        {
            bits.push_back(num % base);
        }

        saga::reverse(saga::cursor::all(bits));

        auto const result
            = saga::binary_sequence_to_integer<NotNegativeInteger>(saga::cursor::all(bits));

        REQUIRE(result == value);
    };
}

TEST_CASE("Gray code - empty sequence")
{
    std::vector<bool> code;

    auto const value = saga::gray_code_to_integer<int>(saga::cursor::all(code));

    REQUIRE(value == 0);
}

TEST_CASE("Gray code - generates all")
{
    using Integer = int;
    auto const dim = 20;
    auto const n_max = 1 << 10;

    assert(n_max > 0);

    using Digit_container = std::vector<bool>;

    std::vector<Digit_container> codes(n_max);
    std::vector<Integer> values;

    // @todo заменить на алгоритм
    for(auto num : saga::view::indices(n_max))
    {
        // Преобразуем целое в двоичный код
        Digit_container code;
        saga::copy(saga::cursor::digits_of(num, 2), saga::back_inserter(code));
        saga::reverse(saga::cursor::all(code));

        // Переводим Код грея в целое число
        auto const value = saga::gray_code_to_integer<Integer>(saga::cursor::all(code));

        values.push_back(value);

        // Проверка вхождения значения в интервал
        REQUIRE(0 <= value);
        REQUIRE(value < n_max);

        codes.at(value) = code;
    }

    // Все коды должны быть разные
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());

    REQUIRE(values.size() == n_max);
    REQUIRE(values.front() == 0);
    REQUIRE(values.back() == n_max-1);

    // Дополняем ведущими нулями слишком короткие коды
    for(auto & code : codes)
    {
        REQUIRE(code.size() <= dim);

        code.resize(dim, 0);
    }

    // Соседним значениям должны соответствовать соседние коды, отличающиеся одним битом
    for(auto i = 0* codes.size(); i+1 != codes.size(); ++ i)
    {
        auto const d = saga::boolean_manhattan_distance(codes[i], codes[i+1]);

        REQUIRE(d == 1);
    }
}
