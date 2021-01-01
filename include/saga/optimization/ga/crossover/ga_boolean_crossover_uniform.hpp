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

#ifndef Z_SAGA_OPTIMIZATION_GA_CROSSOVER_GA_BOOLEAN_CROSSOVER_UNIFORM_HPP_INCLUDED
#define Z_SAGA_OPTIMIZATION_GA_CROSSOVER_GA_BOOLEAN_CROSSOVER_UNIFORM_HPP_INCLUDED

/** @file saga/optimization/ga/crossover/ga_boolean_crossover_uniform.hpp
 @brief Реализация равномерного скрещивания для генетического алгоритма псевдобулевой оптимизации
*/

#include <saga/iterator.hpp>

#include <algorithm>
#include <random>

namespace saga
{
    class ga_boolean_crossover_uniform_fn
    {
    public:
        template <class Genotype, class UniformRandomBitGenerator>
        static Genotype crossover(Genotype const & gen1, Genotype const & gen2,
                                  UniformRandomBitGenerator & rnd)
        {
            assert(gen1.size() == gen2.size());

            Genotype result(gen1.size());

            std::bernoulli_distribution distr(0.5);

            std::transform(saga::begin(gen1), saga::end(gen1), saga::begin(gen2),
                           saga::begin(result),
                           [&](bool lhs, bool rhs) { return distr(rnd) ? lhs : rhs; });

            return result;
        }

        template <class Genotype, class UniformRandomBitGenerator>
        Genotype operator ()(Genotype const & gen1, Genotype const & gen2,
                             UniformRandomBitGenerator & rnd) const
        {
            return this->crossover(gen1, gen2, rnd);
        }

    };
}
// namespace saga

#endif
// Z_SAGA_OPTIMIZATION_GA_CROSSOVER_GA_BOOLEAN_CROSSOVER_UNIFORM_HPP_INCLUDED
