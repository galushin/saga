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

#ifndef Z_SAGA_GA_SELECTION_PROPORTIONAL_HPP_INCLUDED
#define Z_SAGA_GA_SELECTION_PROPORTIONAL_HPP_INCLUDED

/** @file saga/optimization/ga/selection/proportional.hpp
 @brief Реализация пропорциональной селекции
*/

#include <cassert>

#include <algorithm>
#include <functional>
#include <random>

namespace saga
{
    class selection_proportional
    {
    public:
        template <class Container, class Compare>
        std::discrete_distribution<typename Container::difference_type>
        build_distribution(Container const & obj_values, Compare cmp) const
        {
            assert(obj_values.empty() == false);

            auto const extr
                = std::minmax_element(obj_values.begin(), obj_values.end(), std::ref(cmp));

            std::vector<double> weights;
            weights.reserve(obj_values.size());

            auto objective_value_to_weight = [&](typename Container::value_type const & obj_value)
            {
                return static_cast<double>(*extr.second - obj_value) / (*extr.second - *extr.first);
            };

            std::transform(obj_values.begin(), obj_values.end(), std::back_inserter(weights),
                           objective_value_to_weight);

            assert(std::all_of(weights.begin(), weights.end(),
                               [](double const & weight) { return weight >= 0; }));
            assert(std::accumulate(weights.begin(), weights.end(), 0.0) > 0.0);

            using Distribution = std::discrete_distribution<typename Container::difference_type>;

            return Distribution(weights.begin(), weights.end());
        }
    };
}
// namespace saga

#endif
// Z_SAGA_GA_SELECTION_PROPORTIONAL_HPP_INCLUDED
