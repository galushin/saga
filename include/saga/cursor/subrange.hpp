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

#ifndef Z_SAGA_CURSOR_SUBRANGE_HPP_INCLUDED
#define Z_SAGA_CURSOR_SUBRANGE_HPP_INCLUDED

/** @file saga/cursor/subrange.hpp
 @brief Курсор, проходящий интервал, заданный итератором и часовым
*/

#include <saga/iterator.hpp>
#include <saga/cursor/cursor_traits.hpp>

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

        // Создание, копирование, уничтожение
        subrange_cursor(ForwardIterator first, Sentinel last)
         : cur_(std::move(first))
         , last_(std::move(last))
         , back_(this->last_)
        {
            if(this->back_ != this->cur_)
            {
                -- back_;
            }
        }

        // Курсор ввода
        bool operator!() const
        {
            return this->cur_ == this->last_;
        }

        void drop(saga::front_fn)
        {
            assert(!!*this);

            ++ this->cur_;
        }

        subrange_cursor & operator++()
        {
            this->drop(saga::front_fn{});

            return *this;
        }

        reference operator[](saga::front_fn) const
        {
            assert(!!*this);

            return *this->cur_;
        }

        reference operator*() const
        {
            return (*this)[saga::front_fn{}];
        }

        // Двунаправленный курсор
        void drop(saga::back_fn)
        {
            assert(!!*this);

            this->last_ = this->back_;

            if(!!*this)
            {
                --this->back_;
            }
        }

        reference operator[](saga::back_fn)
        {
            assert(!!*this);

            return *this->back_;
        }

    private:
        ForwardIterator cur_;
        Sentinel last_;
        Sentinel back_;
    };

    template <class ForwardIterator>
    auto make_subrange_cursor(ForwardIterator first, ForwardIterator last)
    -> subrange_cursor<ForwardIterator>
    {
        return subrange_cursor<ForwardIterator>(std::move(first), std::move(last));
    }

    namespace cursor
    {
        template <class ForwardRange, std::enable_if_t<std::is_reference<ForwardRange>::value, bool> = true>
        auto all(ForwardRange && range)
        {
            return ::saga::make_subrange_cursor(saga::begin(range), saga::end(range));
        }
    }
    // namespace cursor
}
// namespace saga

#endif
// Z_SAGA_CURSOR_SUBRANGE_HPP_INCLUDED
