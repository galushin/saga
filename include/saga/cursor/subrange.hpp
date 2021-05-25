/* (c) 2021 Галушин Павел Викторович, galushin@gmail.com

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
#include <saga/iterator.hpp>
#include <saga/defs.hpp>

#include <cassert>
#include <iterator>

namespace saga
{
    template <class ForwardIterator, class Sentinel = ForwardIterator>
    class subrange_cursor
    {
    public:
        // Типы
        using reference = typename std::iterator_traits<ForwardIterator>::reference;
        using difference_type = typename std::iterator_traits<ForwardIterator>::difference_type;
        using iterator = ForwardIterator;

        // Создание, копирование, уничтожение
        constexpr subrange_cursor(ForwardIterator first, Sentinel last, unsafe_tag_t)
         : cur_(std::move(first))
         , last_(std::move(last))
         , back_(this->last_)
        {
            this->tweak_back(typename std::iterator_traits<Sentinel>::iterator_category{});
        }

        // Итераторы
        iterator begin() const
        {
            return this->cur_;
        }

        Sentinel end() const
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

        constexpr subrange_cursor & operator++()
        {
            this->drop_front();

            return *this;
        }

        constexpr reference front() const
        {
            assert(!!*this);

            return *this->cur_;
        }

        constexpr reference operator*() const
        {
            return this->front();
        }

        // Курсор вывода
        template <class Arg>
        constexpr subrange_cursor & operator<<(Arg && arg)
        {
            **this = std::forward<Arg>(arg);
            ++*this;
            return *this;
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

            return *this->back_;
        }

        // Курсор произвольного доступа
        difference_type size() const
        {
            return this->end() - this->begin();
        }

    private:
        void tweak_back(std::input_iterator_tag)
        {}

        constexpr void tweak_back(std::bidirectional_iterator_tag)
        {
            if(this->back_ != this->cur_)
            {
                -- back_;
            }
        }

    private:
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
                 ,std::enable_if_t<std::is_reference<ForwardRange>::value> * = nullptr>
        constexpr auto all(ForwardRange && range)
        {
            return ::saga::make_subrange_cursor(saga::begin(range), saga::end(range)
                                               ,unsafe_tag_t{});
        }
    }
    // namespace cursor
}
// namespace saga

#endif
// Z_SAGA_CURSOR_SUBRANGE_HPP_INCLUDED
