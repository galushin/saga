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

#ifndef Z_SAGA_CURSOR_TAKE_WHILE_HPP_INCLUDED
#define Z_SAGA_CURSOR_TAKE_WHILE_HPP_INCLUDED

/** @brief saga/cursor/take_while.hpp
 @brief Адаптер курсора, останавливающийся, когда элементы исходной последовательности
 перестают удовлевторять заданному предикату
*/

namespace saga
{
    template <class Cursor, class UnaryPredicate>
    class take_while_cursor
     : saga::cursor_facade<take_while_cursor<Cursor, UnaryPredicate>
                           , cursor_reference_t<Cursor>>
    {
    public:
        // Типы
        using reference = cursor_reference_t<Cursor>;

        // Создание, копирование, уничтожение
        explicit take_while_cursor(Cursor cur, UnaryPredicate pred)
         : base_(std::move(cur))
         , pred_(std::move(pred))
        {}

        // Курсор ввода
        bool operator!() const
        {
            return !this->base() || !this->pred_(this->base().front());
        }

        reference front() const
        {
            return this->base_.front();
        }

        void drop_front()
        {
            this->base_.drop_front();
        }

        // Адаптер курсора
        Cursor const & base() const
        {
            return this->base_;
        }

    private:
        Cursor base_;
        UnaryPredicate pred_;
    };

    namespace cursor
    {
        template <class Cursor, class UnaryPredicate>
        take_while_cursor<Cursor, UnaryPredicate>
        take_while(Cursor cur, UnaryPredicate pred)
        {
            return take_while_cursor<Cursor, UnaryPredicate>(std::move(cur), std::move(pred));
        }
    }
    // namespace cursor
}
// namespace saga

#endif
// Z_SAGA_CURSOR_TAKE_WHILE_HPP_INCLUDED
