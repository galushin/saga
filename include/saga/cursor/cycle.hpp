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

#ifndef Z_SAGA_CURSOR_CYCLE_HPP_INCLUDED
#define Z_SAGA_CURSOR_CYCLE_HPP_INCLUDED

/** @file saga/cursor/cycle.hpp
 @brief Адаптор курсора, циклически проходящий базовый курсор
*/

#include <saga/cursor/cursor_traits.hpp>

#include <cassert>

#include <utility>

namespace saga
{
    template <class ForwardCursor>
    class cycled_cursor
    {
    public:
        // Типы
        using reference = saga::cursor_reference<ForwardCursor>;

        // Создание, копирование, уничтожение
        explicit cycled_cursor(ForwardCursor cur)
         : cur_(cur)
         , orig_(std::move(cur))
        {}

        // Курсор ввода
        bool operator!() const
        {
            return !this->orig_;
        }

        cycled_cursor & operator++()
        {
            assert(!!*this);

            ++ this->cur_;

            if(!this->cur_)
            {
                this->cur_ = this->orig_;
            }

            return *this;
        }

        reference operator*() const
        {
            assert(!!*this);

            return *this->cur_;
        }

    private:
        // @todo Можно ли избежать дублирования?
        ForwardCursor cur_;
        ForwardCursor orig_;
    };

    namespace cursor
    {
        template <class ForwardCursor>
        auto make_cycled(ForwardCursor cur)
        {
            return ::saga::cycled_cursor<ForwardCursor>(std::move(cur));
        }
    }
    // namespace cursor
}
// namespace saga



#endif
// Z_SAGA_CURSOR_CYCLE_HPP_INCLUDED
