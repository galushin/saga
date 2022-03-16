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

#ifndef Z_SAGA_OPTIMIZATION_GA_HPP_INCLUDED
#define Z_SAGA_OPTIMIZATION_GA_HPP_INCLUDED

/** @brief saga/optimization/ga.hpp
 @brief Функциональность, связанная с генетическими алгоритмами
*/

#include <saga/math/probability.hpp>
#include <saga/numeric/polynomial.hpp>
#include <saga/optimization/ga/crossover/ga_boolean_crossover_one_point.hpp>
#include <saga/optimization/ga/crossover/ga_boolean_crossover_two_point.hpp>
#include <saga/optimization/ga/crossover/ga_boolean_crossover_uniform.hpp>
#include <saga/optimization/ga/selection/any_selection.hpp>
#include <saga/optimization/ga/selection/proportional.hpp>
#include <saga/optimization/ga/selection/ranking.hpp>
#include <saga/optimization/ga/selection/tournament.hpp>
#include <saga/optimization/evaluated_solution.hpp>
#include <saga/optimization/optimization_problem.hpp>
#include <saga/random/iid_distribution.hpp>

namespace saga
{
    /** @brief Инициализация начальной популяции генетического алгоритма псведо-булевой оптимизации
    @tparam Container тип контейнера, используемого для хранения популяции
    @param population_size требуемый размер начальной популяции
    @param dim размерность (число генов в генотипе)
    @param objective целевая функция
    @param rnd_engine генератор равномерно распределённых битов, используемый для порождения
    псевдо-случайных чисел
    @return Начальная популяции для генетического алгоритма, созданная согласно заданным параметрам
    */
    template <class Container, class Size, class Objective, class UniformRandomBitGenerator>
    Container
    ga_boolen_initial_population(typename Container::difference_type const population_size
                                 , Size const dim
                                 , Objective objective
                                 , UniformRandomBitGenerator & rnd_engine)
    {
        assert(population_size >= 0);
        assert(dim >= 0);

        auto const pop_size = static_cast<typename Container::size_type>(population_size);

        using Individual = typename Container::value_type;
        using Genotype = typename Individual::solution_type;

        saga::iid_distribution<std::bernoulli_distribution, Genotype> distr(dim);

        auto generator = [&]
        {
            auto x = distr(rnd_engine);
            auto y = objective(x);

            return Individual{std::move(x), std::move(y)};
        };

        Container population;
        population.reserve(pop_size);

        saga::generate(saga::cursor::all(population), generator);

        assert(population.size() <= pop_size);
        std::generate_n(std::back_inserter(population), pop_size - population.size(), generator);

        return population;
    }

    class ga_boolean_mutation_xor_fn
    {
    public:
        template <class Genotype, class UniformRandomBitGenerator>
        void operator()(Genotype & genotype,
                        saga::probability<double> const & p_mutation,
                        UniformRandomBitGenerator & rnd) const
        {
            std::bernoulli_distribution distr(p_mutation.value());

            for(auto & gen : genotype)
            {
                gen ^= distr(rnd);
            }
        }
    };

    template <class Genotype, class Crossover, class Selection,
              class Mutation = saga::ga_boolean_mutation_xor_fn>
    struct GA_settings
    {
    public:
        using genotype_type = Genotype;

        int population_size = 0;
        int max_iterations = 0;
        double mutation_strength = 1.0;
        Crossover crossover{};
        Selection selection{};
        Mutation mutation{};

        template <class Population, class Compare>
        void change_generation(Population & parents, Population & kids, Compare cmp) const
        {
            assert(kids.size() == parents.size());

            using Individual = typename Population::value_type;
            auto const cmp_individ = [&](Individual const & x, Individual const & y)
            {
                return cmp(x.objective_value, y.objective_value);
            };

            auto elite_pos = std::min_element(saga::begin(parents), saga::end(parents), cmp_individ);
            kids.push_back(std::move(*elite_pos));

            auto worst_pos = std::max_element(saga::begin(kids), saga::end(kids), cmp_individ);
            *worst_pos = std::move(kids.back());
            kids.pop_back();

            parents.swap(kids);
        }

        template <class UniformRandomBitGenerator>
        void mutate(Genotype & genotype, UniformRandomBitGenerator & urbg) const
        {
            auto const genes_count = genotype.size();

            auto const avg_mutant_genes = std::min<double>(mutation_strength, genes_count);

            auto const p_mutation = saga::probability<double>{avg_mutant_genes / genes_count};

            this->mutation(genotype, p_mutation, urbg);
        }
    };

    template <class Population, class Problem, class GA_settings, class UniformRandomBitGegerator>
    void genetic_algorithm_boolean_cycle(Population & population,
                                         Problem const & problem,
                                         GA_settings const & settings,
                                         UniformRandomBitGegerator & rnd )
    {
        if(population.empty())
        {
            return;
        }

        using Individual = typename Population::value_type;

        // Построение распределения для селекции
        // @todo Обойтись без копирования
        std::vector<double> obj_values;
        obj_values.reserve(settings.population_size);

        std::transform(population.begin(), population.end(), std::back_inserter(obj_values),
                       std::mem_fn(&Individual::objective_value));

        auto s_distr = settings.selection.build_distribution(obj_values, problem.compare);

        // Репродукция: селекция, скрещивание, нормальная мутация
        std::vector<Individual> kids;
        kids.reserve(settings.population_size);

        assert(s_distr.min() == 0);
        assert(static_cast<std::size_t>(s_distr.max()+1) == population.size());

        std::generate_n(std::back_inserter(kids), settings.population_size,
                        [&]()
                        {
                            auto const par_1 = s_distr(rnd);
                            auto const par_2 = s_distr(rnd);

                            auto kid = settings.crossover(population[par_1].solution,
                                                          population[par_2].solution, rnd);
                            settings.mutate(kid, rnd);
                            auto obj_value = problem.objective(kid);

                            return Individual{std::move(kid), std::move(obj_value)};
                        });

        // Смена поколений
        settings.change_generation(population, kids, problem.compare);

        assert(population.size() == static_cast<std::size_t>(settings.population_size));
    }

    template <class Problem, class GA_settings, class UniformRandomBitGegerator>
    auto genetic_algorithm_boolean(Problem const & problem,
                                   GA_settings const & settings,
                                   UniformRandomBitGegerator & rnd)
    {
        using Genotype = typename GA_settings::genotype_type;

        // Инициализация
        using Individual = saga::evaluated_solution<Genotype, double>;
        using Population = std::vector<Individual>;

        auto population = saga::ga_boolen_initial_population<Population>(settings.population_size
                                                                         , problem.dimension
                                                                         , problem.objective, rnd);

        saga::for_n(settings.max_iterations, [&]
        {
            ::saga::genetic_algorithm_boolean_cycle(population, problem, settings, rnd);
        });

        return population;
    }

    template <class Result, class InputCursor>
    Result binary_sequence_to_integer(InputCursor cur)
    {
        return saga::polynomial_horner(std::move(cur), Result(2), Result(0));
    }

    template <class Result, class InputCursor>
    Result binary_sequence_to_real(InputCursor cur)
    {
        return saga::polynomial_horner(std::move(cur), Result(0.5), Result(0.0)) * 0.5;
    }

    template <class Result, class InputCursor>
    Result gray_code_to_integer(InputCursor cur)
    {
        auto result = Result(0);
        bool parity = 0;

        for(; !!cur; ++ cur)
        {
            bool value = (*cur != false);

            result = 2 * result + (value != parity);
            parity = (parity != value);
        }

        return result;
    }

    template <class Result, class InputCursor>
    Result gray_code_to_real(InputCursor cur)
    {
        auto result = Result(0);
        saga::cursor_difference_t<InputCursor> num(0);
        bool parity = 0;

        for(; !!cur; ++ cur)
        {
            ++ num;
            bool value = (*cur != false);

            result = 2 * result + (value != parity);
            parity = (parity != value);
        }

        return result * std::pow(Result(0.5), num);
    }
}
// namespace saga

#endif
// Z_SAGA_OPTIMIZATION_GA_HPP_INCLUDED
