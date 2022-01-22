/* (c) 2022 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_ALGORITHM_FIND_HPP_INCLUDED
#define Z_SAGA_ALGORITHM_FIND_HPP_INCLUDED

/** @file saga/algorithm/find_if.hpp
 @brief Алгоритм поиска элемента, удовлетворяющего унарному предикату
*/

#include <saga/detail/static_empty_const.hpp>
#include <saga/functional.hpp>

namespace saga
{
    struct find_if_fn
    {
        template <class InputCursor, class UnaryPredicate>
        InputCursor operator()(InputCursor cur, UnaryPredicate pred) const
        {
            for(; !!cur; ++cur)
            {
                if(saga::invoke(pred, *cur))
                {
                    break;
                }
            }

            return cur;
        }
    };

    namespace
    {
        constexpr auto const & find_if = detail::static_empty_const<find_if_fn>::value;
    }
}
// namespace saga

#endif
// Z_SAGA_ALGORITHM_FIND_HPP_INCLUDED
