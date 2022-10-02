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

#ifndef Z_SAGA_ACTIONS_SORT_HPP_INCLUDED
#define Z_SAGA_ACTIONS_SORT_HPP_INCLUDED

/** @file saga/actions/sort.hpp
 @brief Функциональный объект, выполняющий сортировку контейнера.
*/

#include <saga/actions/action_closure.hpp>
#include <saga/algorithm.hpp>
#include <saga/cursor/subrange.hpp>

namespace saga
{
namespace actions
{
    struct sort_fn
    {
        template <class Range>
        friend Range operator|(Range && arg, sort_fn pipe)
        {
            return std::move(pipe)(std::forward<Range>(arg));
        }

        template <class Range>
        friend void operator|=(Range & arg, sort_fn pipe)
        {
            std::ref(arg) | std::move(pipe);
        }

        template <class Range, class Compare = std::less<>
                 , class = std::enable_if_t<saga::is_range<Range>{}>
                 , class = std::enable_if_t<!std::is_reference<Range>{}>>
        Range operator()(Range && arg, Compare cmp = {}) const
        {
            saga::sort(saga::cursor::all(arg), std::move(cmp));

            return arg;
        }

        template <class Compare, class = std::enable_if_t<!saga::is_range<Compare>{}>>
        auto operator()(Compare cmp) const
        {
            auto fun = [cmp](auto && arg)
            {
                return sort_fn{}(std::forward<decltype(arg)>(arg), cmp);
            };

            return saga::actions::make_action_closure(std::move(fun));
        }
    };

    constexpr auto sort = saga::actions::sort_fn{};
}
// namespace actions
}
// namespace saga

#endif
// Z_SAGA_ACTIONS_SORT_HPP_INCLUDED
