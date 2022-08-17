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

#ifndef Z_SAGA_PIPES_FOR_EACH_HPP_INCLUDED
#define Z_SAGA_PIPES_FOR_EACH_HPP_INCLUDED

/** @file saga/pipes/for_each.hpp
 @brief Умный курсор вывода, передающий элементы функциональному объекту
*/

#include <saga/functional.hpp>
#include <saga/type_traits.hpp>

#include <type_traits>

namespace saga
{
    template <class UnaryFunction>
    class output_function_cursor
     : private UnaryFunction
    {
    public:
        // Создание, копирование, уничтожение
        constexpr explicit output_function_cursor(UnaryFunction fun)
         : UnaryFunction(std::move(fun))
        {}

        // Курсор вывода
        constexpr bool operator!() const
        {
            return false;
        }

        template <class Arg, class = std::enable_if_t<saga::is_invocable<UnaryFunction, Arg>{}>>
        constexpr output_function_cursor &
        operator<<(Arg && arg)
        {
            static_cast<void>(saga::invoke(this->function_ref(), std::forward<Arg>(arg)));

            return *this;
        }

        // Функция
        constexpr UnaryFunction function() &&
        {
            return *this;
        }

    private:
        constexpr UnaryFunction & function_ref()
        {
            return *this;
        }
    };

    namespace pipes
    {
        template <class UnaryFunction>
        constexpr output_function_cursor<UnaryFunction>
        for_each(UnaryFunction fun)
        {
            return saga::output_function_cursor<UnaryFunction>(std::move(fun));
        }
    }
    // namespace pipes
}
// namespace saga

#endif
// Z_SAGA_PIPES_FOR_EACH_HPP_INCLUDED
