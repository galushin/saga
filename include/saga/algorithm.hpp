/* (c) 2020-2021 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_ALGORITHM_HPP_INCLUDED
#define Z_SAGA_ALGORITHM_HPP_INCLUDED

/** @file saga/algorithm.hpp
 @brief Аналоги алгоритмов STL, работающие с курсорами и интервалами
*/

#include <saga/algorithm/result_types.hpp>
#include <saga/functional.hpp>
#include <saga/iterator.hpp>
#include <saga/cursor/cursor_traits.hpp>

#include <saga/detail/static_empty_const.hpp>

#include <cassert>
#include <algorithm>
#include <functional>
#include <random>

namespace saga
{
    // Немодифицирующие операции
    struct find_if_fn
    {
        template <class InputCursor, class UnaryPredicate>
        InputCursor operator()(InputCursor cur, UnaryPredicate pred) const
        {
            for(; !!cur; ++cur)
            {
                if(saga::invoke(pred, *cur))
                {
                    break;
                }
            }

            return cur;
        }
    };

    struct find_if_not_fn
    {
        template <class InputCursor, class UnaryPredicate>
        InputCursor operator()(InputCursor cur, UnaryPredicate pred) const
        {
            return find_if_fn{}(std::move(cur), saga::not_fn(std::move(pred)));
        }
    };

    struct find_fn
    {
        template <class InputCursor, class T, class BinaryPredicate = std::equal_to<>>
        InputCursor
        operator()(InputCursor cur, T const & value, BinaryPredicate bin_pred = {}) const
        {
            auto pred = [&](auto && arg)
                        { return saga::invoke(bin_pred, std::forward<decltype(arg)>(arg), value); };

            return find_if_fn{}(std::move(cur), std::move(pred));
        }
    };

    struct all_of_fn
    {
        template <class InputCursor, class UnaryPredicate>
        bool operator()(InputCursor cur, UnaryPredicate pred) const
        {
            return !find_if_not_fn{}(std::move(cur), std::move(pred));
        }
    };

    struct any_of_fn
    {
        template <class InputCursor, class UnaryPredicate>
        bool operator()(InputCursor cur, UnaryPredicate pred) const
        {
            return !!find_if_fn{}(std::move(cur), std::move(pred));
        }
    };

    struct none_of_fn
    {
        template <class InputCursor, class UnaryPredicate>
        bool operator()(InputCursor cur, UnaryPredicate pred) const
        {
            return !any_of_fn{}(std::move(cur), std::move(pred));
        }
    };

    template <class InputCursor, class UnaryFunction>
    using for_each_result = in_fun_result<InputCursor, UnaryFunction>;

    struct for_each_fn
    {
        template <class InputCursor, class UnaryFunction>
        for_each_result<InputCursor, UnaryFunction>
        operator()(InputCursor cur, UnaryFunction fun) const
        {
            for(; !!cur; ++cur)
            {
                saga::invoke(fun, *cur);
            }

            return {std::move(cur), std::move(fun)};
        }
    };

    template <class InputCursor, class UnaryFunction>
    using for_each_n_result = in_fun_result<InputCursor, UnaryFunction>;

    struct for_each_n_fn
    {
        template <class InputCursor, class Size, class UnaryFunction>
        for_each_n_result<InputCursor, UnaryFunction>
        operator()(InputCursor input, Size num, UnaryFunction fun) const
        {
            for(; !!input && num > 0; ++ input, void(--num))
            {
                saga::invoke(fun, *input);
            }

            return {std::move(input), std::move(fun)};
        }
    };

    struct count_if_fn
    {
        template <class InputCursor, class UnaryPredicate>
        cursor_difference_t<InputCursor>
        operator()(InputCursor cur, UnaryPredicate pred) const
        {
            auto result = cursor_difference_t<InputCursor>(0);

            for(; !!cur; ++ cur)
            {
                if(saga::invoke(pred, *cur))
                {
                    ++ result;
                }
            }

            return result;
        }
    };

    struct count_fn
    {
        template <class InputCursor, class T, class BinaryPredicate = std::equal_to<>>
        cursor_difference_t<InputCursor>
        operator()(InputCursor cur, T const & value, BinaryPredicate bin_pred = {}) const
        {
            auto pred = [&](auto && arg)
                { return saga::invoke(bin_pred, std::forward<decltype(arg)>(arg), value); };

            return count_if_fn{}(std::move(cur), std::move(pred));
        }
    };

    template <class InputCursor1, class InputCursor2>
    using mismatch_result = in_in_result<InputCursor1, InputCursor2>;

    struct mismatch_fn
    {
        template <class InputCursor1, class InputCursor2, class BinaryPredicate = std::equal_to<>>
        constexpr mismatch_result<InputCursor1, InputCursor2>
        operator()(InputCursor1 in1, InputCursor2 in2, BinaryPredicate bin_pred = {}) const
        {
            for(; !!in1 && !!in2; void(++in1), ++in2)
            {
                if(!saga::invoke(bin_pred, *in1, *in2))
                {
                    break;
                }
            }

            return {std::move(in1), std::move(in2)};
        }
    };

    struct find_first_of_fn
    {
        template <class InputCursor, class ForwardCursor, class BinaryPredicate = std::equal_to<>>
        InputCursor
        operator()(InputCursor cur, ForwardCursor s_cur, BinaryPredicate bin_pred = {}) const
        {
            auto const found_in_s_cur = [&](auto && lhs)
            {
                return !!saga::find_fn{}(s_cur,  std::forward<decltype(lhs)>(lhs), bin_pred);
            };

            return saga::find_if_fn{}(std::move(cur), found_in_s_cur);
        }
    };

    struct adjacent_find_fn
    {
        template <class ForwardCursor, class BinaryPredicate = std::equal_to<>>
        ForwardCursor operator()(ForwardCursor cur, BinaryPredicate bin_pred = {}) const
        {
            if(!cur)
            {
                return cur;
            }

            auto next = cur;
            ++ next;

            for(; !!next; ++ next)
            {
                if(saga::invoke(bin_pred, *cur, *next))
                {
                    return cur;
                }

                cur = next;
            }

            return next;
        }
    };

    struct search_fn
    {
        template <class ForwardCursor1, class ForwardCursor2
                 , class BinaryPredicate = std::equal_to<>>
        ForwardCursor1 operator()(ForwardCursor1 cur, ForwardCursor2 s_cur
                                  , BinaryPredicate bin_pred = {}) const
        {
            auto cur_rest = cur;
            auto s_cur_rest = s_cur;

            for(;;)
            {
                if(!s_cur_rest)
                {
                    return cur;
                }

                if(!cur_rest)
                {
                    return cur_rest;
                }

                if(saga::invoke(bin_pred, *cur_rest, *s_cur_rest))
                {
                    ++ cur_rest;
                    ++ s_cur_rest;
                }
                else
                {
                    ++ cur;
                    cur_rest = cur;

                    s_cur_rest = s_cur;
                }
            }
        }
    };

    struct find_end_fn
    {
        template <class ForwardCursor1, class ForwardCursor2
                 , class BinaryPredicate = std::equal_to<>>
        ForwardCursor1 operator()(ForwardCursor1 cur, ForwardCursor2 s_cur
                                  , BinaryPredicate bin_pred = {}) const
        {
            if(!s_cur)
            {
                cur.exhaust_front();
                return cur;
            }

            auto result = cur;
            result.exhaust_front();

            for(;;)
            {
                auto new_result = saga::search_fn{}(cur, s_cur, bin_pred);

                if(!new_result)
                {
                    return result;
                }
                else
                {
                    result = new_result;
                    cur = result;
                    ++ cur;
                }
            }
        }
    };

    struct search_n_fn
    {
        template <class ForwardCursor, class Size, class T, class BinaryPredicate = std::equal_to<>>
        ForwardCursor operator()(ForwardCursor input, Size const num, T const & value
                                 , BinaryPredicate bin_pred = {}) const
        {
            auto probe = input;
            auto cur_count = Size{0};

            for(;;)
            {
                if(cur_count == num)
                {
                    return input;
                }

                if(!probe)
                {
                    return probe;
                }

                if(saga::invoke(bin_pred, *probe, value))
                {
                    ++ probe;
                    ++ cur_count;
                }
                else
                {
                    ++ probe;
                    input = probe;
                    cur_count = Size{0};
                }
            }
        }
    };

    // Модифицирующие операции
    struct copy_fn
    {
        template <class InputCursor, class OutputCursor>
        in_out_result<InputCursor, OutputCursor>
        operator()(InputCursor in, OutputCursor out) const
        {
            for(; !!in && !!out; ++in)
            {
                out << *in;
            }

            return {std::move(in), std::move(out)};
        }
    };

    template <class InputCursor, class OutputCursor>
    using copy_if_result = in_out_result<InputCursor, OutputCursor>;

    struct copy_if_fn
    {
        template <class InputCursor, class OutputCursor, class Predicate>
        copy_if_result<InputCursor, OutputCursor>
        operator()(InputCursor cur, OutputCursor out, Predicate pred) const
        {
            for(; !!cur && !!out; ++ cur)
            {
                if(saga::invoke(pred, *cur))
                {
                    out << *cur;
                }
            }

            return {std::move(cur), std::move(out)};
        }
    };

    template <class InputCursor, class OutputCursor>
    using copy_n_result = in_out_result<InputCursor, OutputCursor>;

    struct copy_n_fn
    {
        template <class InputCursor, class Size, class OutputCursor>
        copy_n_result<InputCursor, OutputCursor>
        operator()(InputCursor input, Size num, OutputCursor out) const
        {
            for(; !!input && !!out && num > 0; ++ input, void(--num))
            {
                out << *input;
            }

            return {std::move(input), std::move(out)};
        }
    };

    template <class BidirectionalCursor1, class BidirectionalCursor2>
    using copy_backward_result = saga::in_out_result<BidirectionalCursor1, BidirectionalCursor2>;

    struct copy_backward_fn
    {
        template <class BidirectionalCursor1, class BidirectionalCursor2>
        copy_backward_result<BidirectionalCursor1, BidirectionalCursor2>
        operator()(BidirectionalCursor1 input, BidirectionalCursor2 out) const
        {
            for(; !!input && !!out;)
            {
                out.back() = input.back();
                input.drop_back();
                out.drop_back();
            }

            return {std::move(input), std::move(out)};
        }
    };

    template <class InputCursor, class OutputCursor>
    using move_result = in_out_result<InputCursor, OutputCursor>;

    struct move_fn
    {
        template <class InputCursor, class OutputCursor>
        move_result<InputCursor, OutputCursor>
        operator()(InputCursor cur, OutputCursor out) const
        {
            for(; !!cur && !!out; ++cur)
            {
                out << std::move(*cur);
            }

            return {std::move(cur), std::move(out)};
        }
    };

    struct fill_fn
    {
        template <class OutputCursor, class T>
        void operator()(OutputCursor cur, T const & value) const
        {
            for(; !!cur; ++ cur)
            {
                *cur = value;
            }
        }
    };

    struct fill_n_fn
    {
        template <class OutputCursor, class Size, class T>
        OutputCursor operator()(OutputCursor out, Size num, T const & value) const
        {
            for(; !!out && num > 0; -- num)
            {
                out << value;
            }

            return out;
        }
    };

    struct generate_fn
    {
        template <class OutputCursor, class Generator>
        void operator()(OutputCursor cur, Generator gen) const
        {
            for(; !!cur; ++ cur)
            {
                *cur = saga::invoke(gen);
            }
        }
    };

    struct generate_n_fn
    {
        template <class OutputCursor, class Size, class Generator>
        OutputCursor operator()(OutputCursor out, Size num, Generator gen) const
        {
            for(; !!out && num > 0; -- num)
            {
                out << gen();
            }

            return out;
        }
    };

    struct remove_if_fn
    {
        template <class ForwardCursor, class Predicate>
        ForwardCursor operator()(ForwardCursor cur, Predicate pred) const
        {
            auto out = saga::find_if_fn{}(std::move(cur), pred);

            if(!out)
            {
                return out;
            }

            cur = out;
            ++ cur;

            for(; !!cur; ++cur)
            {
                if(!saga::invoke(pred, *cur))
                {
                    out << std::move(*cur);
                }
            }

            return out;
        }
    };

    struct remove_fn
    {
        template <class ForwardCursor, class T, class BinaryPredicate = std::equal_to<>>
        ForwardCursor
        operator()(ForwardCursor cur, T const & value, BinaryPredicate bin_pred = {}) const
        {
            auto pred = [&](auto && arg)
                { return saga::invoke(bin_pred, std::forward<decltype(arg)>(arg), value); };

            return remove_if_fn{}(std::move(cur), std::move(pred));
        }
    };

    template <class InputCursor, class OutputCursor>
    using remove_copy_if_result = in_out_result<InputCursor, OutputCursor>;

    struct remove_copy_if_fn
    {
        template <class InputCursor, class OutputCursor, class UnaryPredicate>
        remove_copy_if_result<InputCursor, OutputCursor>
        operator()(InputCursor in, OutputCursor out, UnaryPredicate pred) const
        {
            return copy_if_fn{}(std::move(in), std::move(out), saga::not_fn(std::move(pred)));
        }
    };

    template <class InputCursor, class OutputCursor>
    using remove_copy_result = in_out_result<InputCursor, OutputCursor>;

    struct remove_copy_fn
    {
        template <class InputCursor, class OutputCursor, class T
                  , class BinaryPredicate = std::equal_to<>>
        remove_copy_result<InputCursor, OutputCursor>
        operator()(InputCursor in, OutputCursor out
                   , T const & value, BinaryPredicate bin_pred = {}) const
        {
            auto pred = [&](auto && x)
                { return saga::invoke(bin_pred, std::forward<decltype(x)>(x), value); };

            return remove_copy_if_fn{}(std::move(in), std::move(out), std::move(pred));
        }
    };

    struct replace_if_fn
    {
        template <class ForwardCursor, class UnaryPredicate, class T>
        void operator()(ForwardCursor cur, UnaryPredicate pred, T const & new_value) const
        {
            for(; !!cur; ++cur)
            {
                if(saga::invoke(pred, *cur))
                {
                    *cur = new_value;
                }
            }
        }
    };

    struct replace_fn
    {
        template <class ForwardCursor, class T, class BinaryPredicate = std::equal_to<>>
        void operator()(ForwardCursor cur, T const & old_value
                        , T const & new_value, BinaryPredicate bin_pred = {}) const
        {
            auto pred = [&](auto && arg)
                { return saga::invoke(bin_pred, std::forward<decltype(arg)>(arg), old_value); };

            return replace_if_fn{}(std::move(cur), std::move(pred), new_value);
        }
    };

    template <class InputCursor, class OutputCursor>
    using replace_copy_if_result = in_out_result<InputCursor, OutputCursor>;

    struct replace_copy_if_fn
    {
        template <class InputCursor, class OutputCursor, class UnaryPredicate, class T>
        replace_copy_if_result<InputCursor, OutputCursor>
        operator()(InputCursor in, OutputCursor out, UnaryPredicate pred, T const & new_value) const
        {
            for(; !!in && !!out; ++ in)
            {
                if(saga::invoke(pred, *in))
                {
                    out << new_value;
                }
                else
                {
                    out << *in;
                }
            }

            return {std::move(in), std::move(out)};
        }
    };

    template <class InputCursor, class OutputCursor>
    using replace_copy_result = in_out_result<InputCursor, OutputCursor>;

    struct replace_copy_fn
    {
        template <class InputCursor, class OutputCursor, class T1, class T2
                 , class BinaryPredicate = std::equal_to<>>
        replace_copy_result<InputCursor, OutputCursor>
        operator()(InputCursor in, OutputCursor out, T1 const & old_value
                   , T2 const & new_value, BinaryPredicate bin_pred = {}) const
        {
            auto pred = [&](auto && x)
                { return saga::invoke(bin_pred, std::forward<decltype(x)>(x), old_value); };

            return replace_copy_if_fn{}(std::move(in), std::move(out), std::move(pred), new_value);
        }
    };

    template <class ForwardCursor1, class ForwardCursor2>
    using swap_ranges_result = in_in_result<ForwardCursor1, ForwardCursor2>;

    struct swap_ranges_fn
    {
        template <class ForwardCursor1, class ForwardCursor2>
        swap_ranges_result<ForwardCursor1, ForwardCursor2>
        operator()(ForwardCursor1 in1, ForwardCursor2 in2) const
        {
            for(; !!in1 && !!in2; ++in1, void(++in2))
            {
                using std::swap;
                swap(*in1, *in2);
            }

            return {std::move(in1), std::move(in2)};
        }
    };

    struct reverse_fn
    {
    public:
        template <class BidirectionalCursor>
        void operator()(BidirectionalCursor cur) const
        {
            for(;!!cur; cur.drop_front())
            {
                auto cur_next = cur;
                cur_next.drop_back();

                if(!cur_next)
                {
                    break;
                }

                using std::swap;
                swap(cur.front(), cur.back());

                cur = std::move(cur_next);
            }
        }
    };

    template <class BidirectionalCursor, class OutputCursor>
    using reverse_copy_result = saga::in_out_result<BidirectionalCursor, OutputCursor>;

    struct reverse_copy_fn
    {
        template <class BidirectionalCursor, class OutputCursor>
        constexpr
        reverse_copy_result<BidirectionalCursor, OutputCursor>
        operator()(BidirectionalCursor input, OutputCursor output) const
        {
            for(;!!input && !!output;)
            {
                output << input.back();

                input.drop_back();
            }

            return {std::move(input), std::move(output)};
        }
    };

    template <class ForwardCursor, class OutputCursor>
    using rotate_copy_result = in_in_out_result<ForwardCursor, ForwardCursor, OutputCursor>;

    struct rotate_copy_fn
    {
        template <class ForwardCursor, class OutputCursor>
        rotate_copy_result<ForwardCursor, OutputCursor>
        operator()(ForwardCursor in, OutputCursor out) const
        {
            auto part2 = in.dropped_front();

            auto result1 = saga::copy_fn{}(std::move(in), std::move(out));
            auto result2 = saga::copy_fn{}(std::move(part2), std::move(result1.out));

            return {std::move(result2.in), std::move(result1.in), std::move(result2.out)};
        }
    };

    struct shuffle_fn
    {
        template <class RandomAccessCursor, class URBG>
        void operator()(RandomAccessCursor cur, URBG && gen) const
        {
            auto num = cur.size();

            if(num == 0)
            {
                return;
            }

            -- num;

            using Diff = saga::cursor_difference_t<RandomAccessCursor>;
            using Distr = std::uniform_int_distribution<Diff>;
            using Param = typename Distr::param_type;

            Distr distr;

            for(; num > 0; -- num)
            {
                using std::swap;
                swap(cur[num], cur[distr(gen, Param(0, num))]);
            }
        }
    };

    struct sample_fn
    {
    private:

        template <class InputCursor, class RandomAccessCursor, class URBG>
        RandomAccessCursor impl(InputCursor input, RandomAccessCursor out
                                , cursor_difference_t<InputCursor> const num, URBG && gen
                                , std::input_iterator_tag) const
        {
            assert(num <= out.size());

            // Инициализация
            auto result = out;

            for(auto rest = num; rest > 0 && !!input; ++input, void(--rest))
            {
                result << *input;
            }

            // Просмотр остальных элементов
            using Distr = std::uniform_int_distribution<cursor_difference_t<InputCursor>>;
            using Param = typename Distr::param_type;

            Distr distr;

            auto n_visited = num;

            for(; !!input; ++input, void(++n_visited))
            {
                auto pos = distr(gen, Param(0, n_visited));

                if(pos < num)
                {
                    out[pos] = *input;
                }
            }

            return result;
        }

        template <class ForwardCursor, class OutputCursor, class URBG>
        OutputCursor impl(ForwardCursor input, OutputCursor out
                          , cursor_difference_t<ForwardCursor> num, URBG && gen
                          , std::forward_iterator_tag) const
        {
            assert(num >= 0);

            auto input_n = saga::cursor::size(input);

            std::uniform_real_distribution<double> distr(0.0, 1.0);

            for(; !!input; ++input, void(--input_n))
            {
                if(distr(gen) * input_n < num)
                {
                    out << *input;
                    -- num;
                }
            }

            return out;
        }

    public:
        template <class PopulationCursor, class SampleCursor, class URBG>
        SampleCursor operator()(PopulationCursor input, SampleCursor out
                                , cursor_difference_t<PopulationCursor> num, URBG && gen) const
        {
            return sample_fn::impl(std::move(input), std::move(out), std::move(num), gen
                                   , saga::cursor_category_t<PopulationCursor>{});
        }
    };

    struct unique_fn
    {
        template <class ForwardCursor, class BinaryPredicate = std::equal_to<>>
        ForwardCursor operator()(ForwardCursor cur, BinaryPredicate bin_pred = {}) const
        {
            cur = saga::adjacent_find_fn{}(std::move(cur), bin_pred);

            if(!cur)
            {
                return cur;
            }

            auto out = cur;

            ++ cur;
            assert(!!cur);
            ++ cur;

            for(; !!cur; ++ cur)
            {
                if(!saga::invoke(bin_pred, *out, *cur))
                {
                    ++ out;
                    *out = std::move(*cur);
                }
            }

            ++ out;
            return out;
        }
    };

    template <class InputCursor, class OutputCursor>
    using unique_copy_result = in_out_result<InputCursor, OutputCursor>;

    struct unique_copy_fn
    {
        template <class InputCursor, class OutputCursor, class BinaryPredicate = std::equal_to<>>
        unique_copy_result<InputCursor, OutputCursor>
        operator()(InputCursor in, OutputCursor out, BinaryPredicate bin_pred = {}) const
        {
            if(!in || !out)
            {
                return {std::move(in), std::move(out)};
            }

            cursor_value_t<InputCursor> last_value = *in;
            ++ in;

            out << last_value;

            for(; !!in && !!out; ++ in)
            {
                if(!saga::invoke(bin_pred, last_value, *in))
                {
                    last_value = *in;
                    out << last_value;
                }
            }

            return {std::move(in), std::move(out)};
        }
    };

    struct is_partitioned_fn
    {
        template <class InputCursor, class UnaryPredicate>
        bool operator()(InputCursor cur, UnaryPredicate pred) const
        {
            cur = find_if_not_fn{}(std::move(cur), std::ref(pred));
            cur = find_if_fn{}(std::move(cur), std::move(pred));

            return !cur;
        }
    };

    struct partition_fn
    {
        template <class BidirectionalCursor, class UnaryPredicate>
        BidirectionalCursor operator()(BidirectionalCursor input, UnaryPredicate pred) const
        {
            input.forget_front();
            input.forget_back();

            for(;;)
            {
                if(!input)
                {
                    break;
                }

                if(saga::invoke(pred, input.front()))
                {
                    input.drop_front();
                    continue;
                }

                if(!saga::invoke(pred, input.back()))
                {
                    input.drop_back();
                    continue;
                }

                using std::swap;
                swap(input.front(), input.back());

                input.drop_front();

                if(!!input)
                {
                    input.drop_back();
                }
            }

            input.rewind_back();

            return input;
        }
    };

    template <class InputCursor, class OutputCursor1, class OutputCursor2>
    using partition_copy_result = in_out_out_result<InputCursor, OutputCursor1, OutputCursor2>;

    struct partition_copy_fn
    {
        template <class InputCursor, class OutputCursor1, class OutputCursor2, class UnaryPredicate>
        partition_copy_result<InputCursor, OutputCursor1, OutputCursor2>
        operator()(InputCursor in, OutputCursor1 out_true, OutputCursor2 out_false
                   , UnaryPredicate pred) const
        {
            for(;!!in && !!out_true && !!out_false; ++ in)
            {
                if(saga::invoke(pred, *in))
                {
                    out_true << *in;
                }
                else
                {
                    out_false << *in;
                }
            }

            return {std::move(in), std::move(out_true), std::move(out_false)};
        }
    };

    struct partition_point_fn
    {
        template <class ForwardCursor, class Predicate>
        ForwardCursor operator()(ForwardCursor cur, Predicate pred) const
        {
            assert(saga::is_partitioned_fn{}(cur, pred));

            if(!cur)
            {
                return cur;
            }

            auto num = saga::cursor::size(cur);

            do
            {
                auto const n1 = num / 2;
                auto const n2 = num - n1;

                assert(n2 > 0);

                auto const middle = saga::cursor::drop_front_n(cur, n1);

                if(saga::invoke(pred, *middle))
                {
                    cur = middle;
                    ++ cur;
                    num = n2 - 1;
                }
                else
                {
                    num = n1;
                }
            }
            while(num > 0);

            return cur;
        }
    };

    struct is_sorted_until_fn
    {
        template <class ForwardCursor, class Compare = std::less<>>
        ForwardCursor operator()(ForwardCursor cur, Compare cmp = {}) const
        {
            auto pred = [cmp = std::move(cmp)](auto && lhs, auto && rhs)
            {
                return cmp(rhs, lhs);
            };

            auto result = saga::adjacent_find_fn{}(std::move(cur), std::move(pred));

            if(!!result)
            {
                ++ result;
            }

            return result;
        }
    };

    struct is_sorted_fn
    {
        template <class ForwardCursor, class Compare = std::less<>>
        bool operator()(ForwardCursor cur, Compare cmp = {}) const
        {
            return !saga::is_sorted_until_fn{}(std::move(cur), std::move(cmp));
        }
    };

    struct lower_bound_fn
    {
        template <class ForwardCursor, class T, class Compare = std::less<>>
        ForwardCursor operator()(ForwardCursor cur, T const & value, Compare cmp = {}) const
        {
            auto pred = [&](auto && arg)
                { return saga::invoke(cmp, std::forward<decltype(arg)>(arg), value); };

            return saga::partition_point_fn{}(std::move(cur), std::move(pred));
        }
    };

    struct upper_bound_fn
    {
        template <class ForwardCursor, class T, class Compare = std::less<>>
        ForwardCursor operator()(ForwardCursor cur, T const & value, Compare cmp = {}) const
        {
            auto pred = [&](auto && arg)
                { return !saga::invoke(cmp, value, std::forward<decltype(arg)>(arg)); };

            return saga::partition_point_fn{}(std::move(cur), std::move(pred));
        }
    };

    struct equal_range_fn
    {
        template <class ForwardCursor, class T, class Compare = std::less<>>
        ForwardCursor operator()(ForwardCursor cur, T const & value, Compare cmp = {}) const
        {
            cur.forget_front();
            cur.forget_back();

            if(!cur)
            {
                return cur;
            }

            cur = saga::upper_bound_fn{}(std::move(cur), value, cmp);

            auto part1 = saga::lower_bound_fn{}(cur.dropped_front(), value, std::move(cmp));

            cur.forget_front();
            cur.exhaust_back();

            part1.splice(cur);

            return part1;
        }
    };

    struct binary_search_fn
    {
        template <class ForwardCursor, class T, class Compare = std::less<>>
        bool operator()(ForwardCursor cur, T const & value, Compare cmp = {}) const
        {
            auto pos = saga::lower_bound_fn{}(std::move(cur), value, std::move(cmp));

            return !!pos && !saga::invoke(cmp, value, *pos);
        }
    };

    template <class InputCursor1, class InputCursor2, class OutputCursor>
    using merge_result = in_in_out_result<InputCursor1, InputCursor2, OutputCursor>;

    struct merge_fn
    {
        template <class InputCursor1, class InputCursor2, class OutputCursor
                 , class Compare = std::less<>>
        merge_result<InputCursor1, InputCursor2, OutputCursor>
        operator()(InputCursor1 in1, InputCursor2 in2, OutputCursor out, Compare cmp = {}) const
        {
            for(; !!in1 && !!in2 && !!out; )
            {
                if(saga::invoke(cmp, *in2, *in1))
                {
                    out << *in2;
                    ++ in2;
                }
                else
                {
                    out << *in1;
                    ++ in1;
                }
            }

            auto result1 = saga::copy_fn{}(std::move(in1), std::move(out));
            auto result2 = saga::copy_fn{}(std::move(in2), std::move(result1.out));

            return {std::move(result1.in), std::move(result2.in), std::move(result2.out)};
        }
    };

    struct includes_fn
    {
        template <class InputCursor1, class InputCursor2, class Compare = std::less<>>
        bool operator()(InputCursor1 in1, InputCursor2 in2, Compare cmp = {}) const
        {
            for(; !!in1 && !!in2;)
            {
                if(saga::invoke(cmp, *in2, *in1))
                {
                    return false;
                }
                else if(saga::invoke(cmp, *in1, *in2))
                {
                    ++ in1;
                }
                else
                {
                    ++ in1;
                    ++ in2;
                }
            }

            return !in2;
        }
    };

    template <class InputCursor, class OutputCursor>
    using set_difference_result = in_out_result<InputCursor, OutputCursor>;

    struct set_difference_fn
    {
        template <class InputCursor1, class InputCursor2, class OutputCursor
                 , class Compare = std::less<>>
        set_difference_result<InputCursor1, OutputCursor>
        operator()(InputCursor1 in1, InputCursor2 in2, OutputCursor out
                        , Compare cmp = {}) const
        {
            for(;!!in1 && !!in2 && !!out;)
            {
                if(saga::invoke(cmp, *in1, *in2))
                {
                    out << *in1;
                    ++ in1;
                }
                else if(saga::invoke(cmp, *in2, *in1))
                {
                    ++ in2;
                }
                else
                {
                    ++ in1;
                    ++ in2;
                }
            }

            return saga::copy_fn{}(std::move(in1), std::move(out));
        }
    };

    template <class InputCursor1, class InputCursor2, class OutputCursor>
    using set_intersection_result = in_in_out_result<InputCursor1, InputCursor2, OutputCursor>;

    struct set_intersection_fn
    {
        template <class InputCursor1, class InputCursor2, class OutputCursor
                 , class Compare = std::less<>>
        set_intersection_result<InputCursor1, InputCursor2, OutputCursor>
        operator()(InputCursor1 in1, InputCursor2 in2, OutputCursor out
                        , Compare cmp = {}) const
        {
            for(; !!in1 && !!in2 && !!out;)
            {
                if(saga::invoke(cmp, *in1, *in2))
                {
                    ++ in1;
                }
                else if(saga::invoke(cmp, *in2, *in1))
                {
                    ++ in2;
                }
                else
                {
                    out << *in1;
                    ++ in1;
                    ++ in2;
                }
            }

            return {std::move(in1), std::move(in2), std::move(out)};
        }
    };

    template <class InputCursor1, class InputCursor2, class OutputCursor>
    using set_symmetric_difference_result
        = in_in_out_result<InputCursor1, InputCursor2, OutputCursor>;

    struct set_symmetric_difference_fn
    {
        template <class InputCursor1, class InputCursor2, class OutputCursor
                 , class Compare = std::less<>>
        set_symmetric_difference_result<InputCursor1, InputCursor2, OutputCursor>
        operator()(InputCursor1 in1, InputCursor2 in2, OutputCursor out
                        , Compare cmp = {}) const
        {
            for(; !!in1 && !!in2 && !!out;)
            {
                if(saga::invoke(cmp, *in1, *in2))
                {
                    out << *in1;
                    ++ in1;
                }
                else if(saga::invoke(cmp, *in2, *in1))
                {
                    out << *in2;
                    ++ in2;
                }
                else
                {
                    ++ in1;
                    ++ in2;
                }
            }

            auto result1 = saga::copy_fn{}(std::move(in1), std::move(out));
            auto result2 = saga::copy_fn{}(std::move(in2), std::move(result1.out));

            return {std::move(result1.in), std::move(result2.in), std::move(result2).out};
        }
    };

    template <class InputCursor1, class InputCursor2, class OutputCursor>
    using set_union_result = saga::in_in_out_result<InputCursor1, InputCursor2, OutputCursor>;

    struct set_union_fn
    {
        template <class InputCursor1, class InputCursor2, class OutputCursor
                 , class Compare = std::less<>>
        set_union_result<InputCursor1, InputCursor2, OutputCursor>
        operator()(InputCursor1 in1, InputCursor2 in2, OutputCursor out
                        , Compare cmp = {}) const
        {
            for(; !!in1 && !!in2 && !!out;)
            {
                if(saga::invoke(cmp, *in1, *in2))
                {
                    out << *in1;
                    ++ in1;
                }
                else if(saga::invoke(cmp, *in2, *in1))
                {
                    out << *in2;
                    ++ in2;
                }
                else
                {
                    out << *in1;
                    ++ in1;
                    ++ in2;
                }
            }

            auto result1 = saga::copy_fn{}(std::move(in1), std::move(out));
            auto result2 = copy_fn{}(std::move(in2), std::move(result1.out));

            return {std::move(result1.in), std::move(result2.in), std::move(result2.out)};
        }
    };

    namespace detail
    {
        template <class IntType>
        IntType heap_parent(IntType index)
        {
            return (index - 1)/2;
        }

        template <class IntType>
        IntType heap_second_child(IntType index)
        {
            return 2*(index + 1);
        }

        template <class RandomAccessCursor, class Distance, class T, class Compare>
        void push_heap(RandomAccessCursor input, Distance hole, Distance top_index
                       , T value, Compare & cmp)
        {
            auto parent = detail::heap_parent(hole);

            for(; hole > top_index && saga::invoke(cmp, input[parent], value);)
            {
                input[hole] = std::move(input[parent]);
                hole = parent;
                parent = detail::heap_parent(hole);
            }

            input[hole] = std::move(value);
        }

        template <class RandomAccessCursor, class Distance, class T, class Compare>
        void
        adjust_heap(RandomAccessCursor cur, Distance hole, Distance length, T value, Compare & cmp)
        {
            auto const top_index = hole;
            auto second_child = hole;

            while(second_child < (length - 1)/2)
            {
                second_child = saga::detail::heap_second_child(second_child);

                if(saga::invoke(cmp, cur[second_child], cur[second_child - 1]))
                {
                    -- second_child;
                }

                cur[hole] = std::move(cur[second_child]);
                hole = second_child;
            }

            if(length % 2 == 0 && second_child == (length - 2)/2)
            {
                second_child = saga::detail::heap_second_child(second_child);
                cur[hole] = std::move(cur[second_child - 1]);
                hole = second_child - 1;
            }

            saga::detail::push_heap(std::move(cur), hole, top_index, std::move(value), cmp);
        }
    }

    struct is_heap_until_fn
    {
        template <class RandomAccessCursor, class Compare = std::less<>>
        RandomAccessCursor
        operator()(RandomAccessCursor cur, Compare cmp = {}) const
        {
            if(!cur)
            {
                return cur;
            }

            auto const num = cur.size();
            auto index = 1 + 0*num;

            for(; index < num; ++index)
            {
                auto const parent = detail::heap_parent(index);

                if(saga::invoke(cmp, cur[parent], cur[index]))
                {
                    break;
                }
            }

            cur.drop_front(index);

            return cur;
        }
    };

    struct is_heap_fn
    {
        template <class RandomAccessCursor, class Compare = std::less<>>
        bool operator()(RandomAccessCursor cur, Compare cmp = {}) const
        {
            return !saga::is_heap_until_fn{}(std::move(cur), std::move(cmp));
        }
    };

    struct make_heap_fn
    {
        template <class RandomAccessCursor, class Compare = std::less<>>
        void operator()(RandomAccessCursor input, Compare cmp = {}) const
        {
            auto const len = input.size();

            if(len < 2)
            {
                return;
            }

            auto parent = (len - 2)/2;

            for(;; -- parent)
            {
                saga::detail::adjust_heap(input, parent, len, std::move(input[parent]), cmp);

                if(parent == 0)
                {
                    return;
                }
            }
        }
    };

    struct push_heap_fn
    {
        template <class RandomAccessCursor, class Compare = std::less<>>
        void operator()(RandomAccessCursor input, Compare cmp = {}) const
        {
            auto const num = input.size();

            assert(num > 0);

            detail::push_heap(std::move(input), num - 1, num*0, std::move(input[num-1]), cmp);
        }
    };

    struct pop_heap_fn
    {
        template <class RandomAccessCursor, class Compare = std::less<>>
        void operator()(RandomAccessCursor input, Compare cmp = {}) const
        {
            auto num = input.size();
            assert(num > 0);

            if(num > 1)
            {
                saga::cursor_value_t<RandomAccessCursor> value = std::move(input.back());

                input.back() = std::move(input.front());

                saga::detail::adjust_heap(input, 0*num, num-1, std::move(value), cmp);
            }
        }
    };

    struct sort_heap_fn
    {
        template <class RandomAccessCursor, class Compare = std::less<>>
        void operator()(RandomAccessCursor input, Compare cmp = {}) const
        {
            for(; !!input; input.drop_back())
            {
                saga::pop_heap_fn{}(input, cmp);
            }
        }
    };

    struct min_element_fn
    {
        template <class ForwardCursor, class Compare = std::less<>>
        ForwardCursor operator()(ForwardCursor input, Compare cmp = {}) const
        {
            if(!input)
            {
                return input;
            }

            auto result = input;
            ++ input;

            for(; !!input; ++input)
            {
                if(saga::invoke(cmp, *input, *result))
                {
                    result = input;
                }
            }

            return result;
        }
    };

    struct max_element_fn
    {
        template <class ForwardCursor, class Compare = std::less<>>
        ForwardCursor operator()(ForwardCursor input, Compare cmp = {}) const
        {
            return saga::min_element_fn{}(std::move(input), saga::not_fn(std::move(cmp)));

        }
    };

    template <class ForwardCursor>
    using minmax_element_result = min_max_result<ForwardCursor>;

    struct minmax_element_fn
    {
        template <class ForwardCursor, class Compare = std::less<>>
        minmax_element_result<ForwardCursor>
        operator()(ForwardCursor input, Compare cmp = {}) const
        {
            minmax_element_result<ForwardCursor> result{input, input};

            if(!input)
            {
                return result;
            }
            ++ input;

            for(;!!input;)
            {
                auto pos1 = input;

                auto pos2 = pos1;
                ++ pos2;

                if(!pos2)
                {
                    break;
                }

                if(saga::invoke(cmp, *pos2, *pos1))
                {
                    using std::swap;
                    swap(pos1, pos2);
                }

                if(saga::invoke(cmp, *pos1, *result.min))
                {
                    result.min = pos1;
                }

                if(!saga::invoke(cmp, *pos2, *result.max))
                {
                    result.max = pos2;
                }

                input = pos2;
                ++ input;
            }

            if(!!input)
            {
                if(saga::invoke(cmp, *input, *result.min))
                {
                    result.min = input;
                }
                else if(!saga::invoke(cmp, *input, *result.max))
                {
                    result.max = input;
                }
            }

            return result;
        }
    };

    struct clamp_fn
    {
        template <class T, class Compare = std::less<>>
        constexpr T const &
        operator()(T const & value, T const & low, T const & high, Compare cmp = {}) const
        {
            void(!cmp(low, high) ? void(0) : assert(!cmp(high, low)));

            if(saga::invoke(cmp, value, low))
            {
                return low;
            }
            else if(saga::invoke(cmp, high, value))
            {
                return high;
            }
            else
            {
                return value;
            }
        }
    };

    struct partial_sort_fn
    {
        template <class RandomAccessCursor, class Compare = std::less<>>
        void operator()(RandomAccessCursor input, Compare cmp = {}) const
        {
            auto out = input.dropped_front();

            if(!out)
            {
                return;
            }

            saga::make_heap_fn{}(out, cmp);

            auto const out_size = out.size();

            for(; !!input; ++ input)
            {
                if(saga::invoke(cmp, *input, *out))
                {
                    saga::cursor_value_t<RandomAccessCursor> value = std::move(*input);
                    *input = std::move(*out);

                    saga::detail::adjust_heap(out, 0*out_size, out_size, std::move(value), cmp);
                }
            }

            saga::sort_heap_fn{}(out, cmp);
        }
    };

    template <class InputCursor, class RandomAccessCursor>
    using partial_sort_copy_result = in_out_result<InputCursor, RandomAccessCursor>;

    struct partial_sort_copy_fn
    {
        template <class InputCursor, class RandomAccessCursor, class Compare = std::less<>>
        partial_sort_copy_result<InputCursor, RandomAccessCursor>
        operator()(InputCursor input, RandomAccessCursor out, Compare cmp = {}) const
        {
            if(!input || !out)
            {
                return {std::move(input), std::move(out)};
            }

            auto result = saga::copy_fn{}(std::move(input), out);

            auto const num = out.size() - result.out.size();

            auto const heap = saga::cursor::drop_back_n(std::move(out), result.out.size());

            saga::make_heap_fn{}(heap, cmp);

            assert(heap.size() == num);

            for(; !!result.in; ++ result.in)
            {
                assert(!!heap);

                if(saga::invoke(cmp, *result.in, *heap))
                {
                    saga::detail::adjust_heap(heap, 0*num, num, *result.in, cmp);
                }
            }

            saga::sort_heap_fn{}(std::move(heap), std::move(cmp));

            return result;
        }
    };

    struct equal_fn
    {
    private:
        template <class InputCursor1, class InputCursor2, class BinaryPredicate>
        constexpr bool impl(InputCursor1 cur1, InputCursor2 cur2, BinaryPredicate bin_pred
                            , std::input_iterator_tag, std::input_iterator_tag) const
        {
            auto const stop
                = saga::mismatch_fn{}(std::move(cur1), std::move(cur2), std::move(bin_pred));

            return !stop.in1 && !stop.in2;
        }

        template <class InputCursor1, class InputCursor2, class BinaryPredicate>
        constexpr bool impl(InputCursor1 cur1, InputCursor2 cur2, BinaryPredicate bin_pred
                            , std::random_access_iterator_tag
                            , std::random_access_iterator_tag) const
        {
            if(cur1.size() != cur2.size())
            {
                return false;
            }

            return this->impl(std::move(cur1), std::move(cur2), std::move(bin_pred)
                              , std::input_iterator_tag{}, std::input_iterator_tag{});
        }

    public:
        template <class InputCursor1, class InputCursor2, class BinaryPredicate = std::equal_to<>>
        constexpr bool operator()(InputCursor1 cur1, InputCursor2 cur2
                                  , BinaryPredicate bin_pred = {}) const
        {
            return equal_fn::impl(std::move(cur1), std::move(cur2), std::move(bin_pred)
                                  , cursor_category_t<InputCursor1>{}
                                  , cursor_category_t<InputCursor2>{});
        }
    };

    struct lexicographical_compare_fn
    {
        template <class InputCursor1, class InputCursor2, class Compare = std::less<>>
        bool operator()(InputCursor1 cur1, InputCursor2 cur2, Compare cmp = {}) const
        {
            for(; !!cur1 && !!cur2; ++cur1, (void)++cur2)
            {
                if(saga::invoke(cmp, *cur1, *cur2))
                {
                    return true;
                }
                else if(saga::invoke(cmp, *cur2, *cur1))
                {
                    return false;
                }
            }

            return !cur1 && !!cur2;
        }
    };

    struct is_permutation_fn
    {
        template <class ForwardCursor1, class ForwardCursor2
                 , class BinaryPredicate = std::equal_to<>>
        bool
        operator()(ForwardCursor1 cur1, ForwardCursor2 cur2, BinaryPredicate bin_pred = {}) const
        {
            // Пропускаем общую часть последовательностей
            auto rest = saga::mismatch_fn{}(std::move(cur1), std::move(cur2), bin_pred);

            if(!rest.in1 || !rest.in2)
            {
                return !rest.in1 && !rest.in2;
            }

            // Проверяем, что длины совпадают
            if(saga::cursor::size(rest.in1) != saga::cursor::size(rest.in2))
            {
                return false;
            }

            // Проверяем остальные элементы
            for(cur1 = rest.in1; !!cur1; ++cur1)
            {
                auto pred
                    = [&](auto && arg) { return bin_pred(*cur1, std::forward<decltype(arg)>(arg));};

                auto const n1 = saga::count_if_fn{}(rest.in1, pred);
                auto const n2 = saga::count_if_fn{}(rest.in2, pred);

                if(n1 != n2)
                {
                    return false;
                }
            }

            return true;
        }
    };

    struct starts_with_fn
    {
        template <typename InputRange1, typename InputRange2>
        bool operator()(InputRange1 const & input, InputRange2 const & test) const
        {
            auto const test_end = saga::end(test);
            auto const pos = std::mismatch(saga::begin(input), saga::end(input),
                                           saga::begin(test), test_end);

            return pos.second == test_end;
        }
    };

    struct ends_with_fn
    {
    public:
        template <typename BidirectionalRange1, typename BidirectionalRange2>
        bool operator()(BidirectionalRange1 const & input, BidirectionalRange2 const & test) const
        {
            auto const test_end = saga::rend(test);
            auto const pos = std::mismatch(saga::rbegin(input), saga::rend(input),
                                           saga::rbegin(test), test_end);

            return pos.second == test_end;
        }
    };

    struct for_n_fn
    {
    public:
        template <class Size, class NullaryFunction>
        constexpr
        NullaryFunction
        operator()(Size num, NullaryFunction fun) const
        {
            assert(num >= 0);

            for(; num > 0; -- num)
            {
                saga::invoke(fun);
            }

            return fun;
        }
    };

    template <class InputCursor, class OutputCursor>
    using unary_transform_result = in_out_result<InputCursor, OutputCursor>;

    template <class InputCursor1, class InputCursor2, class OutputCursor>
    using binary_transform_result = in_in_out_result<InputCursor1, InputCursor2, OutputCursor>;

    struct transform_fn
    {
    public:
        template <class InputCursor, class OutputCursor, class UnaryFunction>
        constexpr
        unary_transform_result<InputCursor, OutputCursor>
        operator()(InputCursor input, OutputCursor output, UnaryFunction fun) const
        {
            for(; !!input && !!output; ++input)
            {
                output << saga::invoke(fun, *input);
            }

            return {std::move(input), std::move(output)};
        }

        template <class InputCursor1, class InputCursor2, class OutputCursor, class BinaryFunction>
        constexpr
        binary_transform_result<InputCursor1, InputCursor2, OutputCursor>
        operator()(InputCursor1 in1, InputCursor2 in2, OutputCursor out
                   , BinaryFunction bin_fun) const
        {
            for(; !!in1 && !!in2 && !!out; (void)++in1, ++in2)
            {
                out << saga::invoke(bin_fun, *in1, *in2);
            }

            return {std::move(in1), std::move(in2), std::move(out)};
        }
    };

    // Функциональные объекты
    namespace
    {
        constexpr auto const & all_of = detail::static_empty_const<all_of_fn>::value;
        constexpr auto const & any_of = detail::static_empty_const<any_of_fn>::value;
        constexpr auto const & none_of = detail::static_empty_const<none_of_fn>::value;

        constexpr auto const & for_each = detail::static_empty_const<for_each_fn>::value;
        constexpr auto const & for_each_n = detail::static_empty_const<for_each_n_fn>::value;

        constexpr auto const & count = detail::static_empty_const<count_fn>::value;
        constexpr auto const & count_if = detail::static_empty_const<count_if_fn>::value;

        constexpr auto const & mismatch = detail::static_empty_const<mismatch_fn>::value;

        constexpr auto const & find = detail::static_empty_const<find_fn>::value;
        constexpr auto const & find_if = detail::static_empty_const<find_if_fn>::value;
        constexpr auto const & find_if_not = detail::static_empty_const<find_if_not_fn>::value;
        constexpr auto const & find_end = detail::static_empty_const<find_end_fn>::value;
        constexpr auto const & find_first_of = detail::static_empty_const<find_first_of_fn>::value;
        constexpr auto const & adjacent_find = detail::static_empty_const<adjacent_find_fn>::value;
        constexpr auto const & search = detail::static_empty_const<search_fn>::value;
        constexpr auto const & search_n = detail::static_empty_const<search_n_fn>::value;

        constexpr auto const & copy = detail::static_empty_const<copy_fn>::value;
        constexpr auto const & copy_if = detail::static_empty_const<copy_if_fn>::value;
        constexpr auto const & copy_n = detail::static_empty_const<copy_n_fn>::value;
        constexpr auto const & copy_backward = detail::static_empty_const<copy_backward_fn>::value;
        constexpr auto const & move = detail::static_empty_const<move_fn>::value;

        constexpr auto const & fill = detail::static_empty_const<fill_fn>::value;
        constexpr auto const & fill_n = detail::static_empty_const<fill_n_fn>::value;
        constexpr auto const & transform = detail::static_empty_const<transform_fn>::value;
        constexpr auto const & generate = detail::static_empty_const<generate_fn>::value;
        constexpr auto const & generate_n = detail::static_empty_const<generate_n_fn>::value;
        constexpr auto const & remove = detail::static_empty_const<remove_fn>::value;
        constexpr auto const & remove_if = detail::static_empty_const<remove_if_fn>::value;
        constexpr auto const & remove_copy = detail::static_empty_const<remove_copy_fn>::value;
        constexpr auto const & remove_copy_if
            = detail::static_empty_const<remove_copy_if_fn>::value;
        constexpr auto const & replace = detail::static_empty_const<replace_fn>::value;
        constexpr auto const & replace_if = detail::static_empty_const<replace_if_fn>::value;
        constexpr auto const & replace_copy = detail::static_empty_const<replace_copy_fn>::value;
        constexpr auto const & replace_copy_if
            = detail::static_empty_const<replace_copy_if_fn>::value;
        constexpr auto const & swap_ranges = detail::static_empty_const<swap_ranges_fn>::value;
        constexpr auto const & reverse = detail::static_empty_const<reverse_fn>::value;
        constexpr auto const & reverse_copy = detail::static_empty_const<reverse_copy_fn>::value;
        constexpr auto const & rotate_copy = detail::static_empty_const<rotate_copy_fn>::value;
        constexpr auto const & shuffle = detail::static_empty_const<shuffle_fn>::value;
        constexpr auto const & sample = detail::static_empty_const<sample_fn>::value;
        constexpr auto const & unique = detail::static_empty_const<unique_fn>::value;
        constexpr auto const & unique_copy = detail::static_empty_const<unique_copy_fn>::value;

        constexpr auto const & is_partitioned
            = detail::static_empty_const<is_partitioned_fn>::value;
        constexpr auto const & partition = detail::static_empty_const<partition_fn>::value;
        constexpr auto const & partition_copy
            = detail::static_empty_const<partition_copy_fn>::value;
        constexpr auto const & partition_point
            = detail::static_empty_const<partition_point_fn>::value;

        constexpr auto const & is_sorted = detail::static_empty_const<is_sorted_fn>::value;
        constexpr auto const & is_sorted_until
            = detail::static_empty_const<is_sorted_until_fn>::value;

        constexpr auto const & partial_sort = detail::static_empty_const<partial_sort_fn>::value;
        constexpr auto const & partial_sort_copy
            = detail::static_empty_const<partial_sort_copy_fn>::value;

        constexpr auto const & lower_bound = detail::static_empty_const<lower_bound_fn>::value;
        constexpr auto const & upper_bound = detail::static_empty_const<upper_bound_fn>::value;
        constexpr auto const & equal_range = detail::static_empty_const<equal_range_fn>::value;
        constexpr auto const & binary_search = detail::static_empty_const<binary_search_fn>::value;

        constexpr auto const & merge = detail::static_empty_const<merge_fn>::value;

        constexpr auto const & includes = detail::static_empty_const<includes_fn>::value;
        constexpr auto const & set_difference
            = detail::static_empty_const<set_difference_fn>::value;
        constexpr auto const & set_intersection
            = detail::static_empty_const<set_intersection_fn>::value;
        constexpr auto const & set_symmetric_difference
            = detail::static_empty_const<set_symmetric_difference_fn>::value;
        constexpr auto const & set_union = detail::static_empty_const<set_union_fn>::value;

        constexpr auto const & is_heap = detail::static_empty_const<is_heap_fn>::value;
        constexpr auto const & is_heap_until = detail::static_empty_const<is_heap_until_fn>::value;
        constexpr auto const & make_heap = detail::static_empty_const<make_heap_fn>::value;
        constexpr auto const & push_heap = detail::static_empty_const<push_heap_fn>::value;
        constexpr auto const & pop_heap = detail::static_empty_const<pop_heap_fn>::value;
        constexpr auto const & sort_heap = detail::static_empty_const<sort_heap_fn>::value;

        constexpr auto const & min_element = detail::static_empty_const<min_element_fn>::value;
        constexpr auto const & max_element = detail::static_empty_const<max_element_fn>::value;
        constexpr auto const & minmax_element
            = detail::static_empty_const<minmax_element_fn>::value;
        constexpr auto const & clamp = detail::static_empty_const<clamp_fn>::value;

        constexpr auto const & equal = detail::static_empty_const<equal_fn>::value;
        constexpr auto const & lexicographical_compare
            = detail::static_empty_const<lexicographical_compare_fn>::value;

        constexpr auto const & is_permutation
            = detail::static_empty_const<is_permutation_fn>::value;

        constexpr auto const & starts_with = detail::static_empty_const<starts_with_fn>::value;
        constexpr auto const & ends_with = detail::static_empty_const<ends_with_fn>::value;

        constexpr auto const & for_n = detail::static_empty_const<for_n_fn>::value;
    }
}
// namespace saga

#endif
// Z_SAGA_ALGORITHM_HPP_INCLUDED
