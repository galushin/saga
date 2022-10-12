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

#ifndef Z_SAGA_ACTIONS_REVERSE_HPP_INCLUDED
#define Z_SAGA_ACTIONS_REVERSE_HPP_INCLUDED

/** @file saga/actions/sort.hpp
 @brief Функциональный объект, выполняющий обращение интервала.
*/

#include <saga/actions/action_closure.hpp>
#include <saga/algorithm.hpp>
#include <saga/cursor/subrange.hpp>

namespace saga
{
namespace actions
{
    struct reverse_fn
    {
        template <class Range>
        Range operator()(Range && arg) const
        {
            saga::reverse(saga::cursor::all(arg));

            return arg;
        }
    };

    inline constexpr auto reverse = saga::actions::action_closure<saga::actions::reverse_fn>{};
}
// namespace actions
}
// namespace saga


#endif
// Z_SAGA_ACTIONS_REVERSE_HPP_INCLUDED
