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

#ifndef Z_SAGA_CURSOR_CURSOR_TRAITS_HPP_INCLUDED
#define Z_SAGA_CURSOR_CURSOR_TRAITS_HPP_INCLUDED

#include <cassert>
#include <iterator>

namespace saga
{
    struct any_cursor_cardinality_tag
    {};

    struct finite_cursor_cardinality_tag
     : any_cursor_cardinality_tag
    {};

    struct infinite_cursor_cardinality_tag
     : any_cursor_cardinality_tag
    {};

    template <class Cursor>
    using cursor_reference_t = typename Cursor::reference;

    template <class Cursor>
    using cursor_difference_t = typename Cursor::difference_type;

    template <class Cursor>
    using cursor_category_t = typename Cursor::cursor_category;

    template <class Cursor>
    using cursor_cardinality_t = typename Cursor::cursor_cardinality;

    template <class Cursor>
    using cursor_value_t = typename Cursor::value_type;

    namespace cursor
    {
        namespace detail
        {
            struct size_fn
            {
            private:
                template <class Cursor>
                saga::cursor_difference_t<Cursor>
                operator()(Cursor cur, std::input_iterator_tag) const
                {
                    auto num = typename Cursor::difference_type(0);

                    for(; !!cur; ++ cur)
                    {
                        ++ num;
                    }

                    return num;
                }

                template <class Cursor>
                saga::cursor_difference_t<Cursor>
                operator()(Cursor cur, std::random_access_iterator_tag) const
                {
                    return cur.size();
                }

            public:
                template <class Cursor>
                saga::cursor_difference_t<Cursor>
                operator()(Cursor cur) const
                {
                    return (*this)(std::move(cur), cursor_category_t<Cursor>{});
                }
            };

            struct drop_front_n_fn
            {
            private:
                template <class Cursor>
                static Cursor
                impl(Cursor cur, cursor_difference_t<Cursor> num, std::input_iterator_tag)
                {
                    for(; num > 0; -- num)
                    {
                        cur.drop_front();
                    }

                    return cur;
                }

                template <class Cursor>
                static constexpr Cursor
                impl(Cursor cur, cursor_difference_t<Cursor> num, std::random_access_iterator_tag)
                {
                    cur.drop_front(num);

                    return cur;
                }

            public:
                template <class Cursor>
                constexpr
                Cursor operator()(Cursor cur, cursor_difference_t<Cursor> num) const
                {
                    assert(num >= 0);

                    return this->impl(std::move(cur), std::move(num), cursor_category_t<Cursor>{});
                }
            };

            struct drop_back_n_fn
            {
            private:
                template <class Cursor>
                Cursor operator()(Cursor cur, cursor_difference_t<Cursor> num
                                , std::bidirectional_iterator_tag) const
                {
                    for(; num > 0; -- num)
                    {
                        cur.drop_back();
                    }

                    return cur;
                }

                template <class Cursor>
                Cursor operator()(Cursor cur, cursor_difference_t<Cursor> num
                                , std::random_access_iterator_tag) const
                {
                    cur.drop_back(num);

                    return cur;
                }

            public:
                template <class Cursor>
                Cursor operator()(Cursor cur, cursor_difference_t<Cursor> num) const
                {
                    assert(num >= 0);

                    return (*this)(std::move(cur), std::move(num), cursor_category_t<Cursor>{});
                }
            };

            namespace swap_adl
            {
                template <class T>
                void swap(T &, T &) = delete;

                struct swap_fn
                {
                    template <class T>
                    void operator()(T && lhs, T && rhs) const
                    noexcept(std::is_nothrow_swappable<std::remove_reference_t<T>>{})
                    {
                        using std::swap;
                        return swap(std::forward<T>(lhs), std::forward<T>(rhs));
                    }
                };
            }
            // namespace swap_adl
        }
        // namespace detail

        inline constexpr auto drop_front_n = detail::drop_front_n_fn{};
        inline constexpr auto drop_back_n = detail::drop_back_n_fn{};
        inline constexpr auto size = detail::size_fn{};
        inline constexpr auto swap = detail::swap_adl::swap_fn{};
    }
    //namespace cursor

    namespace detail
    {
        template <class Type, class SFINASE = void>
        struct is_input_cursor
         : std::false_type
        {};

        template <class Type>
        struct is_input_cursor<Type, std::void_t<typename Type::cursor_category>>
         : std::is_base_of<std::input_iterator_tag, saga::cursor_category_t<Type>>
        {};
    }
    // namespace detail

    template <class Type>
    using is_input_cursor = detail::is_input_cursor<Type>;

    template <class Cursor>
    struct is_forward_cursor
     : std::is_base_of<std::forward_iterator_tag, saga::cursor_category_t<Cursor>>
    {};

    template <class Cursor>
    struct is_random_access_cursor
     : std::is_base_of<std::random_access_iterator_tag, saga::cursor_category_t<Cursor>>
    {};
}
// namespace saga

#endif
// Z_SAGA_CURSOR_CURSOR_TRAITS_HPP_INCLUDED
