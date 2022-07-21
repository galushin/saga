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

#ifndef Z_SAGA_CURSOR_CARTESIAN_PRODUCT_HPP_INCLUDED
#define Z_SAGA_CURSOR_CARTESIAN_PRODUCT_HPP_INCLUDED

/** @file saga/cursor/cartesian_product.hpp
 @brief Курсор, последовательность элементов которого представляет собой декартово произведение
 последовательностей элементов курсоров-аргументов
*/

#include <saga/cursor/cursor_facade.hpp>
#include <saga/cursor/cycle.hpp>

namespace saga
{
    template <class InputCursor, class ForwardCursor, class Difference = std::ptrdiff_t>
    class cartesian_product_cursor
     : saga::cursor_facade<cartesian_product_cursor<InputCursor, ForwardCursor>
                          , std::pair<cursor_reference_t<InputCursor>
                                     , cursor_reference_t<ForwardCursor>>>
    {
    public:
        // Типы
        using reference = std::pair<cursor_reference_t<InputCursor>
                                   , cursor_reference_t<ForwardCursor>>;

        using difference_type = Difference;
        using cursor_category = saga::cursor_category_t<InputCursor>;

        // Создание
        explicit cartesian_product_cursor(InputCursor cur1, ForwardCursor cur2)
         : cur1_(std::move(cur1))
         , cur2_(std::move(cur2))
        {
            cur2_.forget_front();
        }

        // Курсор ввода
        bool operator!() const
        {
            return !this->cur1_ || !this->cur2_;
        }

        reference front() const
        {
            return reference(this->cur1_.front(), this->cur2_.front());
        }

        void drop_front()
        {
            assert(!!*this);

            ++ this->cur2_;

            if(!this->cur2_)
            {
                ++ this->cur1_;
                this->cur2_ = this->cur2_.dropped_front();
            }
        }

    private:
        InputCursor cur1_;
        ForwardCursor cur2_;
    };

    namespace cursor
    {
        template <class InputCursor, class... ForwardCursors>
        cartesian_product_cursor<InputCursor, ForwardCursors...>
        cartesian_product(InputCursor cur1, ForwardCursors... curs)
        {
            using Result = cartesian_product_cursor<InputCursor, ForwardCursors...>;
            return Result(std::move(cur1), std::move(curs)...);
        }

    }
    // namespace cursor
}
// namespace saga

#endif
// Z_SAGA_CURSOR_CARTESIAN_PRODUCT_HPP_INCLUDED
