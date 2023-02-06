/* (c) 2023 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_CONTAINER_MAKE_HPP_INCLUDED
#define Z_SAGA_CONTAINER_MAKE_HPP_INCLUDED

/** @file saga/container/make.hpp
 @brief Создание контейнера по списку аргументов
*/

#include <saga/container/reserve_if_supported.hpp>

#include <type_traits>
#include <utility>

namespace saga
{
    template <template <class...> class Container, class... Args>
    auto make_container(Args &&... args)
    {
        using Value = std::common_type_t<std::decay_t<Args>...>;

        if constexpr (std::is_trivially_copyable<Value>{})
        {
            return Container<Value>{std::forward<Args>(args)...};
        }
        else
        {
            Container<Value> result;

            saga::reserve_if_supported(result, sizeof...(Args));

            (result.emplace_back(std::forward<Args>(args)), ...);

            return result;
        }
    }
}
// namespace saga

#endif
// Z_SAGA_CONTAINER_MAKE_HPP_INCLUDED
