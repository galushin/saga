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

#ifndef Z_SAGA_OPTIMIZATION_TEST_OBJECTIVES_HPP_INCLUDED
#define Z_SAGA_OPTIMIZATION_TEST_OBJECTIVES_HPP_INCLUDED

/** @file saga/optimization/test_objetives.hpp
 @brief Тестовые функции оптимизации
*/

#include <saga/algorithm.hpp>
#include <saga/cursor/subrange.hpp>
#include <saga/math.hpp>
#include <saga/numeric.hpp>

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
            return saga::count(saga::cursor::all(arg), true);
        }
    };

    struct boolean_manhattan_distance_fn
    {
        template <class BooleanVector1, class BooleanVector2>
        auto operator()(BooleanVector1 const & x, BooleanVector2 const & y) const
        -> std::common_type_t<decltype(x.size()), decltype(y.size())>
        {
            assert(x.size() == y.size());

            using Size = std::common_type_t<decltype(x.size()), decltype(y.size())>;

            return saga::inner_product(saga::cursor::all(x), saga::cursor::all(y),
                                       Size{0}, std::plus<>{}, std::not_equal_to<>{});
        }
    };

    struct count_adjacent_unequal_fn
    {
        template <class Container>
        auto operator()(Container const & arg) const
        -> typename Container::size_type
        {
            return saga::adjacent_count(saga::cursor::all(arg), std::not_equal_to<>{});
        }
    };

    struct manhattan_norm_fn
    {
    public:
        template <class Argument, class Abs = saga::absolute_value>
        auto operator()(Argument const & arg, Abs abs_fun = Abs()) const
        {
            auto result = abs_fun(typename Argument::value_type(0));

            // @todo Заменить цикл на алгоритм (подумать, в каком имеено варианте)
            for(auto const & each : arg)
            {
                result += abs_fun(each);
            }

            return result;
        }
    };

    inline constexpr auto const boolean_manhattan_norm = boolean_manhattan_norm_fn{};
    inline constexpr auto const boolean_manhattan_distance = boolean_manhattan_distance_fn{};
    inline constexpr auto const count_adjacent_unequal = count_adjacent_unequal_fn{};
    inline constexpr auto const manhattan_norm = manhattan_norm_fn{};
}
// namespace saga

#endif
// Z_SAGA_OPTIMIZATION_TEST_OBJECTIVES_HPP_INCLUDED
