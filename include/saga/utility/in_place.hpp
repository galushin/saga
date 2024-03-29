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

#ifndef Z_SAGA_UTILITY_IN_PLACE_HPP_INCLUDED
#define Z_SAGA_UTILITY_IN_PLACE_HPP_INCLUDED

/** @file saga/utility/in_place.hpp
 @brief Тип-тэг и константа для обозначения того, что конструктор должен создать хранимый объект
 на месте из представленных аргументов
*/

namespace saga
{
    struct in_place_t
    {
        // @todo Почему конструкторе без аргументов должен быть явным?
        explicit in_place_t() = default;
    };

    template <class T>
    struct in_place_type_t
    {
        // @todo Почему конструкторе без аргументов должен быть явным?
        explicit in_place_type_t() = default;
    };

    inline constexpr auto const in_place = in_place_t{};

    template <class T>
    inline constexpr auto const in_place_type = in_place_type_t<T>{};
}
// namespace saga

#endif
// Z_SAGA_UTILITY_IN_PLACE_HPP_INCLUDED
