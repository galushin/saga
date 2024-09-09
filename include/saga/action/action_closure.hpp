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

#ifndef Z_SAGA_ACTION_ACTION_CLOSURE_HPP_INCLUDED
#define Z_SAGA_ACTION_ACTION_CLOSURE_HPP_INCLUDED

/** @file saga/action/action_closure.hpp
 @brief Вспомогательный класс, позволяющий использовать функциональные объекты из пространства
 имён action (со связанными параметрами) с операторами | и |=
*/

#include <saga/ranges.hpp>

#include <functional>
#include <utility>

namespace saga
{
namespace action
{
    template <class UnaryFunction>
    class action_closure;

    template <class UnaryFunction>
    action_closure<UnaryFunction>
    make_action_closure(UnaryFunction fun)
    {
        return action_closure<UnaryFunction>(std::move(fun));
    }

    template <class UnaryFunction>
    class action_closure
     : public UnaryFunction
    {
        template <class Range
                 ,class = std::enable_if_t<saga::is_range<Range>{}>
                 ,class = std::enable_if_t<!std::is_reference<Range>{}>>
        friend auto operator|(Range && arg, action_closure fun)
        {
            return std::move(fun)(std::forward<Range>(arg));
        }

        template <class Range>
        friend void operator|=(Range & arg, action_closure fun)
        {
            std::ref(arg) | std::move(fun);
        }

        template <class OtherUnaryFunction>
        friend auto operator|(action_closure lhs, action_closure<OtherUnaryFunction> rhs)
        {
            auto fun = [lhs = std::move(lhs), rhs = std::move(rhs)](auto && arg)
                { return rhs(lhs(std::forward<decltype(arg)>(arg))); };

            return saga::action::make_action_closure(std::move(fun));
        }

    public:
        template <class... Args>
        constexpr explicit action_closure(Args &&... args)
         : UnaryFunction(std::forward<Args>(args)...)
        {}
    };
}
// namespace action
}
// namespace saga

#endif
// Z_SAGA_ACTION_ACTION_CLOSURE_HPP_INCLUDED
