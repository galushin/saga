/* (c) 2019-2021 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_DETAIL_STATIC_EMPTY_CONST_H_INCLUDED
#define Z_SAGA_DETAIL_STATIC_EMPTY_CONST_H_INCLUDED

/** @file saga/detail/static_empty_const.hpp
 @brief Замена inline переменных для компиляторов, которые их не поддерживаю
 @todo Подумать, как можно было бы смягчить переход между такими переменными и inline
*/

namespace saga
{
namespace detail
{
    template <class T>
    struct static_empty_const
    {
        static constexpr T value{};
    };

    template <class T>
    constexpr T static_empty_const<T>::value;
}
// namespace detail
}
// namespace saga

#endif
// Z_SAGA_DETAIL_STATIC_EMPTY_CONST_H_INCLUDED
