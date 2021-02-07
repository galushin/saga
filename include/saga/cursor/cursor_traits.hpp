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

#ifndef Z_SAGA_CURSOR_CURSOR_TRAITS_HPP_INCLUDED
#define Z_SAGA_CURSOR_CURSOR_TRAITS_HPP_INCLUDED

#include <saga/detail/static_empty_const.hpp>

namespace saga
{
    template <class Cursor>
    using cursor_reference = typename Cursor::reference;

    struct front_fn
    {};

    struct back_fn
    {};

    namespace
    {
        constexpr auto const & front = detail::static_empty_const<front_fn>::value;
        constexpr auto const & back = detail::static_empty_const<back_fn>::value;
    }
}
// namespace saga

#endif
// Z_SAGA_CURSOR_CURSOR_TRAITS_HPP_INCLUDED
