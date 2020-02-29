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
}
// namespace saga

#endif
// Z_SAGA_OPTIMIZATION_GA_HPP_INCLUDED
