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

#ifndef Z_SAGA_OPTIMIZATION_GA_SELECTION_RANKING_H_INCLUDED
#define Z_SAGA_OPTIMIZATION_GA_SELECTION_RANKING_H_INCLUDED

/** @file saga/optimization/ga/selection/ranking.hpp
 @brief Реализация ранговой селекции
*/

#include <saga/iterator.hpp>

#include <algorithm>
#include <map>
#include <random>

namespace saga
{
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
            ws.reserve(obj_values.size());
            std::transform(saga::begin(obj_values), saga::end(obj_values), std::back_inserter(ws),
                           [&](typename Container::value_type const & value)
                           {
                               return fs.at(value);
                           });

            return Distribution(ws.begin(), ws.end());
        }
    };
}
// namespace saga


#endif
// Z_SAGA_OPTIMIZATION_GA_SELECTION_RANKING_H_INCLUDED

