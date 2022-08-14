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

#ifndef Z_SAGA_CURSOR_STRIDE_HPP_INCLUDED
#define Z_SAGA_CURSOR_STRIDE_HPP_INCLUDED

/** @file saga/cursor/stride.hpp
 @brief Адаптер курсора, продвижение которого на один шаг соответствует продвижению на заданное
 число шагов базового курсора
*/

#include <saga/algorithm.hpp>
#include <saga/cursor/cursor_traits.hpp>
#include <saga/utility/pipeable.hpp>

namespace saga
{
    template <class InputCursor>
    class stride_cursor
    {
    public:
        // Типы
        using difference_type = saga::cursor_difference_t<InputCursor>;
        using reference = saga::cursor_reference_t<InputCursor>;
        using value_type = saga::cursor_value_t<InputCursor>;

        // Создание, копирование, уничтожение
        constexpr explicit stride_cursor(InputCursor cur, difference_type step)
         : cur_(std::move(cur))
         , step_(std::move(step))
        {}

        // Курсор ввода
        constexpr bool operator!() const
        {
            return !this->cur_;
        }

        constexpr reference front() const
        {
            return this->cur_.front();
        }

        constexpr void drop_front()
        {
            this->cur_ = detail::drop_front_n_guarded(std::move(this->cur_), this->step_);
        }

    private:
        InputCursor cur_;
        difference_type step_ = 0;
    };

    namespace cursor
    {
        template <class InputCursor>
        constexpr stride_cursor<InputCursor>
        stride(InputCursor cur, saga::cursor_difference_t<InputCursor> num)
        {
            return stride_cursor<InputCursor>(std::move(cur), std::move(num));
        }

        template <class Size>
        constexpr auto stride(Size num)
        {
            return saga::make_pipeable([arg = std::move(num)](auto cur)
                                       { return cursor::stride(std::move(cur), arg);});
        }
    }
    // namespace cursor
}
// namespace saga

#endif
// Z_SAGA_CURSOR_STRIDE_HPP_INCLUDED
