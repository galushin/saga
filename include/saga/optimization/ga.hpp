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

#include <saga/optimization/evaluated_solution.hpp>
#include <saga/optimization/optimization_problem.hpp>
#include <saga/math/probability.hpp>
#include <saga/random/iid_distribution.hpp>

#include <random>
#include <map>

namespace saga
{
    /** @brief Инициализация начальной популяции генетического алгоритма псведо-булевой оптимизации
    @param population контейнер, используемый для хранения популяции
    @param population_size требуемый размер начальной популяции
    @param dim размерность (число генов в генотипе)
    @param objective целевая функция
    @param rnd_engine генератор равномерно распределённых битов, используемый для порождения
    псевдо-случайных чисел
    */
    template <class Container, class Size, class Objective, class UniformRandomBitGenerator>
    void ga_boolen_initialize_population(Container & population,
                                         typename Container::difference_type const population_size,
                                         Size const dim,
                                         Objective objective,
                                         UniformRandomBitGenerator & rnd_engine)
    {
        assert(population_size >= 0);
        assert(dim >= 0);

        auto const pop_size = static_cast<typename Container::size_type>(population_size);

        using Individual = typename Container::value_type;
        using Genotype = typename Individual::solution_type;

        saga::iid_distribution<std::bernoulli_distribution, Genotype> distr(dim);

        auto gen = [&]
        {
            auto x = distr(rnd_engine);
            auto y = objective(x);

            return Individual{std::move(x), std::move(y)};
        };

        population.reserve(pop_size);

        if(population.size() > pop_size)
        {
            population.resize(pop_size);
        }

        saga::generate(saga::cursor::all(population), gen);

        assert(population.size() <= pop_size);
        std::generate_n(std::back_inserter(population), pop_size - population.size(), gen);
    }

    template <class Container, class Compare>
    class selection_tournament_distribution
    {
    public:
        using result_type = typename Container::size_type;

        /**
        @pre <tt>objective_values.empty() == false</tt>
        @pre <tt>tournament_size > 0 </tt>
        @pre <tt>tournament_size < objective_values.size()</tt>
        */
        explicit selection_tournament_distribution(int tournament_size, bool repeat,
                                                   Container objective_values,
                                                   Compare cmp)
         : tournament_(tournament_size)
         , repeat_(repeat)
         , obj_values_(std::move(objective_values))
         , cmp_(std::move(cmp))
        {
            assert(!this->obj_values_.empty());
            assert(0 < this->tournament_);
            assert(static_cast<std::size_t>(this->tournament_) <= this->obj_values_.size());
        }

        template <class UniformRandomBitGenerator>
        result_type operator()(UniformRandomBitGenerator & rnd) const
        {
            if(this->repeat_)
            {
                return this->selection_repeat(rnd);
            }
            else
            {
                return this->selection_no_repeat(rnd);
            }
        }


    private:
        template <class UniformRandomBitGenerator>
        result_type
        selection_repeat(UniformRandomBitGenerator & rnd) const
        {
            std::uniform_int_distribution<result_type> distr(0, this->obj_values_.size() - 1);

            auto best = distr(rnd);
            assert(0 <= best && best < this->obj_values_.size());

            for(auto num = this->tournament_ - 1; num > 0; -- num)
            {
                auto cur = distr(rnd);

                assert(0 <= cur && cur < this->obj_values_.size());

                if(this->cmp_(this->obj_values_[cur], this->obj_values_[best]))
                {
                   best = std::move(cur);
                }
            }

            return best;
        }

        template <class UniformRandomBitGenerator>
        result_type
        selection_no_repeat(UniformRandomBitGenerator & rnd) const
        {
            std::vector<result_type> selected;
            selected.reserve(this->tournament_);

            std::uniform_real_distribution<double> distr(0.0, 1.0);
            auto left = this->tournament_;

            for(auto num = this->obj_values_.size(); num > 0; -- num)
            {
                auto p = distr(rnd);

                if(p*num < left)
                {
                    selected.push_back(num - 1);
                    -- left;
                }
            }
            assert(selected.size() == static_cast<std::size_t>(this->tournament_));

            for(auto const & x : selected)
            {
                assert(0 <= x && x < this->obj_values_.size());
            }

            auto index_cmp = [this](result_type const & x, result_type const & y)
            {
                return this->cmp_(this->obj_values_[x], this->obj_values_[y]);
            };

            return *std::min_element(selected.begin(), selected.end(), std::move(index_cmp));
        }

        int tournament_ = 2;
        bool repeat_ = true;
        Container obj_values_{};
        Compare cmp_ {};
    };

    class selection_tournament
    {
    public:
        /** @pre <tt>tournament_size > 0</tt>
        */
        explicit selection_tournament(int tournament_size = 2, bool repeat = true)
         : tournament_(tournament_size)
         , repeat_(repeat)
        {
            assert(tournament_size > 0);
        }

        template <class Container, class Compare>
        selection_tournament_distribution<Container, Compare>
        build_distribution(Container const & obj_values, Compare cmp) const
        {
            using Distribution = selection_tournament_distribution<Container, Compare>;
            // @todo Избегать копирования obj_values?
            return Distribution(this->tournament_, this->repeat_, obj_values, std::move(cmp));
        }

    private:
        int tournament_ = 2;
        bool repeat_ = true;
    };

    class selection_ranking
    {
    public:
        template <class Container, class Compare>
        std::discrete_distribution<typename Container::difference_type>
        build_distribution(Container const & obj_values, Compare cmp) const
        {
            using Value = typename Container::value_type;
            using Distribution = std::discrete_distribution<typename Container::difference_type>;

            auto cmp_tr = [&cmp](Value const & x, Value const & y)
            {
                return cmp(y, x);
            };

            std::map<Value, int, decltype(cmp_tr)> fs(std::move(cmp_tr));
            for(auto const & value : obj_values)
            {
                fs[value] += 1;
            }

            int N_cur = 0;
            for(auto & f : fs)
            {
                auto N_prev = N_cur;
                N_cur += f.second;
                f.second = (N_cur + N_prev + 1);
            }

            std::vector<double> ws;
            for(auto const & value : obj_values)
            {
                ws.push_back(fs.at(value));
            }

            assert(ws.size() == obj_values.size());

            return Distribution(ws.begin(), ws.end());
        }
    };

    class selection_proportional
    {
    public:
        template <class Container, class Compare>
        std::discrete_distribution<typename Container::difference_type>
        build_distribution(Container const & obj_values, Compare cmp) const
        {
            assert(obj_values.empty() == false);

            auto const extr
                = std::minmax_element(obj_values.begin(), obj_values.end(), std::move(cmp));

            std::vector<double> weights;
            weights.reserve(obj_values.size());

            for(auto const & value : obj_values)
            {
                auto p = static_cast<double>(*extr.second - value) / (*extr.second - *extr.first);
                weights.push_back(std::move(p));
            }

            assert(weights.size() == obj_values.size());
            for(auto const & w : weights)
            {
                assert(w >= 0);
            }
            assert(std::accumulate(weights.begin(), weights.end(), 0.0) > 0.0);

            using Distribution = std::discrete_distribution<typename Container::difference_type>;

            return Distribution(weights.begin(), weights.end());
        }

    private:
        double alpha_ = 2.0;
    };

    class ga_boolean_crossover_uniform_fn
    {
    public:
        template <class Genotype, class UniformRandomBitGenerator>
        static Genotype crossover(Genotype const & gen1, Genotype const & gen2,
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
        Genotype operator ()(Genotype const & gen1, Genotype const & gen2,
                             UniformRandomBitGenerator & rnd) const
        {
            return this->crossover(gen1, gen2, rnd);
        }

    };

    class ga_boolean_crossover_one_point_fn
    {
    public:
        template <class Genotype, class UniformRandomBitGenerator>
        static Genotype crossover(Genotype const & gen1, Genotype const & gen2,
                                  UniformRandomBitGenerator & rnd)
        {
            assert(gen1.size() == gen2.size());
            auto const dim = gen1.size();


            std::uniform_int_distribution<typename saga::size_type<Genotype>::type> distr(0, dim);
            auto const pos = distr(rnd);

            Genotype result(dim);
            auto const out
                = std::copy(saga::begin(gen1), saga::begin(gen1) + pos, saga::begin(result));
            std::copy(saga::begin(gen2) + pos, saga::end(gen2), out);

            return result;
        }

        template <class Genotype, class UniformRandomBitGenerator>
        Genotype operator ()(Genotype const & gen1, Genotype const & gen2,
                             UniformRandomBitGenerator & rnd) const
        {
            return this->crossover(gen1, gen2, rnd);
        }

    };

    class ga_boolean_crossover_two_point_fn
    {
    public:
        template <class Genotype, class UniformRandomBitGenerator>
        static Genotype crossover(Genotype const & gen1, Genotype const & gen2,
                                  UniformRandomBitGenerator & rnd)
        {
            assert(gen1.size() == gen2.size());
            auto const dim = gen1.size();

            std::uniform_int_distribution<typename saga::size_type<Genotype>::type> distr(0, dim);
            auto first = distr(rnd);
            auto last = (first + distr(rnd));

            if(last > dim)
            {
                last -= dim;
            }

            if(last < first)
            {
                using std::swap;
                swap(first, last);
            }

            assert(0 <= first && first <= last && last <= dim);

            Genotype result(dim);
            auto out = std::copy(saga::begin(gen1), saga::begin(gen1) + first, saga::begin(result));
            out = std::copy(saga::begin(gen2) + first, saga::begin(gen2) + last, out);
            std::copy(saga::begin(gen1) + last, saga::end(gen1), out);

            return result;
        }

        template <class Genotype, class UniformRandomBitGenerator>
        Genotype operator ()(Genotype const & gen1, Genotype const & gen2,
                             UniformRandomBitGenerator & rnd) const
        {
            return this->crossover(gen1, gen2, rnd);
        }

    };

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

    template <class Genotype, class Crossover, class Selection>
    struct GA_settings
    {
    public:
        using genotype_type = Genotype;

        int population_size = 0;
        int max_iterations = 0;
        double mutation_strength = 1.0;
        Crossover crossover{};
        Selection selection{};
    };

    auto mutation_probability(double mutation_strength, int genes_count)
    {
        auto const avg_mutant_genes = std::min<double>(mutation_strength, genes_count);
        return saga::probability<double>{avg_mutant_genes / genes_count};
    }

    template <class Problem, class GA_settings, class UniformRandomBitGegerator>
    auto genetic_algorithm(Problem const & problem, GA_settings const & settings,
                           UniformRandomBitGegerator & rnd)
    {
        using Genotype = typename GA_settings::genotype_type;

        auto const p_mutation
            = ::saga::mutation_probability(settings.mutation_strength, problem.dimension);

        // Инициализация
        using Individual = saga::evaluated_solution<Genotype, double>;
        std::vector<Individual> population;

        saga::ga_boolen_initialize_population(population, settings.population_size,
                                              problem.dimension, problem.objective, rnd);

        auto const cmp = [&](Individual const & x, Individual const & y)
        {
            return problem.compare(x.objective_value, y.objective_value);
        };

        for(auto n = settings.max_iterations; n > 0; -- n)
        {
            // Репродукция: селекция, скрещивание, нормальная мутация
            std::vector<Individual> kids;
            kids.reserve(settings.population_size);

            auto const best = std::min_element(saga::begin(population), saga::end(population), cmp);
            kids.push_back(*best);

            // @todo Обойтись без копирования
            std::vector<double> obj_values;
            obj_values.reserve(settings.population_size);

            for(auto const & each : population)
            {
                obj_values.push_back(each.objective_value);
            }

            auto s_distr = settings.selection.build_distribution(obj_values, problem.compare);

            for(; kids.size() < static_cast<std::size_t>(settings.population_size); )
            {
                auto const par_1 = s_distr(rnd);
                auto const par_2 = s_distr(rnd);

                assert(0 <= par_1 && static_cast<std::size_t>(par_1) < population.size());
                assert(0 <= par_2 && static_cast<std::size_t>(par_2) < population.size());

                auto kid = settings.crossover(population[par_1].solution,
                                              population[par_2].solution, rnd);
                saga::ga_boolean_mutation(kid, p_mutation, rnd);
                auto obj_value = problem.objective(kid);

                kids.push_back(Individual{std::move(kid), std::move(obj_value)});
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
