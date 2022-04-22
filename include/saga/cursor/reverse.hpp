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

/** @file saga/cursor/reverse.hpp
 @brief Адаптер курсора, проходящий элементы базового курсора в обратном порядке
*/

#ifndef Z_SAGA_CURSOR_REVERSE_HPP_INCLUDED
#define Z_SAGA_CURSOR_REVERSE_HPP_INCLUDED

#include <saga/cursor/cursor_facade.hpp>
#include <saga/cursor/cursor_traits.hpp>

namespace saga
{
    template <class BidirectionalCursor>
    class reverse_cursor
     : cursor_facade<reverse_cursor<BidirectionalCursor>
                    , saga::cursor_reference_t<BidirectionalCursor>>
    {
    public:
        // Типы
        // @todo using cursor_category = ;
        // @todo using value_type = ;
        using reference = saga::cursor_reference_t<BidirectionalCursor>;
        using difference_type = saga::cursor_difference_t<BidirectionalCursor>;

        // Создание, копирование, уничтожение
        constexpr explicit reverse_cursor(BidirectionalCursor cur)
         : base_(std::move(cur))
        {}

        // Курсор ввода
        constexpr bool operator!() const
        {
            return !this->base();
        }

        constexpr reference front() const
        {
            return this->base().back();
        }

        constexpr void drop_front()
        {
            this->base_.drop_back();
        }

        // Прямой курсор
        reverse_cursor dropped_front() const
        {
            return reverse_cursor(this->base().dropped_back());
        }

        void exhaust_front()
        {
            this->base_.exhaust_back();
        }

        void exhaust_back()
        {
            this->base_.exhaust_front();
        }

        void forget_front()
        {
            return this->base_.forget_back();
        }

        // @todo splice

        void rewind_front()
        {
            return this->base_.rewind_back();
        }

        // Двунаправленный курсор
        void drop_back()
        {
            this->base_.drop_front();
        }

        reference back() const
        {
            return this->base().front();
        }

        reverse_cursor dropped_back() const
        {
            return reverse_cursor(this->base().dropped_front());
        }

        void rewind_back()
        {
            return this->base_.rewind_front();
        }

        void forget_back()
        {
            return this->base_.forget_front();
        }

        // Курсор произвольного доступа
        difference_type size() const
        {
            return this->base().size();
        }

        void drop_front(difference_type num)
        {
            this->base_.drop_back(std::move(num));
        }

        void drop_back(difference_type num)
        {
            this->base_.drop_front(std::move(num));
        }

        reference operator[](difference_type index) const
        {
            assert(0 <= index && index < this->size());

            return this->base()[this->size() - index - 1];
        }

        // Адаптер курсора
        constexpr BidirectionalCursor const & base() const &
        {
            return this->base_;
        }

        constexpr BidirectionalCursor base() &&
        {
            return std::move(this->base_);
        }

    private:
        BidirectionalCursor base_;
    };

    namespace cursor
    {
        template <class BidirectionalCursor>
        constexpr reverse_cursor<BidirectionalCursor>
        reverse(BidirectionalCursor cur)
        {
            return reverse_cursor<BidirectionalCursor>(std::move(cur));
        }

        template <class BidirectionalCursor>
        constexpr BidirectionalCursor
        reverse(reverse_cursor<BidirectionalCursor> const & cur)
        {
            return cur.base();
        }
    }
    // namespace cursor
}
// namespace saga

#endif
// Z_SAGA_CURSOR_REVERSE_HPP_INCLUDED
