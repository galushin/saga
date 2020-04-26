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

#ifndef Z_SAGA_OPTIMIZATION_GA_CROSSOVER_GA_BOOLEAN_CROSSOVER_TWO_POINT_HPP_INCLUDED
#define Z_SAGA_OPTIMIZATION_GA_CROSSOVER_GA_BOOLEAN_CROSSOVER_TWO_POINT_HPP_INCLUDED

/** @file saga/optimization/ga/crossover/ga_boolean_crossover_two_point.hpp
 @brief Реализация двухточечного скрещивания для генетического алгоритма псевдобулевой оптимизации
*/

#include <saga/iterator.hpp>
#include <saga/type_traits.hpp>

#include <algorithm>
#include <random>

namespace saga
{
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
}
// namespace saga

#endif
// Z_SAGA_OPTIMIZATION_GA_CROSSOVER_GA_BOOLEAN_CROSSOVER_TWO_POINT_HPP_INCLUDED
