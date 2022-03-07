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

#ifndef Z_SAGA_CURSOR_TAKE_HPP_INCLUDED
#define Z_SAGA_CURSOR_TAKE_HPP_INCLUDED

/** @brief saga/cursor/take.hpp
 @brief Адаптер курсора, ограниченный заданным количеством элементов базового курсора
*/

#include <saga/cursor/cursor_traits.hpp>
#include <saga/cursor/cursor_facade.hpp>

#include <cassert>
#include <algorithm>

namespace saga
{
    template <class Cursor, class Difference>
    class take_cursor
     : saga::cursor_facade<take_cursor<Cursor, Difference>
                          , saga::cursor_reference_t<Cursor>>
    {
        friend
        bool operator==(take_cursor const & lhs, take_cursor const & rhs)
        {
            return lhs.base() == rhs.base() && lhs.count_ == rhs.count_;
        }

    public:
        // Типы
        using reference = saga::cursor_reference_t<Cursor>;
        using difference_type = Difference;
        using cursor_category = saga::cursor_category_t<Cursor>;

        // Создание, копирование, уничтожение
        constexpr explicit take_cursor(Cursor cur, Difference count)
         : base_(std::move(cur))
         , count_(std::move(count))
        {}

        // Курсор ввода
        constexpr bool operator!() const
        {
            return !this->base_ || !(this->count_ > 0);
        }

        constexpr reference front() const
        {
            assert(!!*this);

            return this->base_.front();
        }

        constexpr void drop_front()
        {
            assert(!!*this);

            this->base_.drop_front();
            -- this->count_;
        }

        // Курсор вывода
        template <class Arg, class = decltype(std::declval<Cursor>() << std::declval<Arg>())>
        take_cursor & operator<<(Arg && arg)
        {
            this->base_ << std::forward<Arg>(arg);
            -- this->count_;

            return *this;
        }

        // Курсор произвольного доступа
        difference_type size() const
        {
            return std::min(difference_type(this->base().size()), difference_type(this->count_));
        }

        void drop_front(difference_type num)
        {
            assert(0 <= num && num <= this->size());

            this->base_.drop_front(num);
            count_ -= num;
        }

        // Адаптер курсора
        Cursor const & base() const
        {
            return this->base_;
        }

    private:
        Cursor base_;
        Difference count_;
    };

    namespace cursor
    {
        // @todo Отпимизация для отдельных типов курсоров
        template <class Cursor, class Difference>
        take_cursor<Cursor, Difference>
        take(Cursor cur, Difference count)
        {
            return take_cursor<Cursor, Difference>(std::move(cur), std::move(count));
        }
    }
    //namespace cursor
}
// namespace saga

#endif
// Z_SAGA_CURSOR_TAKE_HPP_INCLUDED
