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

#ifndef Z_SAGA_UTILITY_PIPEABLE_HPP_INCLUDED
#define Z_SAGA_UTILITY_PIPEABLE_HPP_INCLUDED

/** @file saga/utility/pipeable.hpp
 @brief Функциональность для упрощения добавления конвейерного (через оператор |) синтаксиса
 создания адаптеров курсоров
*/

namespace saga
{
    template <class UnaryFunction>
    class pipeable_unary
     : private UnaryFunction
    {
        template <class Cursor>
        constexpr friend auto operator|(Cursor cur, pipeable_unary const & pipe)
        {
            return static_cast<UnaryFunction const&>(pipe)(std::move(cur));
        }

    public:
        constexpr explicit pipeable_unary(UnaryFunction fun)
         : UnaryFunction(fun)
        {}
    };

    template <class UnaryFunction>
    constexpr pipeable_unary<UnaryFunction>
    make_pipeable(UnaryFunction fun)
    {
        return pipeable_unary<UnaryFunction>(std::move(fun));
    }
}
// namespace saga

#endif
// Z_SAGA_UTILITY_PIPEABLE_HPP_INCLUDED
