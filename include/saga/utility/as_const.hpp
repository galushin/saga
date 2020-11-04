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

#ifndef Z_SAGA_UTILITY_AS_CONST_HPP_INCLUDED
#define Z_SAGA_UTILITY_AS_CONST_HPP_INCLUDED

/** @file saga/utility/as_const.hpp
 @brief Реализация функции @c as_const из C++17 для систем, где она недоступна. Описана в
 http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0007r1.html
*/

#include <type_traits>

namespace saga
{
    template <class T>
    constexpr std::add_const_t<T> & as_const(T & arg) noexcept
    {
        return arg;
    }

    template <class T>
    void as_const(T const &&) = delete;
}
// namespace saga

#endif
// Z_SAGA_UTILITY_AS_CONST_HPP_INCLUDED
