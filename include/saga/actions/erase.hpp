/* (c) 2024 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_ACTIONS_ERASE_HPP_INCLUDED
#define Z_SAGA_ACTIONS_ERASE_HPP_INCLUDED

/** @file saga/actions/erase.hpp
 @brief Функциональный объект, выполняющий удаление подинтервала.
*/

#include <saga/actions/action_closure.hpp>

namespace saga
{
namespace actions
{
    struct erase_fn
    {
        template <class Range, class Iterator, class Sentinel>
        auto operator()(Range & arg, Iterator first, Sentinel last) const
        {
            return arg.erase(first, last);
        }

        template <class Range, class Iterator, class Sentinel>
        auto operator()(std::reference_wrapper<Range> & arg, Iterator first, Sentinel last) const
        {
            return (*this)(arg.get(), first, last);
        }
    };

    inline constexpr auto erase = saga::actions::action_closure<erase_fn>{};
}
// namespace actions
}
// namespace saga

#endif
// Z_SAGA_ACTIONS_ERASE_HPP_INCLUDED
