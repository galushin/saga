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

#ifndef Z_SAGA_UTILITY_EXCHANGE_HPP_INCLUDED
#define Z_SAGA_UTILITY_EXCHANGE_HPP_INCLUDED

/** @file saga/utility/exchange.hpp
 @brief Функция, присваивающая новое значение переменной и возвращающая её предыдущее значение
*/

#include <utility>

namespace saga
{
    template <class T, class U = T>
    constexpr T exchange(T & obj, U && new_value) noexcept(std::is_nothrow_move_constructible<T>{}
                                                           && std::is_nothrow_assignable<T&,U>{})
    {
        T old_value = std::move(obj);
        obj = std::forward<U>(new_value);
        return old_value;
    }
}
// namespace saga

#endif
// Z_SAGA_UTILITY_EXCHANGE_HPP_INCLUDED
