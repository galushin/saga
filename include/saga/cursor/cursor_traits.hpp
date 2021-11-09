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

#include <saga/detail/static_empty_const.hpp>

#include <cassert>
#include <iterator>

namespace saga
{
    template <class Cursor>
    using cursor_reference_t = typename Cursor::reference;

    template <class Cursor>
    using cursor_difference_t = typename Cursor::difference_type;

    template <class Cursor>
    using cursor_category_t = typename Cursor::cursor_category;

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
                void operator()(Cursor & cur, cursor_difference_t<Cursor> num
                                , std::forward_iterator_tag) const
                {
                    for(; num > 0; -- num)
                    {
                        cur.drop_front();
                    }
                }

                template <class Cursor>
                void operator()(Cursor & cur, cursor_difference_t<Cursor> num
                                , std::random_access_iterator_tag) const
                {
                    cur.drop_front(num);
                }

            public:
                template <class Cursor>
                void operator()(Cursor & cur, cursor_difference_t<Cursor> num) const
                {
                    assert(num >= 0);

                    return (*this)(cur, num, cursor_category_t<Cursor>{});
                }
            };

            struct drop_back_n_fn
            {
            private:
                template <class Cursor>
                void operator()(Cursor & cur, cursor_difference_t<Cursor> num
                                , std::bidirectional_iterator_tag) const
                {
                    for(; num > 0; -- num)
                    {
                        cur.drop_back();
                    }
                }

                template <class Cursor>
                void operator()(Cursor & cur, cursor_difference_t<Cursor> num
                                , std::random_access_iterator_tag) const
                {
                    cur.drop_back(num);
                }

            public:
                template <class Cursor>
                void operator()(Cursor & cur, cursor_difference_t<Cursor> num) const
                {
                    assert(num >= 0);

                    return (*this)(cur, num, cursor_category_t<Cursor>{});
                }
            };
        }
        // namespace detail

        namespace
        {
            constexpr auto const & drop_front_n
                = saga::detail::static_empty_const<detail::drop_front_n_fn>::value;

            constexpr auto const & drop_back_n
                = saga::detail::static_empty_const<detail::drop_back_n_fn>::value;

            constexpr auto const & size
                = saga::detail::static_empty_const<detail::size_fn>::value;
        }
    }
    //namespace cursor
}
// namespace saga

#endif
// Z_SAGA_CURSOR_CURSOR_TRAITS_HPP_INCLUDED
