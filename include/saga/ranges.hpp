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

#ifndef Z_SAGA_RANGES_HPP_INCLUDED
#define Z_SAGA_RANGES_HPP_INCLUDED

#include <saga/iterator.hpp>
#include <saga/type_traits.hpp>

namespace saga
{
    namespace detail
    {
        template <class T>
        using detect_begin_end = saga::void_t<decltype(saga::begin(std::declval<T&>()))
                                             , decltype(saga::end(std::declval<T&>()))>;
    }

    template <class T>
    struct is_range
     : saga::is_detected<detail::detect_begin_end, T>
    {};

    template <class R>
    struct enable_borrowed_range_t
     : std::false_type
    {};

    template <class R>
    struct is_borrowed_range
     : saga::bool_constant<saga::is_range<R>{} && (std::is_lvalue_reference<R>{}
                                                   || saga::enable_borrowed_range_t<R>{})>
    {};
}

#endif
// Z_SAGA_RANGES_HPP_INCLUDED
