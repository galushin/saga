/* (c) 2019-2021 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_NUMERIC_HPP_INCLUDED
#define Z_SAGA_NUMERIC_HPP_INCLUDED

#include <saga/algorithm/result_types.hpp>
#include <saga/cursor/cursor_traits.hpp>
#include <saga/functional.hpp>
#include <saga/detail/static_empty_const.hpp>

#include <functional>
#include <utility>

namespace saga
{
    struct iota_fn
    {
    public:
        template <class OutputCursor, class Incrementable>
        void operator()(OutputCursor out, Incrementable value) const
        {
            for(; !!out; ++ value)
            {
                out << value;
            }
        }
    };

    class accumulate_fn
    {
    public:
        template <class InputCursor, class Value, class BinaryOperation = std::plus<>>
        constexpr
        Value operator()(InputCursor cur,
                         Value init,
                         BinaryOperation bin_op = BinaryOperation()) const
        {
            for(; !!cur; ++ cur)
            {
                init = saga::invoke(bin_op, std::move(init), *cur);
            }

            return init;
        }
    };

    class inner_product_fn
    {
    public:
        template <class InputCursor1, class InputCursor2, class T,
                  class BinaryOperator1 = std::plus<>, class BinaryOperator2 = std::multiplies<>>
        constexpr T operator()(InputCursor1 in1, InputCursor2 in2, T init,
                               BinaryOperator1 op1 = {}, BinaryOperator2 op2 = {}) const
        {
            for(; !!in1 && !!in2; ++ in1, (void)++in2)
            {
                init = saga::invoke(op1, std::move(init), saga::invoke(op2, *in1, *in2));
            }

            return init;
        }
    };

    struct adjacent_difference_fn
    {
        template <class InputCursor, class OutputCursor, class BinaryOperation = std::minus<>>
        in_out_result<InputCursor, OutputCursor>
        operator()(InputCursor in, OutputCursor out, BinaryOperation op = {}) const
        {
            if(!in || !out)
            {
                return {std::move(in), std::move(out)};
            }

            saga::cursor_value_t<InputCursor> prev_value = *in;
            ++ in;

            out << prev_value;

            for(; !!in && !!out; (void)++in, ++out)
            {
                auto cur_value = *in;
                *out = saga::invoke(op, cur_value, std::move(prev_value)) ;
                prev_value = std::move(cur_value);
            }

            return {std::move(in), std::move(out)};
        }
    };

    struct partial_sum_fn
    {
        template <class InputCursor, class OutputCursor, class BinaryOperation = std::plus<>>
        in_out_result<InputCursor, OutputCursor>
        operator()(InputCursor in, OutputCursor out, BinaryOperation op = {}) const
        {
            if(!in || !out)
            {
                return {std::move(in), std::move(out)};
            }

            cursor_value_t<InputCursor> total = *in;
            ++ in;

            out << total;

            for(; !!in && !!out; ++in)
            {
                total = saga::invoke(op, std::move(total), *in);
                out << total;
            }

            return {std::move(in), std::move(out)};
        }
    };

    struct reduce_fn
    {
    public:
        template <class InputCursor
                  , class Value = cursor_value_t<InputCursor>
                  , class BinaryOperation = std::plus<>>
        Value
        operator()(InputCursor cur, Value init_value = {}, BinaryOperation bin_op = {}) const
        {
            return accumulate_fn{}(std::move(cur), std::move(init_value), std::move(bin_op));
        }
    };

    struct inclusive_scan_fn
    {
        template <class InputCursor, class OutputCursor, class BinaryOperation, class T>
        in_out_result<InputCursor, OutputCursor>
        operator()(InputCursor in, OutputCursor out, BinaryOperation bin_op, T init) const
        {
            for(; !!in && !!out; ++ in)
            {
                init = saga::invoke(bin_op, std::move(init), *in);
                out << init;
            }

            return {std::move(in), std::move(out)};
        }

        template <class InputCursor, class OutputCursor, class BinaryOperation = std::plus<>>
        in_out_result<InputCursor, OutputCursor>
        operator()(InputCursor in, OutputCursor out, BinaryOperation bin_op = {}) const
        {
            return saga::partial_sum_fn{}(std::move(in), std::move(out), std::move(bin_op));
        }
    };

namespace
{
    constexpr auto const & iota          = detail::static_empty_const<iota_fn>::value;
    constexpr auto const & accumulate    = detail::static_empty_const<accumulate_fn>::value;
    constexpr auto const & inner_product = detail::static_empty_const<inner_product_fn>::value;
    constexpr auto const & adjacent_difference
        = detail::static_empty_const<adjacent_difference_fn>::value;
    constexpr auto const & partial_sum   = detail::static_empty_const<partial_sum_fn>::value;
    constexpr auto const & reduce        = detail::static_empty_const<reduce_fn>::value;
    constexpr auto const & inclusive_scan = detail::static_empty_const<inclusive_scan_fn>::value;
}

}
// namespace saga

#endif
// Z_SAGA_NUMERIC_HPP_INCLUDED
