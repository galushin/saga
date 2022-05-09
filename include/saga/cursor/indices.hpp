/* (c) 2020-2022 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_VIEW_INDICES_HPP_INCLUDED
#define Z_SAGA_VIEW_INDICES_HPP_INCLUDED

#include <saga/cursor/iota.hpp>
#include <saga/ranges.hpp>

#include <limits>
#include <utility>

namespace saga
{
    namespace cursor
    {
        struct indices_fn
        {
        public:
            template <class Incrementable1, class Incrementable2>
            constexpr auto operator()(Incrementable1 from, Incrementable2 to) const
            -> saga::iota_cursor<std::common_type_t<Incrementable1, Incrementable2>>
            {
                using Cursor = saga::iota_cursor<std::common_type_t<Incrementable1, Incrementable2>>;
                return Cursor{std::move(from), std::move(to)};
            }

            template <class Incrementable>
            constexpr auto operator()(Incrementable num) const
            -> saga::iota_cursor<Incrementable>
            {
                return (*this)(Incrementable(), num);
            }
        };

        struct indices_of_fn
        {
        public:
            template <class SizedContainer>
            auto operator()(SizedContainer const & container) const
            {
                return saga::cursor::indices_fn{}(saga::size(container));
            }
        };

        inline constexpr auto const indices = indices_fn{};
        inline constexpr auto const indices_of = indices_of_fn{};
    }
    // namespace view
}
// namespace saga

#endif
// Z_SAGA_VIEW_INDICES_HPP_INCLUDED
