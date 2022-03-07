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

#ifndef Z_SAGA_CURSOR_SLIDE_HPP_INCLUDED
#define Z_SAGA_CURSOR_SLIDE_HPP_INCLUDED

/** @file saga/cursor/slide.hpp
 @brief Курсор курсоров такой, что M-ый курсор содержит элементы с M-ого по (M+N-1)-ый исходного
 курсора.
*/

// @todo Перенести в заголовочный файл
#include <saga/cursor/take.hpp>

namespace saga
{
    namespace cursor
    {
        // @todo Оптимизация размера и алгоритмов
        template <class Cursor>
        class slide_cursor
         : saga::cursor_facade<slide_cursor<Cursor>
                              , saga::take_cursor<Cursor, saga::cursor_difference_t<Cursor>>>
        {
        public:
            // Типы
            // @todo Проверить, что Cursor как минимум прямой
            // @todo Точно такой тип курсора? Так используется двойной учёт исчерпания
            using reference = saga::take_cursor<Cursor, saga::cursor_difference_t<Cursor>>;
            using difference_type = saga::cursor_difference_t<Cursor>;
            using cursor_category = saga::cursor_category_t<Cursor>;

            // Создание, копирование, уничтожение
            constexpr explicit slide_cursor(Cursor cur, difference_type num)
             : cur_(std::move(cur))
             , probe_(saga::cursor::drop_front_n(cur_, num - 1))
             , num_(std::move(num))
            {
                assert(this->num_ > 0);
            }

            // Курсор ввода
            constexpr bool operator!() const
            {
                return !this->probe_;
            }

            constexpr reference front() const
            {
                return reference(this->cur_, this->num_);
            }

            constexpr void drop_front()
            {
                cur_.drop_front();
                probe_.drop_front();
            }

        private:
            Cursor cur_;
            Cursor probe_;
            difference_type num_ = 0;
        };

        template <class Cursor>
        constexpr slide_cursor<Cursor>
        slide(Cursor cur, saga::cursor_difference_t<Cursor> num)
        {
            return slide_cursor<Cursor>(std::move(cur), std::move(num));
        }

    }
    // namespace cursor
}
// namespace saga

#endif
// Z_SAGA_CURSOR_SLIDE_HPP_INCLUDED
