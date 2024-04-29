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
#include <saga/algorithm.hpp>
#include <saga/cursor/cursor_traits.hpp>
#include <saga/cursor/subrange.hpp>
#include <saga/functional.hpp>
#include <saga/utility/exchange.hpp>

#include <functional>
#include <utility>

namespace saga
{
    template <class OutputCursor, class Incrementable>
    using iota_result = out_value_result<OutputCursor, Incrementable>;

    struct iota_fn
    {
    public:
        template <class OutputCursor, class Incrementable>
        constexpr iota_result<OutputCursor, Incrementable>
        operator()(OutputCursor out, Incrementable value) const
        {
            for(; !!out; ++ value)
            {
                out << value;
            }

            return {std::move(out), std::move(value)};
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
        constexpr in_out_result<InputCursor, OutputCursor>
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
        constexpr Value
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

    struct exclusive_scan_fn
    {
        template <class InputCursor, class OutputCursor, class T
                 , class BinaryOperation = std::plus<>>
        in_out_result<InputCursor, OutputCursor>
        operator()(InputCursor in, OutputCursor out, T init, BinaryOperation bin_op = {}) const
        {
            for(; !!in && !!out; ++ in)
            {
                auto next_init = saga::invoke(bin_op, std::move(init), *in);

                out << init;

                init = std::move(next_init);
            }

            return {std::move(in), std::move(out)};
        }
    };

    struct transform_reduce_fn
    {
        template <class InputCursor1, class InputCursor2, class T>
        T operator()(InputCursor1 in1, InputCursor2 in2, T init) const
        {
            return (*this)(std::move(in1), std::move(in2), std::move(init)
                           , std::plus<>{}, std::multiplies<>{});
        }

        template <class InputCursor1, class InputCursor2, class T
                 , class ReductionOp, class BinaryOperation>
        T operator()(InputCursor1 in1, InputCursor2 in2, T init
                     , ReductionOp reducer, BinaryOperation transformer) const
        {
            for(;!!in1 && !!in2; ++in1, void(++in2))
            {
                init = saga::invoke(reducer, std::move(init)
                                    , saga::invoke(transformer, *in1, *in2));
            }

            return init;
        }

        template <class InputCursor, class T, class ReductionOp, class UnaryOp>
        constexpr
        T operator()(InputCursor input, T init, ReductionOp reducer, UnaryOp transformer) const
        {
            for(; !!input; ++input)
            {
                init = saga::invoke(reducer, std::move(init), saga::invoke(transformer, *input));
            }

            return init;
        }
    };

    struct transform_exclusive_scan_fn
    {
        template <class InputCursor, class OutputCursor, class T
                 , class BinaryOperation, class UnaryOperation>
        in_out_result<InputCursor, OutputCursor>
        operator()(InputCursor input, OutputCursor out, T init, BinaryOperation bin_op
                   , UnaryOperation unary_op) const
        {
            for(; !!input && !!out; ++input)
            {
                auto next_init
                    = saga::invoke(bin_op, std::move(init), saga::invoke(unary_op, *input));

                out << init;

                init = std::move(next_init);
            }

            return {std::move(input), std::move(out)};
        }
    };

    struct transform_inclusive_scan_fn
    {
        template <class InputCursor, class OutputCursor
                 , class BinaryOperation, class UnaryOperation, class T>
        in_out_result<InputCursor, OutputCursor>
        operator()(InputCursor input, OutputCursor out
                   , BinaryOperation bin_op, UnaryOperation unary_op, T init) const
        {
            for(; !!input && !!out; ++ input)
            {
                init = saga::invoke(bin_op, std::move(init), saga::invoke(unary_op, *input));

                out << init;
            }

            return {std::move(input), std::move(out)};
        }

        template <class InputCursor, class OutputCursor
                 , class BinaryOperation, class UnaryOperation>
        in_out_result<InputCursor, OutputCursor>
        operator()(InputCursor input, OutputCursor out
                   , BinaryOperation bin_op, UnaryOperation unary_op) const
        {
            if(!input || !out)
            {
                return {std::move(input), std::move(out)};
            }

            saga::cursor_value_t<InputCursor> value = saga::invoke(unary_op, *input);
            ++ input;

            out << value;

            return (*this)(std::move(input), std::move(out), std::move(bin_op)
                           , std::move(unary_op), std::move(value));
        }
    };

    struct gcd_fn
    {
        template <class IntType1, class IntType2>
        constexpr
        std::common_type_t<IntType1, IntType2>
        operator()(IntType1 lhs, IntType2 rhs) const
        {
            return std::gcd(std::move(lhs), std::move(rhs));
        }
    };

    template <class IntType>
    struct extended_gcd_result
    {
        IntType gcd = 0;
        IntType first = 0;
        IntType second = 0;
    };

    struct extended_gcd_euclidean_fn
    {
        template <class IntType1, class IntType2>
        constexpr
        extended_gcd_result<std::common_type_t<IntType1, IntType2>>
        operator()(IntType1 lhs, IntType2 rhs) const
        {
            using Result = std::common_type_t<IntType1, IntType2>;
            auto x_0 = Result(0);
            auto x_1 = Result(1);
            auto y_0 = Result(1);
            auto y_1 = Result(0);

            for(; lhs != IntType1(0);)
            {
                auto qoutient = rhs / lhs;
                rhs = saga::exchange(lhs, rhs % lhs);
                y_0 = saga::exchange(y_1, y_0 - qoutient * y_1);
                x_0 = saga::exchange(x_1, x_0 - qoutient * x_1);
            }

            return {std::move(rhs), std::move(x_0), std::move(y_0)};
        }
    };

    struct lcm_fn
    {
        template <class IntType1, class IntType2>
        constexpr
        std::common_type_t<IntType1, IntType2>
        operator()(IntType1 lhs, IntType2 rhs) const
        {
            using Result = std::common_type_t<IntType1, IntType2>;
            return std::lcm(Result(std::move(lhs)), Result(std::move(rhs)));
        }
    };

    struct mark_eratosthenes_seive_fn
    {
        template <class RandomAccessCursor, class Size>
        void operator()(RandomAccessCursor cur, Size factor) const
        {
            assert(!!cur);

            *cur = false;

            for(; cur.size() > factor;)
            {
                cur.drop_front(factor);
                *cur = false;
            }
        }
    };

    struct eratosthenes_seive_fn
    {
        template <class RandomAccessCursor>
        void operator()(RandomAccessCursor const cur) const
        {
            using Size = typename RandomAccessCursor::difference_type;

            auto const num = cur.size();

            saga::fill_fn{}(cur, true);

            auto index = Size(0);
            auto index_square = Size(3);
            auto factor = Size(3);

            for(; index_square < num;)
            {
                if(cur[index])
                {
                    saga::mark_eratosthenes_seive_fn{}(saga::cursor::drop_front_n(cur, index_square)
                                                      , factor);
                }
                ++ index;

                index_square += factor;
                factor += Size(2);
                index_square += factor;
            }
        }
    };

    struct copy_primes_below_fn
    {
        template <class IntType, class OutputCursor>
        OutputCursor
        operator()(IntType n_max, OutputCursor out) const
        {
            using Seive = std::vector<unsigned short>;

            if(n_max < IntType(3))
            {
                return out;
            }

            out << IntType(2);

            if(n_max == IntType(3))
            {
                return out;
            }

            auto const N_seive = (n_max - 2)/2;

            Seive seive(N_seive, true);

            saga::eratosthenes_seive_fn{}(saga::cursor::all(seive));

            for(auto index = IntType(0); index < N_seive; ++ index)
            {
                if(seive[index])
                {
                    out << 2*index + 3;
                }
            }

            return out;
        }
    };

    struct primes_below_fn
    {
        template <class IntType>
        std::vector<IntType>
        operator()(IntType n_max) const
        {
            std::vector<IntType> primes;

            saga::copy_primes_below_fn{}(std::move(n_max), saga::back_inserter(primes));

            return primes;
        }
    };

    struct euler_phi_below_fn
    {
        template <class IntType>
        std::vector<IntType>
        operator()(IntType n_max) const
        {
            auto const primes = saga::primes_below_fn{}(n_max);

            std::vector<IntType> result(n_max);

            saga::iota_fn{}(saga::cursor::all(result), IntType(0));

            for(auto const & prime : primes)
            {
                for(auto num = prime; num < n_max; num += prime)
                {
                    result[num] /= prime;
                    result[num] *= (prime - 1);
                }
            }

            return result;
        }
    };

    struct factoriadic_fn
    {
        template <class Size, class OutputCursor>
        void operator()(Size num, OutputCursor out) const
        {
            for(auto d = Size(1); num > 0; num /= d, void(++d))
            {
                out  << num % d;
            }
        }
    };

    struct nth_permutation_fn
    {
        template <class RandomAccessCursor, class Size>
        void operator()(RandomAccessCursor cur, Size num) const
        {
            if(num == 0)
            {
                return;
            }

            std::vector<Size> indices(cur.size(), Size(0));
            factoriadic_fn{}(std::move(num), saga::cursor::all(indices));

            for(; !indices.empty(); void(++cur), indices.pop_back())
            {
                assert(!!cur);

                if(indices.back() != 0)
                {
                    assert(indices.back() < cur.size());

                    saga::cursor_value_t<RandomAccessCursor> item = std::move(cur[indices.back()]);

                    auto pos = cur;
                    pos.forget_front();
                    pos.drop_front(indices.back() + 1);

                    saga::shift_right_fn{}(pos.dropped_front(), Size(1));

                    cur.front() = std::move(item);
                }
            }
        }
    };

    inline constexpr auto const iota          = iota_fn{};
    inline constexpr auto const accumulate    = accumulate_fn{};
    inline constexpr auto const inner_product = inner_product_fn{};
    inline constexpr auto const adjacent_difference = adjacent_difference_fn{};
    inline constexpr auto const partial_sum   = partial_sum_fn{};
    inline constexpr auto const reduce        = reduce_fn{};
    inline constexpr auto const inclusive_scan = inclusive_scan_fn{};
    inline constexpr auto const exclusive_scan = exclusive_scan_fn{};
    inline constexpr auto const transform_reduce = transform_reduce_fn{};
    inline constexpr auto const transform_exclusive_scan = transform_exclusive_scan_fn{};
    inline constexpr auto const transform_inclusive_scan = transform_inclusive_scan_fn{};

    inline constexpr auto const gcd = gcd_fn{};
    inline constexpr auto const lcm = lcm_fn{};
    inline constexpr auto const extended_gcd_euclidean = extended_gcd_euclidean_fn{};

    inline constexpr auto const mark_eratosthenes_seive = mark_eratosthenes_seive_fn{};
    inline constexpr auto const eratosthenes_seive = eratosthenes_seive_fn{};
    inline constexpr auto const copy_primes_below = copy_primes_below_fn{};
    inline constexpr auto const primes_below = primes_below_fn{};
    inline constexpr auto const euler_phi_below = euler_phi_below_fn{};

    inline constexpr auto const nth_permutation = nth_permutation_fn{};
}
// namespace saga

#endif
// Z_SAGA_NUMERIC_HPP_INCLUDED
