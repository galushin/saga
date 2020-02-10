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

#ifndef Z_SAGA_OPTIMIZATION_TEST_OBJECTIVES_HPP_INCLUDED
#define Z_SAGA_OPTIMIZATION_TEST_OBJECTIVES_HPP_INCLUDED

#include <algorithm>
#include <functional>
#include <numeric>

namespace saga
{
    struct boolean_manhattan_norm_fn
    {
        template <class BooleanVector = void>
        auto operator()(BooleanVector const & arg) const
        -> decltype(arg.size())
        {
            return std::count(arg.begin(), arg.end(), true);
        }
    };

    // @todo Должно быть inline
    constexpr auto boolean_manhattan_norm = boolean_manhattan_norm_fn{};

    struct boolean_manhattan_distance_fn
    {
        template <class BooleanVector1, class BooleanVector2>
        auto operator()(BooleanVector1 const & x, BooleanVector2 const & y) const
        -> decltype(x.size())
        {
            assert(x.size() == y.size());

            return std::inner_product(x.begin(), x.end(), y.begin(), 0*x.size(),
                                      std::plus<>{}, std::not_equal_to<>{});
        }
    };

    // @todo Должно быть inline
    constexpr auto boolean_manhattan_distance = boolean_manhattan_distance_fn{};

    struct count_adjacent_unequal_fn
    {
        // @todo Ослабить требования к аргументу
        template <class Vector>
        auto operator()(Vector const & arg) const
        -> decltype(arg.size())
        {
            auto result = arg.size()*0;

            for(auto i = 0*arg.size(); i+1 != arg.size(); ++ i)
            {
                result += (arg[i] != arg[i+1]);
            }

            return result;
        }
    };

    // @todo Должно быть inline
    constexpr auto count_adjacent_unequal = count_adjacent_unequal_fn{};
}
// namespace saga

#endif
// Z_SAGA_OPTIMIZATION_TEST_OBJECTIVES_HPP_INCLUDED
