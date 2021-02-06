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
        -> decltype(x.size())
        {
            assert(x.size() == y.size());

            return saga::inner_product(saga::cursor::all(x), saga::cursor::all(y),
                                       0*x.size(), std::plus<>{}, std::not_equal_to<>{});
        }
    };

    struct count_adjacent_unequal_fn
    {
        template <class Container>
        auto operator()(Container const & arg) const
        -> typename Container::size_type
        {
            using Size = typename Container::size_type;
            auto result = Size(0);

            auto first = saga::begin(arg);
            auto const last = saga::end(arg);

            if(first == last)
            {
                return result;
            }

            auto second = std::next(first);

            for(; second != last;)
            {
                result += (*first != *second);

                ++ first;
                ++ second;
            }

            return result;
        }
    };

    namespace detail
    {
        // @todo В библиотеку
        struct absolute_value
        {
            template <class Arg>
            auto operator()(Arg const & arg) const
            {
                using std::abs;
                return abs(arg);
            }
        };
    }

    struct manhattan_norm_fn
    {
    public:
        template <class Argument, class Abs = detail::absolute_value>
        auto operator()(Argument const & arg, Abs abs_fun = Abs()) const
        {
            using std::abs;
            auto result = abs_fun(typename Argument::value_type(0));

            // @todo Заменить цикл на алгоритм (подумать, в каком имеено варианте)
            for(auto const & each : arg)
            {
                result += abs_fun(each);
            }

            return result;
        };
    };

    namespace
    {
        constexpr auto const & boolean_manhattan_norm
            = detail::static_empty_const<boolean_manhattan_norm_fn>::value;

        constexpr auto const & boolean_manhattan_distance
            = detail::static_empty_const<boolean_manhattan_distance_fn>::value;

        constexpr auto const & count_adjacent_unequal
            = detail::static_empty_const<count_adjacent_unequal_fn>::value;

        constexpr auto const & manhattan_norm
            = detail::static_empty_const<manhattan_norm_fn>::value;
    }
}
// namespace saga

#endif
// Z_SAGA_OPTIMIZATION_TEST_OBJECTIVES_HPP_INCLUDED
