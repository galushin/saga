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

#ifndef Z_SAGA_OPTIMIZATION_GA_HPP_INCLUDED
#define Z_SAGA_OPTIMIZATION_GA_HPP_INCLUDED

/** @brief saga/optimization/ga.hpp
 @brief Функциональность, связанная с генетическими алгоритмами
*/

#include <saga/math/probability.hpp>
#include <saga/optimization/optimization_problem.hpp>
#include <saga/random/iid_distribution.hpp>

#include <random>

namespace saga
{
    /** @brief Инициализация начальной популяции генетического алгоритма псведо-булевой оптимизации
    @param population контейнер, используемый для хранения популяции
    @param population_size требуемый размер начальной популяции
    @param dim размерность (число генов в генотипе)
    @param rnd_engine генератор равномерно распределённых битов, используемый для порождения
    псевдо-случайных чисел
    */
    template <class Container, class Size, class UniformRandomBitGenerator>
    void ga_boolen_initialize_population(Container & population,
                                         typename Container::difference_type const population_size,
                                         Size const dim,
                                         UniformRandomBitGenerator & rnd_engine)
    {
        assert(population_size >= 0);
        assert(dim >= 0);

        auto const pop_size = static_cast<typename Container::size_type>(population_size);

        saga::iid_distribution<std::bernoulli_distribution, typename Container::value_type>
            distr(dim);
        auto gen = [&] { return distr(rnd_engine); };

        population.reserve(pop_size);

        if(population.size() > pop_size)
        {
            population.resize(pop_size);
        }

        saga::generate(saga::cursor::all(population), gen);

        assert(population.size() <= pop_size);
        std::generate_n(std::back_inserter(population), pop_size - population.size(), gen);
    }

    template <class Container, class UniformRandomBitGenerator>
    typename Container::size_type
    selection_tournament_2(Container const & obj_values, UniformRandomBitGenerator & rnd)
    {
        assert(!obj_values.empty());

        std::uniform_int_distribution<typename Container::size_type> distr(0, obj_values.size() - 1);

        auto const i1 = distr(rnd);
        auto const i2 = distr(rnd);

        assert(0 <= i1 && i1 < obj_values.size());
        assert(0 <= i2 && i2 < obj_values.size());

        return (obj_values[i1] > obj_values[i2]) ? i1 : i2;
    }

    template <class Genotype, class UniformRandomBitGenerator>
    Genotype
    ga_boolean_crossover_uniform(Genotype const & gen1, Genotype const & gen2,
                                 UniformRandomBitGenerator & rnd)
    {
        assert(gen1.size() == gen2.size());
        auto const dim = gen1.size();

        Genotype result(dim);

        std::bernoulli_distribution distr(0.5);

        for(auto i = 0*dim; i != dim; ++ i)
        {
            result[i] = distr(rnd) ? gen1[i] : gen2[i];
        }

        return result;
    }

    template <class Genotype, class UniformRandomBitGenerator>
    void ga_boolean_mutation(Genotype & genotype, saga::probability<double> const & p_mutation,
                             UniformRandomBitGenerator & rnd)
    {
        std::bernoulli_distribution distr(p_mutation.value());

        for(auto & gen : genotype)
        {
            gen ^= distr(rnd);
        }
    }

    struct GA_settings
    {
    public:
        int population_size = 0;
        int max_iterations = 0;
    };

    template <class Objective, class UniformRandomBitGegerator>
    auto genetic_algorithm(optimization_problem_boolean<Objective> const & problem,
                           GA_settings const & settings,
                           UniformRandomBitGegerator & rnd)
    {
        using Genotype = std::valarray<bool>;
        saga::probability<double> const p_mutation{1.0 / static_cast<double>(problem.dimension)};

        // Инициализация
        std::vector<Genotype> population;

        saga::ga_boolen_initialize_population(population, settings.population_size,
                                              problem.dimension, rnd);

        for(auto n = settings.max_iterations; n > 0; -- n)
        {
            // Оценка
            std::vector<double> obj_values;
            obj_values.reserve(settings.population_size);

            std::transform(saga::begin(population), saga::end(population),
                           std::back_inserter(obj_values), problem.objective);

            // Селекция и Скрещивание
            std::vector<Genotype> kids;
            kids.reserve(settings.population_size);

            // Элитизм
            auto const best = std::max_element(saga::begin(obj_values), saga::end(obj_values))
                            - saga::begin(obj_values);
            kids.push_back(population[best]);

            // Турнирная селекция + равномерное скрещивание + нормальная мутация
            for(; kids.size() < settings.population_size; )
            {
                auto const par_1 = saga::selection_tournament_2(obj_values, rnd);
                auto const par_2 = saga::selection_tournament_2(obj_values, rnd);

                auto kid = saga::ga_boolean_crossover_uniform(population.at(par_1),
                                                              population.at(par_2), rnd);
                saga::ga_boolean_mutation(kid, p_mutation, rnd);
                kids.push_back(std::move(kid));
            }

            assert(kids.size() == population.size());

            // Смена поколений
            population.swap(kids);
        }

        return population;
    }
}
// namespace saga

#endif
// Z_SAGA_OPTIMIZATION_GA_HPP_INCLUDED
