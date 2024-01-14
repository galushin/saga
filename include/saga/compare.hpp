/* (c) 2024 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_COMPARE_HPP_INCLUDED
#define Z_SAGA_COMPARE_HPP_INCLUDED

/** @file saga/compare.hpp
 @brief Функциональность, связанная с функциями сравнения
*/

#include <saga/functional.hpp>

namespace saga
{
    struct out_of_order_fn
    {
        template <class T1, class T2, class Compare = std::less<>>
        constexpr bool operator()(T1 && lhs, T2 && rhs, Compare && cmp = {}) const
        {
            return saga::invoke(std::forward<Compare>(cmp)
                               ,std::forward<T2>(rhs)
                               ,std::forward<T1>(lhs));
        }
    };

    inline constexpr auto const out_of_order = out_of_order_fn{};
    inline constexpr auto const in_order = saga::not_fn(out_of_order);
}
// namespace saga

#endif
// Z_SAGA_COMPARE_HPP_INCLUDED
