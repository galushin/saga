/* (c) 2021 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_MATH_HPP_INCLUDED
#define Z_SAGA_MATH_HPP_INCLUDED

/** @file saga/math.hpp
 @brief Функциональность, связанная с математикой, общего назначения
*/

#include <cmath>

#include <functional>

namespace saga
{
    namespace detail
    {
        namespace abs_adl_ns
        {
            void abs() = delete;

            struct absolute_value
            {
                template <class Arg>
                auto operator()(Arg const & arg) const
                {
                    using std::abs;
                    return abs(arg);
                }
            };
        }
        // namespace abs_adl_ns
    }
    // namespace detail

    using detail::abs_adl_ns::absolute_value;

    struct square_fn
    {
        template <class T, class BinaryOperation = std::multiplies<>>
        constexpr T operator()(T const & arg, BinaryOperation bin_op = {}) const
        {
            return bin_op(arg, arg);
        }
    };

    inline constexpr auto const abs = saga::absolute_value{};
    inline constexpr auto const square = saga::square_fn{};
}
//namespace saga

#endif
// Z_SAGA_MATH_HPP_INCLUDED
