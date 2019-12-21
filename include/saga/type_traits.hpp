/* (c) 2019 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_TYPE_TRAITS_HPP_INCLUDED
#define Z_SAGA_TYPE_TRAITS_HPP_INCLUDED

/** @file saga/type_traits.hpp
 @brief Характеристики-типов
*/

#include <utility>

namespace saga
{
    template <class T>
    struct size_type
    {
    private:
        template <class U>
        static typename U::size_type impl(int);

        template <class U>
        static auto impl(...) -> decltype(std::declval<U>().size());

    public:
        using type = decltype(size_type::impl<T>(0));
    };
}
// namespace saga

#endif
// Z_SAGA_TYPE_TRAITS_HPP_INCLUDED
