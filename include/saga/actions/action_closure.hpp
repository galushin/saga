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

/** @file saga/actions/action_closure.hpp
 @brief Вспомогательный класс, позволяющий использовать функциональные объекты из пространства
 имён actions с операторами | и |=
*/

#include <functional>
#include <utility>

namespace saga
{
namespace actions
{
    template <class UnaryFunction>
    class action_closure
     : private UnaryFunction
    {
        template <class Range>
        friend auto operator|(Range && arg, action_closure fun)
        {
            return std::move(fun)(std::forward<Range>(arg));
        }

        template <class Range>
        friend void operator|=(Range & arg, action_closure fun)
        {
            std::ref(arg) | std::move(fun);
        }

    public:
        explicit action_closure(UnaryFunction fun)
         : UnaryFunction(std::move(fun))
        {}

    private:
    };

    template <class UnaryFunction>
    action_closure<UnaryFunction>
    make_action_closure(UnaryFunction fun)
    {
        return action_closure<UnaryFunction>(std::move(fun));
    }
}
// namespace actions
}
// namespace saga

#endif
// Z_SAGA_ACTION_ACTION_CLOSURE_HPP_INCLUDED
