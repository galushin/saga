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

#ifndef Z_SAGA_ITERATOR_REVERSE_HPP_INCLUDED
#define Z_SAGA_ITERATOR_REVERSE_HPP_INCLUDED

/** @file saga/iterator/reverse.hpp
 @brief Адаптор итератора, обходящий базовую последовательность в обратном направлении.
*/

#include <saga/utility/operators.hpp>

#include <iterator>

namespace saga
{
    /** @brief Адаптор итератора, проходящий базовую последовательность в обратном направлении

    Так как у std::reverse_iterator<T> есть неявное преобразование в std::reverse_iterator<U>, когда
    есть неявное преобразование T в U, то у saga::reverse_iterator<T> есть неявное преобразование
    из и в std::reverse_iterator<U>, когда, есть преобразование T в или из U соответственно.
    */
    template <class Iterator>
    class reverse_iterator
     : saga::rel_ops::enable_adl<reverse_iterator<Iterator>>
    {
        template <class Iterator2>
        friend
        constexpr bool operator==(reverse_iterator<Iterator> const & lhs,
                                  reverse_iterator<Iterator2> const & rhs)
        {
            return lhs.base() == rhs.base();
        }

        template <class Iterator2>
        friend
        constexpr bool operator<(reverse_iterator<Iterator> const & lhs,
                                 reverse_iterator<Iterator2> const & rhs)
        {
            return rhs.base() < lhs.base();
        }

        template <class Iterator2>
        friend
        constexpr bool operator==(reverse_iterator<Iterator> const & lhs,
                                  std::reverse_iterator<Iterator2> const & rhs)
        {
            return lhs.base() == rhs.base();
        }

        template <class Iterator2>
        friend
        constexpr bool operator==(std::reverse_iterator<Iterator2> const & lhs,
                                  reverse_iterator<Iterator> const & rhs)
        {
            return lhs.base() == rhs.base();
        }

        template <class Iterator2>
        friend
        constexpr bool operator<(reverse_iterator<Iterator> const & lhs,
                                 std::reverse_iterator<Iterator2> const & rhs)
        {
            return rhs.base() < lhs.base();
        }

        template <class Iterator2>
        friend
        constexpr bool operator<(std::reverse_iterator<Iterator2> const & lhs,
                                 reverse_iterator<Iterator> const & rhs)
        {
            return rhs.base() < lhs.base();
        }

    public:
        // Типы
        using iterator_type = Iterator;
        using iterator_category = saga::iterator_category_t<Iterator>;
        using value_type = typename std::iterator_traits<Iterator>::value_type;
        using difference_type = typename std::iterator_traits<Iterator>::difference_type;
        using pointer = typename std::iterator_traits<Iterator>::pointer;
        using reference = typename std::iterator_traits<Iterator>::reference;

        // Создание и копирование
        constexpr reverse_iterator() = default;

        constexpr explicit reverse_iterator(Iterator iter)
         : current_(std::move(iter))
        {}

        /**
        @brief В отличие от STL, этот конструктор не участвует в разрешении перегрузки, если
        @c Iterator нельзя сконструировать из @c OtherIterator
        */
        template <class OtherIterator,
                  typename = std::enable_if_t<std::is_constructible<Iterator, OtherIterator const &>{}>>
        constexpr reverse_iterator(reverse_iterator<OtherIterator> const & other)
         : current_(other.base())
        {}

        template <class OtherIterator,
                  typename = std::enable_if_t<std::is_constructible<Iterator, OtherIterator const &>{}>>
        constexpr reverse_iterator(std::reverse_iterator<OtherIterator> const & other)
         : current_(other.base())
        {}

        /**
        @brief В отличие от STL, этот конструктор не участвует в разрешении перегрузки, если
        @c Iterator нельзя сконструировать из @c OtherIterator
        */
        template <class OtherIterator,
                  typename = std::enable_if_t<std::is_constructible<Iterator, OtherIterator const &>{}
                                              && std::is_assignable<Iterator &, OtherIterator const &>{}>>
        constexpr reverse_iterator & operator=(reverse_iterator<OtherIterator> const & other)
        {
            this->current_ = other.base();
            return *this;
        }

        template <class OtherIterator,
                  typename = std::enable_if_t<std::is_constructible<Iterator, OtherIterator const &>{}
                                              && std::is_assignable<Iterator &, OtherIterator const &>{}>>
        constexpr reverse_iterator & operator=(std::reverse_iterator<OtherIterator> const & other)
        {
            this->current_ = other.base();
            return *this;
        }

        // Преобразование
        template <class OtherIterator,
                  typename = std::enable_if_t<std::is_constructible<OtherIterator, Iterator const &>{}>>
        constexpr operator std::reverse_iterator<OtherIterator>() const
        {
            return std::reverse_iterator<OtherIterator>(this->base());
        }

        constexpr Iterator base() const
        {
            return this->current_;
        }

        // Итератор
        constexpr reference operator*() const
        {
            auto tmp = this->current_;
            -- tmp;
            return *tmp;
        }

        constexpr pointer operator->() const
        {
            return std::addressof(**this);
        }

        constexpr reverse_iterator & operator++()
        {
            -- this->current_;
            return *this;
        }

        constexpr reverse_iterator operator++(int)
        {
            auto old_state = *this;
            ++ *this;

            return old_state;
        }

        // Двунаправленный итератор
        constexpr reverse_iterator & operator--()
        {
            ++ this->current_;
            return *this;
        }

        constexpr reverse_iterator operator--(int)
        {
            auto old_state = *this;
            -- *this;
            return old_state;
        }

        // Итератор произвольного доступа
        constexpr reverse_iterator operator+(difference_type num) const
        {
            return reverse_iterator(this->base() - num);
        }

        constexpr reverse_iterator & operator+=(difference_type num)
        {
            this->current_ -= num;
            return *this;
        }

        constexpr reverse_iterator operator-(difference_type num) const
        {
            return reverse_iterator(this->base() + num);
        }

        constexpr reverse_iterator & operator-=(difference_type num)
        {
            this->current_ += num;
            return *this;
        }

        constexpr decltype(auto) operator[](difference_type num) const
        {
            return this->base()[-num-1];
        }

        template <class Iterator2>
        friend
        constexpr decltype(std::declval<Iterator2>() - std::declval<Iterator>())
        operator-(reverse_iterator<Iterator> const & lhs, reverse_iterator<Iterator2> const & rhs)
        {
            return rhs.base() - lhs.base();
        }

        template <class Iterator2>
        friend
        constexpr decltype(std::declval<Iterator2>() - std::declval<Iterator>())
        operator-(reverse_iterator<Iterator> const & lhs,
                  std::reverse_iterator<Iterator2> const & rhs)
        {
            return rhs.base() - lhs.base();
        }

        template <class Iterator2>
        friend
        constexpr decltype(std::declval<Iterator>() - std::declval<Iterator2>())
        operator-(std::reverse_iterator<Iterator2> const & lhs,
                  reverse_iterator<Iterator> const & rhs)
        {
            return rhs.base() - lhs.base();
        }

        friend
        constexpr reverse_iterator
        operator+(typename reverse_iterator<Iterator>::difference_type num,
                  reverse_iterator const & rhs)
        {
            return reverse_iterator(rhs.base() - num);
        }

    private:
        Iterator current_ {};
    };

    template <class Iterator>
    constexpr reverse_iterator<Iterator>
    make_reverse_iterator(Iterator iter)
    {
        return reverse_iterator<Iterator>(std::move(iter));
    }

    template <class Iterator>
    constexpr Iterator
    make_reverse_iterator(reverse_iterator<Iterator> const & iter)
    {
        return iter.base();
    }

    template <class Iterator>
    constexpr Iterator
    make_reverse_iterator(std::reverse_iterator<Iterator> const & iter)
    {
        return iter.base();
    }
}
// namespace saga

#endif
// Z_SAGA_ITERATOR_REVERSE_HPP_INCLUDED
