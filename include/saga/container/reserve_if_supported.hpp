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

#ifndef Z_SAGA_CONTAINER_RESERVE_IF_SUPPORTED_HPP_INCLUDED
#define Z_SAGA_CONTAINER_RESERVE_IF_SUPPORTED_HPP_INCLUDED

/** @file saga/container/reserve_if_supported.hpp
 @brief Вызов функции-члена @c reserve для контейнеров, у которых она есть.
*/

#include <saga/type_traits.hpp>

namespace saga
{
    namespace detail
    {
        template <class Container>
        void reserve_if_supported(Container &, typename Container::size_type, saga::priority_tag<0>)
        {
            return;
        }

        template <class Container>
        auto reserve_if_supported(Container & container
                                  , typename Container::size_type new_capacity
                                  , saga::priority_tag<1>)
        -> saga::void_t<decltype(container.reserve(new_capacity))>
        {
            container.reserve(new_capacity);
        }
    }

    template <class Container>
    void reserve_if_supported(Container & container, typename Container::size_type new_capacity)
    {
        return detail::reserve_if_supported(container, new_capacity, saga::priority_tag<1>{});
    }
}
//namespace saga

#endif
// Z_SAGA_CONTAINER_RESERVE_IF_SUPPORTED_HPP_INCLUDED
