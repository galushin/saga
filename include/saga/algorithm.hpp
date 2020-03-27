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

#ifndef Z_SAGA_ALGORITHM_HPP_INCLUDED
#define Z_SAGA_ALGORITHM_HPP_INCLUDED

/** @file saga/algorithm.hpp
 @brief Аналоги алгоритмов STL, работающие с курсорами
*/

#include <saga/detail/static_empty_const.hpp>

namespace saga
{
    struct count_fn
    {
        template <class InputCursor, class T>
        typename InputCursor::difference_type
        operator()(InputCursor cur, T const & value) const
        {
            typename InputCursor::difference_type result = {0};

            for(; !!cur; ++ cur)
            {
                result += (*cur == value);
            }

            return result;
        }
    };

    struct generate_fn
    {
        template <class OutputCursor, class Generator>
        void operator()(OutputCursor cur, Generator gen) const
        {
            for(; !!cur; ++ cur)
            {
                *cur = gen();
            }
        }
    };

    struct equal_fn
    {
        template <class InputCursor1, class InputCursor2>
        bool operator()(InputCursor1 cur1, InputCursor2 cur2) const
        {
            for(; !!cur1 && !!cur2; ++cur1, (void)++cur2)
            {
                if(!(*cur1 == *cur2))
                {
                    return false;
                }
            }

            return !cur1 && !cur2;
        }
    };

    struct lexicographical_compare_fn
    {
        template <class InputCursor1, class InputCursor2>
        bool operator()(InputCursor1 cur1, InputCursor2 cur2) const
        {
            for(; !!cur1 && !!cur2; ++cur1, (void)++cur2)
            {
                if(*cur1 < *cur2)
                {
                    return true;
                }
                else if(*cur2 < *cur1)
                {
                    return false;
                }
            }

            return !cur1 && !!cur2;
        }
    };


    namespace
    {
        constexpr auto const count = detail::static_empty_const<count_fn>::value;

        constexpr auto const generate = detail::static_empty_const<generate_fn>::value;

        constexpr auto const equal = detail::static_empty_const<equal_fn>::value;

        constexpr auto const lexicographical_compare
            = detail::static_empty_const<lexicographical_compare_fn>::value;
    }
}
// namespace saga

#endif
// Z_SAGA_ALGORITHM_HPP_INCLUDED