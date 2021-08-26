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

#include <saga/functional.hpp>
#include <saga/iterator.hpp>
#include <saga/cursor/cursor_traits.hpp>

#include <saga/detail/static_empty_const.hpp>

#include <algorithm>

namespace saga
{
    // Типы результатов алгоритмов
    template <class Input, class Output>
    struct in_out_result
    {
        Input in;
        Output out;
    };

    template <class Input1, class Input2, class Output>
    struct in_in_out_result
    {
        Input1 in1;
        Input2 in2;
        Output out;
    };

    // Немодифицирующие операции
    struct find_if_fn
    {
        template <class InputCursor, class UnaryPredicate>
        InputCursor operator()(InputCursor cur, UnaryPredicate pred) const
        {
            for(; !!cur; ++cur)
            {
                if(saga::invoke(pred,*cur))
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

    struct count_if_fn
    {
        template <class InputCursor, class UnaryPredicate>
        cursor_difference<InputCursor>
        operator()(InputCursor cur, UnaryPredicate pred) const
        {
            auto result = cursor_difference<InputCursor>(0);

            for(; !!cur; ++ cur)
            {
                if(pred(*cur))
                {
                    ++ result;
                }
            }

            return result;
        }
    };

    struct count_fn
    {
        template <class InputCursor, class T>
        cursor_difference<InputCursor>
        operator()(InputCursor cur, T const & value) const
        {
            auto pred = [&value](auto && arg) { return std::forward<decltype(arg)>(arg) == value; };

            return count_if_fn{}(std::move(cur), std::move(pred));
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

    struct generate_fn
    {
        template <class OutputCursor, class Generator>
        void operator()(OutputCursor cur, Generator gen) const
        {
            for(; !!cur; ++ cur)
            {
                *cur = gen();
            }
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

    struct includes_fn
    {
        template <class InputCursor1, class InputCursor2, class Compare = std::less<>>
        bool operator()(InputCursor1 in1, InputCursor2 in2, Compare cmp = Compare()) const
        {
            for(; !!in1 && !!in2;)
            {
                if(cmp(*in2, *in1))
                {
                    return false;
                }
                else if(cmp(*in1, *in2))
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

    struct equal_fn
    {
        template <class InputCursor1, class InputCursor2, class BinaryPredicate = std::equal_to<>>
        constexpr bool operator()(InputCursor1 cur1, InputCursor2 cur2
                                  , BinaryPredicate bin_pred = {}) const
        {
            for(; !!cur1 && !!cur2; ++cur1, (void)++cur2)
            {
                if(!saga::invoke(bin_pred, *cur1, *cur2))
                {
                    return false;
                }
            }

            return !cur1 && !cur2;
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
                fun();
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
                output << fun(*input);
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
                out << bin_fun(*in1, *in2);
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

        constexpr auto const & count = detail::static_empty_const<count_fn>::value;
        constexpr auto const & count_if = detail::static_empty_const<count_if_fn>::value;

        constexpr auto const & copy = detail::static_empty_const<copy_fn>::value;
        constexpr auto const & copy_if = detail::static_empty_const<copy_if_fn>::value;

        constexpr auto const & fill = detail::static_empty_const<fill_fn>::value;
        constexpr auto const & transform = detail::static_empty_const<transform_fn>::value;
        constexpr auto const & generate = detail::static_empty_const<generate_fn>::value;
        constexpr auto const & reverse = detail::static_empty_const<reverse_fn>::value;
        constexpr auto const & reverse_copy = detail::static_empty_const<reverse_copy_fn>::value;

        constexpr auto const & includes = detail::static_empty_const<includes_fn>::value;

        constexpr auto const & equal = detail::static_empty_const<equal_fn>::value;

        constexpr auto const & lexicographical_compare
            = detail::static_empty_const<lexicographical_compare_fn>::value;

        constexpr auto const & starts_with = detail::static_empty_const<starts_with_fn>::value;
        constexpr auto const & ends_with = detail::static_empty_const<ends_with_fn>::value;

        constexpr auto const & for_n = detail::static_empty_const<for_n_fn>::value;
    }
}
// namespace saga

#endif
// Z_SAGA_ALGORITHM_HPP_INCLUDED
