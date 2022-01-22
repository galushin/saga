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

#ifndef Z_SAGA_CURSOR_FILTER_HPP_INCLUDED
#define Z_SAGA_CURSOR_FILTER_HPP_INCLUDED

#include <saga/algorithm/find_if.hpp>
#include <saga/cursor/forward_cursor_facade.hpp>

namespace saga
{
    template <class InputCursor, class UnaryPredicate>
    class filter_cursor
     : forward_cursor_facade<filter_cursor<InputCursor, UnaryPredicate>
                            , cursor_reference_t<InputCursor>>
    {
    public:
        // Типы
        using reference = cursor_reference_t<InputCursor>;

        // Создание, копирование, уничтожение
        explicit filter_cursor(InputCursor cur, UnaryPredicate pred)
         : data_(std::move(cur), std::move(pred))
        {
            this->base_ref() = saga::find_if(this->base(), this->predicate());
        }

        // Курсор ввода
        bool operator!() const
        {
            return !this->base();
        }

        void drop_front()
        {
            this->base_ref().drop_front();

            this->base_ref() = saga::find_if(this->base(), this->predicate());
        }

        reference front() const
        {
            return this->base().front();
        }

        // Прямой курсор

        // Адаптер курсора
        InputCursor const & base() const
        {
            return std::get<0>(this->data_);
        }

        UnaryPredicate const & predicate() const
        {
            return std::get<1>(this->data_);
        }

    private:
        InputCursor & base_ref()
        {
            return std::get<0>(this->data_);
        }

        std::tuple<InputCursor, UnaryPredicate> data_;
    };

    namespace cursor
    {
        template <class Cursor, class UnaryPredicate>
        filter_cursor<Cursor, UnaryPredicate>
        filter(Cursor cur, UnaryPredicate pred)
        {
            return filter_cursor<Cursor, UnaryPredicate>(std::move(cur), std::move(pred));
        }
    }
    // namespace cursor
}
// namespace saga

#endif // Z_SAGA_CURSOR_FILTER_HPP_INCLUDED
