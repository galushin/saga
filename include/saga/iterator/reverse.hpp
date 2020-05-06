/* (c) 2020 Галушин Павел Викторович, galushin@gmail.com

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
 @todo Совместимость с std::reverse_iterator
*/

#include <iterator>

namespace saga
{
    template <class Iterator>
    class reverse_iterator
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
        constexpr bool operator!=(reverse_iterator<Iterator> const & lhs,
                                  reverse_iterator<Iterator2> const & rhs)
        {
            // Стандарт описывает как lhs.current != rhs.current, но для итераторов должно быть
            // (lhs.current != rhs.current) == !(lhs.current == rhs.current)
            return !(lhs == rhs);
        }

    public:
        // Типы
        using iterator_type = Iterator;
        using iterator_category = typename std::iterator_traits<Iterator>::iterator_category;
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

        // Преобразование
        constexpr Iterator base() const
        {
            return this->current_;
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
}
// namespace saga

#endif
// Z_SAGA_ITERATOR_REVERSE_HPP_INCLUDED
