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

#ifndef Z_SAGA_ACTION_UNIQUE_HPP_INCLUDED
#define Z_SAGA_ACTION_UNIQUE_HPP_INCLUDED

/** @file saga/action/unique.hpp
 @brief Функциональный объект, выполняющий сортировку интервала.
*/

#include <saga/action/erase.hpp>
#include <saga/action/action_closure.hpp>
#include <saga/algorithm.hpp>
#include <saga/cursor/subrange.hpp>

namespace saga
{
namespace action
{
    struct unique_fn
    {
        template <class Range, class BinaryPredicate = std::equal_to<>
                 , class = std::enable_if_t<saga::is_range<Range>{}>
                 , class = std::enable_if_t<!std::is_reference<Range>{}>>
        Range operator()(Range && arg, BinaryPredicate bin_pred = {}) const
        {
            auto to_erase = saga::unique(saga::cursor::all(arg), std::move(bin_pred));
            assert(to_erase.end() == saga::end(arg));

            saga::action::erase(arg, to_erase.begin(), to_erase.end());

            return std::move(arg);
        }

        template <class BinaryPredicate
                 ,class = std::enable_if_t<!saga::is_range<BinaryPredicate>{}>>
        auto operator()(BinaryPredicate bin_pred) const
        {
            auto fun = [bin_pred = std::move(bin_pred)](auto && arg)
            {
                return unique_fn{}(std::forward<decltype(arg)>(arg), bin_pred);
            };

            return saga::action::make_action_closure(std::move(fun));
        }
    };

    inline constexpr auto unique = saga::action::action_closure<unique_fn>{};
}
// namespace action
}
// namespace saga

#endif
// Z_SAGA_ACTION_UNIQUE_HPP_INCLUDED
