#ifndef Z_SAGA_CURSOR_TAKE_HPP_INCLUDED
#define Z_SAGA_CURSOR_TAKE_HPP_INCLUDED

/** @brief saga/cursor/take.hpp
 @brief Адаптер курсора, ограниченный заданным количеством элементов базового курсора
*/

#include <saga/cursor/cursor_traits.hpp>
#include <saga/cursor/forward_cursor_facade.hpp>

#include <cassert>

namespace saga
{
    template <class Cursor, class Difference>
    class take_cursor
     : saga::forward_cursor_facade<take_cursor<Cursor, Difference>
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
        explicit take_cursor(Cursor cur, Difference count)
         : base_(std::move(cur))
         , count_(std::move(count))
        {}

        // Курсор ввода
        bool operator!() const
        {
            return !this->base_ || !(this->count_ > 0);
        }

        reference front() const
        {
            assert(!!*this);

            return this->base_.front();
        }

        void drop_front()
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
