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

    struct triangular_number_fn
    {
        template <class IntType>
        constexpr IntType operator()(IntType num) const
        {
            assert(num >= 0);

            if(num % 2 == 0)
            {
                return num / 2 * (num + 1);
            }
            else
            {
                return (num + 1) / 2 * num;
            }
        }
    };

    struct square_pyramidal_number_fn
    {
        template <class IntType>
        constexpr IntType operator()(IntType num) const
        {
            auto P = saga::triangular_number_fn{}(num);

            if(P % 3 == 0)
            {
                return std::move(P) / 3 * (2 * num + 1);
            }
            else
            {
                return (2 * num + 1) / 3 * std::move(P);
            }
        }
    };

    struct power_natural_fn
    {
        template <class IntType, class Power, class BinaryOperation = std::multiplies<>>
        constexpr IntType operator()(IntType base, Power power, BinaryOperation bin_op = {}) const
        {
            static_assert(std::is_arithmetic<Power>{}, "");
            assert(power > 0);

            if(power == 1)
            {
                return base;
            }

            auto result = saga::square_fn{}((*this)(base, power / 2), bin_op);

            if(power % 2 == 1)
            {
                result = bin_op(std::move(result), std::move(base));
            }

            return result;
        }
    };

    inline constexpr auto abs = saga::absolute_value{};
    inline constexpr auto square = saga::square_fn{};
    inline constexpr auto triangular_number = triangular_number_fn{};
    inline constexpr auto square_pyramidal_number = square_pyramidal_number_fn{};
    inline constexpr auto power_natural = saga::power_natural_fn{};
}
//namespace saga

#endif
// Z_SAGA_MATH_HPP_INCLUDED
