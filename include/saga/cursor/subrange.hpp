/* (c) 2021-2022 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_CURSOR_SUBRANGE_HPP_INCLUDED
#define Z_SAGA_CURSOR_SUBRANGE_HPP_INCLUDED

/** @file saga/cursor/subrange.hpp
 @brief Курсор, проходящий интервал, заданный итератором и часовым
*/

#include <saga/cursor/cursor_traits.hpp>
#include <saga/cursor/cursor_facade.hpp>
#include <saga/defs.hpp>
#include <saga/iterator.hpp>
#include <saga/ranges.hpp>

#include <cassert>
#include <iterator>

namespace saga
{
    template <class ForwardIterator, class Sentinel = ForwardIterator>
    class subrange_cursor
     : saga::cursor_facade<subrange_cursor<ForwardIterator, Sentinel>
                          , typename std::iterator_traits<ForwardIterator>::reference>
    {
        friend constexpr bool operator==(subrange_cursor const & lhs, subrange_cursor const & rhs)
        {
            return lhs.begin() == rhs.begin()
                    && lhs.end() == rhs.end()
                    && lhs.cur_old_ == rhs.cur_old_
                    && lhs.last_old_ == rhs.last_old_;
        }

    public:
        // Типы
        using cursor_category = typename std::iterator_traits<ForwardIterator>::iterator_category;
        using cursor_cardinality
            = saga::conditional_t<std::is_same<Sentinel, saga::unreachable_sentinel_t>{}
                                 , saga::infinite_cursor_cardinality_tag
                                 , saga::finite_cursor_cardinality_tag>;
        using difference_type = typename std::iterator_traits<ForwardIterator>::difference_type;
        using value_type = typename std::iterator_traits<ForwardIterator>::value_type;
        using pointer = typename std::iterator_traits<ForwardIterator>::pointer;
        using reference = typename std::iterator_traits<ForwardIterator>::reference;
        using iterator = ForwardIterator;

        // Создание, копирование, уничтожение
        constexpr subrange_cursor(ForwardIterator first, Sentinel last, unsafe_tag_t)
         : cur_old_(std::move(first))
         , last_old_(std::move(last))
         , cur_(this->cur_old_)
         , last_(this->last_old_)
         , back_(this->last_)
        {
            this->tweak_back();
        }

        // Итераторы
        constexpr iterator begin() const
        {
            return this->cur_;
        }

        constexpr Sentinel end() const
        {
            return this->last_;
        }

        // Курсор ввода
        constexpr bool operator!() const
        {
            return this->cur_ == this->last_;
        }

        constexpr void drop_front()
        {
            assert(!!*this);

            ++ this->cur_;
        }

        constexpr reference front() const
        {
            assert(!!*this);

            return *this->cur_;
        }

        // Прямой курсор
        subrange_cursor dropped_front() const
        {
            return subrange_cursor(this->cur_old_, this->cur_, unsafe_tag_t{});
        }

        void exhaust_front()
        {
            this->cur_ = this->last_;
        }

        void exhaust_back()
        {
            this->last_ = this->cur_;
            this->back_ = this->cur_;
        }

        void forget_front()
        {
            this->cur_old_ = this->cur_;
        }

        void splice(subrange_cursor const & other)
        {
            assert(this->end() == other.begin());

            this->back_ = other.back_;
            this->last_ = other.last_;
            this->last_old_ = other.last_old_;

            if(this->back_ == this->last_)
            {
                this->tweak_back();
            }
        }

        void rewind_front()
        {
            this->cur_ = this->cur_old_;
        }

        // Двунаправленный курсор
        constexpr void drop_back()
        {
            assert(!!*this);

            this->last_ = this->back_;

            if(!!*this)
            {
                --this->back_;
            }
        }

        constexpr reference back() const
        {
            assert(!!*this);
            assert(this->back_ != this->last_);

            return *this->back_;
        }

        subrange_cursor dropped_back() const
        {
            return subrange_cursor(this->last_, this->last_old_, saga::unsafe_tag_t{});
        }

        void rewind_back()
        {
            this->last_ = this->last_old_;
            this->back_ = this->last_;
            this->tweak_back();
        }

        void forget_back()
        {
            this->last_old_ = this->last_;
        }

        // Курсор произвольного доступа
        constexpr difference_type size() const
        {
            return this->end() - this->begin();
        }

        constexpr void drop_front(difference_type num)
        {
            assert(0 <= num && num <= this->size());

            this->cur_ += num;
        }

        void drop_back(difference_type num)
        {
            assert(0 <= num && num <= this->size());

            this->last_ -= num;
            this->back_ -= num;
        }

        reference operator[](difference_type index) const
        {
            assert(0 <= index && index < this->size());

            return this->cur_[index];
        }

    private:
        constexpr void tweak_back()
        {
            return this->tweak_back(typename std::iterator_traits<Sentinel>::iterator_category{});
        }

        constexpr void tweak_back(std::input_iterator_tag)
        {}

        constexpr void tweak_back(std::bidirectional_iterator_tag)
        {
            if(this->back_ != this->cur_)
            {
                -- back_;
            }
        }

    private:
        ForwardIterator cur_old_;
        ForwardIterator last_old_;
        ForwardIterator cur_;
        Sentinel last_;
        Sentinel back_;
    };

    template <class ForwardIterator>
    constexpr auto make_subrange_cursor(ForwardIterator first, ForwardIterator last, unsafe_tag_t)
    -> subrange_cursor<ForwardIterator>
    {
        return subrange_cursor<ForwardIterator>(std::move(first), std::move(last), unsafe_tag_t{});
    }

    namespace cursor
    {
        template <class ForwardRange
                 , std::enable_if_t<saga::is_borrowed_range<ForwardRange>{}> * = nullptr>
        constexpr auto all(ForwardRange && range)
        {
            return ::saga::make_subrange_cursor(saga::begin(range), saga::end(range)
                                               ,unsafe_tag_t{});
        }
    }
    // namespace cursor

    /** @brief Перенос итератора с одного контейнера на другой
    @pre pos - является итератором для контейнера src
    @pre dest содержит не меньше элементов, чем std::distance(src.begin(), pos)
    */
    template <class Container1, class Container2>
    auto rebase_iterator(typename Container1::const_iterator pos
                        , Container1 const & src, Container2 && dest, unsafe_tag_t)
    {
        return std::next(dest.begin(), std::distance(src.begin(), pos));
    }

    namespace detail
    {
        template <class ForwardCursor1, class ForwardCursor2>
        ForwardCursor2
        rebase_cursor(ForwardCursor1 src, ForwardCursor2 dest, std::forward_iterator_tag)
        {
            auto const n_before = saga::cursor::size(src.dropped_front());
            auto const n_body = saga::cursor::size(src);

            auto const n_result = saga::cursor::size(dest);

            assert(n_before + n_body <= n_result);

            auto after = saga::cursor::drop_front_n(std::move(dest), n_before + n_body);
            dest = after.dropped_front();
            after.exhaust_back();
            dest.splice(after);

            return saga::cursor::drop_front_n(std::move(dest), n_before);
        }

        template <class ForwardCursor, class BidirectionalCursor>
        BidirectionalCursor
        rebase_cursor(ForwardCursor src, BidirectionalCursor dest, std::bidirectional_iterator_tag)
        {
            auto const n_front = saga::cursor::size(src.dropped_front());
            auto const n_not_back = n_front + saga::cursor::size(src);

            auto const n_result = saga::cursor::size(dest);

            assert(n_not_back <= n_result);

            auto const n_back = n_result - n_not_back;

            dest = saga::cursor::drop_front_n(std::move(dest), n_front);
            dest = saga::cursor::drop_back_n(std::move(dest), n_back);

            return dest;
        }
    }

    /** @brief Перенос курсора на другой контейнер
    @pre dest содержит не меньше элементов, чем cur и cur.dropped_front()
    */
    template <class Cursor, class Container>
    auto rebase_cursor(Cursor cur, Container && dest)
    {
        auto cur_dest = saga::cursor::all(dest);
        return saga::detail::rebase_cursor(std::move(cur), std::move(cur_dest)
                                           , saga::cursor_category_t<decltype(cur_dest)>{});
    }
}
// namespace saga

#endif
// Z_SAGA_CURSOR_SUBRANGE_HPP_INCLUDED
