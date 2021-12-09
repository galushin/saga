/* (c) 2021 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_CURSOR_FORWARD_CURSOR_FACADE_HPP_INCLUDED
#define Z_SAGA_CURSOR_FORWARD_CURSOR_FACADE_HPP_INCLUDED

/** @file saga/cursor/forward_cursor_facade.hpp
 @brief Определение операторов *, ++ и << для прямых курсоров
*/

#include <utility>

namespace saga
{
    template <class Cursor, class Reference>
    class forward_cursor_facade
    {
        friend bool operator!=(Cursor const & lhs, Cursor const & rhs)
        {
            return !(lhs == rhs);
        }

        friend constexpr Cursor & operator++(Cursor & cur)
        {
            cur.drop_front();
            return cur;
        }

        friend constexpr Reference operator*(Cursor const & cur)
        {
            return cur.front();
        }

        template <class Arg, class = std::enable_if_t<std::is_assignable<Reference, Arg>{}>>
        friend constexpr Cursor & operator<<(Cursor & cur, Arg && arg)
        {
            *cur = std::forward<Arg>(arg);
            ++ cur;
            return cur;
        }
    };
}
// namespace saga

#endif
// Z_SAGA_CURSOR_FORWARD_CURSOR_FACADE_HPP_INCLUDED
