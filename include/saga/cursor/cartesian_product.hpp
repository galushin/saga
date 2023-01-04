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

#ifndef Z_SAGA_CURSOR_CARTESIAN_PRODUCT_HPP_INCLUDED
#define Z_SAGA_CURSOR_CARTESIAN_PRODUCT_HPP_INCLUDED

/** @file saga/cursor/cartesian_product.hpp
 @brief Курсор, последовательность элементов которого представляет собой декартово произведение
 последовательностей элементов курсоров-аргументов
*/

#include <saga/cursor/cursor_facade.hpp>
#include <saga/cursor/cycle.hpp>

namespace saga
{
    namespace detail
    {
        template <class... Types>
        struct tuple_or_pair
        {
            using type = std::tuple<Types...>;
        };

        template <class T1, class T2>
        struct tuple_or_pair<T1, T2>
        {
            using type = std::pair<T1, T2>;
        };
    }

    template <class... Types>
    using tuple_or_pair = typename detail::tuple_or_pair<Types...>::type;

    template <class InputCursor, class... ForwardCursors>
    class cartesian_product_cursor
     : saga::cursor_facade<cartesian_product_cursor<InputCursor, ForwardCursors...>
                          , tuple_or_pair<cursor_reference_t<InputCursor>
                                         ,cursor_reference_t<ForwardCursors>...>>
    {
    public:
        // Типы
        using reference = tuple_or_pair<cursor_reference_t<InputCursor>
                                       ,cursor_reference_t<ForwardCursors>...>;
        using difference_type = std::ptrdiff_t;
        using cursor_category = std::input_iterator_tag;

        // Создание
        explicit cartesian_product_cursor(InputCursor cur1, ForwardCursors... others)
         : curs_(std::move(cur1), std::move(others)...)
        {
            this->forget_fronts_impl(std::index_sequence_for<ForwardCursors...>{});
        }

        // Курсор ввода
        bool operator!() const
        {
            return this->is_empty_impl(std::make_index_sequence<sizeof...(ForwardCursors) + 1>{});
        }

        reference front() const
        {
            assert(!!*this);

            return this->front_impl(std::make_index_sequence<sizeof...(ForwardCursors) + 1>{});
        }

        void drop_front()
        {
            assert(!!*this);

            this->increment_impl(std::integral_constant<std::size_t, 0>{});
        }

    private:
        template <std::size_t... Indices>
        void forget_fronts_impl(std::index_sequence<Indices...>)
        {
            static_assert(sizeof...(Indices) == sizeof...(ForwardCursors));

            (std::get<Indices+1>(this->curs_).forget_front(),...);
        }

        template <std::size_t... Indices>
        bool is_empty_impl(std::index_sequence<Indices...>) const
        {
            static_assert(sizeof...(Indices) == sizeof...(ForwardCursors) + 1);

            return (!std::get<Indices>(this->curs_) || ...);
        }

        template <std::size_t... Indices>
        reference front_impl(std::index_sequence<Indices...>) const
        {
            static_assert(sizeof...(Indices) == sizeof...(ForwardCursors) + 1);

            return reference(std::get<Indices>(this->curs_).front()...);
        }

        template <std::size_t Index>
        bool increment_impl(std::integral_constant<std::size_t, Index>)
        {
            if(!this->increment_impl(std::integral_constant<std::size_t, Index+1>{}))
            {
                return false;
            }

            std::get<Index>(this->curs_).drop_front();

            if constexpr(Index != 0)
            {
                if(!std::get<Index>(this->curs_))
                {
                    std::get<Index>(this->curs_)
                        = std::move(std::get<Index>(this->curs_)).dropped_front();

                    return true;
                }
                else
                {
                    return false;
                }
            }

            return false;
        }

        bool increment_impl(std::integral_constant<std::size_t, sizeof...(ForwardCursors) + 1>)
        {
            return true;
        }

    private:
        std::tuple<InputCursor, ForwardCursors...> curs_;
    };

    namespace cursor
    {
        template <class InputCursor, class... ForwardCursors>
        cartesian_product_cursor<InputCursor, ForwardCursors...>
        cartesian_product(InputCursor cur1, ForwardCursors... curs)
        {
            using Result = cartesian_product_cursor<InputCursor, ForwardCursors...>;
            return Result(std::move(cur1), std::move(curs)...);
        }

    }
    // namespace cursor
}
// namespace saga

#endif
// Z_SAGA_CURSOR_CARTESIAN_PRODUCT_HPP_INCLUDED
