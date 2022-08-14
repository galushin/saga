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

#ifndef Z_SAGA_CURSOR_TRANSFORM_HPP_INCLUDED
#define Z_SAGA_CURSOR_TRANSFORM_HPP_INCLUDED

#include <saga/cursor/cursor_facade.hpp>
#include <saga/cursor/cursor_traits.hpp>
#include <saga/functional.hpp>
#include <saga/utlity/pipeable.hpp>

namespace saga
{
    template <class Cursor, class UnaryFunction>
    class transform_cursor
     : saga::cursor_facade<transform_cursor<Cursor, UnaryFunction>
                           , saga::invoke_result_t<UnaryFunction const &
                                                  , cursor_reference_t<Cursor>>>
    {
    public:
        // Типы
        using reference = saga::invoke_result_t<UnaryFunction const &, cursor_reference_t<Cursor>>;
        using difference_type = saga::cursor_difference_t<Cursor>;
        using cursor_category = saga::cursor_category_t<Cursor>;
        using value_type = std::decay_t<reference>;

        // Создание, копирование, уничтожение
        constexpr explicit transform_cursor(Cursor cur, UnaryFunction fun)
         : cur_(std::move(cur))
         , fun_(std::move(fun))
        {}

        // Курсор ввода
        constexpr bool operator!() const
        {
            return !this->base();
        }

        constexpr reference front() const
        {
            return saga::invoke(this->fun_, this->cur_.front());
        }

        constexpr void drop_front()
        {
            this->cur_.drop_front();
        }

        // Курсор произвольного доступа
        constexpr difference_type size() const
        {
            return this->cur_.size();
        }

        constexpr void drop_front(difference_type num)
        {
            assert(0 <= num && num <= this->size());

            this->cur_.drop_front(num);
        }

        // Адаптер курсора
        constexpr Cursor const & base() const
        {
            return this->cur_;
        }

    private:
        Cursor cur_;
        UnaryFunction fun_;
    };

    namespace cursor
    {
        template <class Cursor, class UnaryFunction>
        constexpr transform_cursor<Cursor, UnaryFunction>
        transform(Cursor cur, UnaryFunction fun)
        {
            return transform_cursor<Cursor, UnaryFunction>(std::move(cur), std::move(fun));
        }

        template <class UnaryFunction>
        constexpr auto transform(UnaryFunction fun)
        {
            return saga::make_pipeable([arg = std::move(fun)](auto cur)
                                       {return cursor::transform(std::move(cur), arg);});
        }
    }
    // namespace cursor
}
// namespace saga

#endif
// Z_SAGA_CURSOR_TRANSFORM_HPP_INCLUDED
