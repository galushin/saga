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

#ifndef Z_SAGA_CURSOR_SET_UNION_HPP_INCLUDED
#define Z_SAGA_CURSOR_SET_UNION_HPP_INCLUDED

/** @file saga/cursor/set_union.hpp
 @brief Курсор, представляющий теоретико-множественное объединение элементов двух базовых курсоров.
*/

#include <saga/cursor/cursor_facade.hpp>
#include <saga/cursor/cursor_traits.hpp>

namespace saga
{
    template <class InputCursor1, class InputCursor2>
    class set_union_cursor
     : saga::cursor_facade<set_union_cursor<InputCursor1, InputCursor2>
                          , std::common_type_t<saga::cursor_reference_t<InputCursor1>
                                              , saga::cursor_reference_t<InputCursor2>>>
    {
    public:
        // Типы
        using reference = std::common_type_t<saga::cursor_reference_t<InputCursor1>
                                            , saga::cursor_reference_t<InputCursor2>>;
        using value_type = std::common_type_t<saga::cursor_value_t<InputCursor1>
                                             , saga::cursor_value_t<InputCursor2>>;

        // Создание, копирование, уничтожение
        constexpr explicit set_union_cursor(InputCursor1 in1, InputCursor2 in2)
         : in1_(std::move(in1))
         , in2_(std::move(in2))
        {
            this->adjust_state();
        }

        // Курсор ввода
        constexpr bool operator!() const
        {
            return this->state_ == State::empty;
        }

        constexpr reference front() const
        {
            assert(this->state_ != State::empty);

            if(this->state_ == State::second || this->state_ == State::always_second)
            {
                return this->in2_.front();
            }
            else
            {
                return this->in1_.front();
            }
        }

        constexpr void drop_front()
        {
            assert(this->state_ != State::empty);

            if(this->state_ == State::first || this->state_ == State::equiv
               || this->state_ == State::always_first)
            {
                this->in1_.drop_front();
            }

            if(this->state_ == State::second || this->state_ == State::equiv
               || this->state_ == State::always_second)
            {
                this->in2_.drop_front();
            }

            this->adjust_state();
        }

    private:
        enum class State
        {
            empty,
            first,
            second,
            equiv,
            always_first,
            always_second
        };

        constexpr void adjust_state()
        {
            if(this->state_ == State::always_first || this->state_ == State::always_second)
            {
                return;
            }

            if(!in1_ && !in2_)
            {
                this->state_ = State::empty;
            }
            else if(!in1_)
            {
                this->state_ = State::always_second;
            }
            else if(!in2_)
            {
                this->state_ = State::always_first;
            }
            else if(in2_.front() < in1_.front())
            {
                this->state_ = State::second;
            }
            else if(in1_.front() < in2_.front())
            {
                this->state_ = State::first;
            }
            else
            {
                this->state_ = State::equiv;
            }
        }

        InputCursor1 in1_;
        InputCursor2 in2_;
        State state_ = State::empty;
    };

    namespace cursor
    {
        template <class InputCursor1, class InputCursor2>
        constexpr set_union_cursor<InputCursor1, InputCursor2>
        set_union(InputCursor1 in1, InputCursor2 in2)
        {
            return set_union_cursor<InputCursor1, InputCursor2>(std::move(in1), std::move(in2));
        }
    }
    // namespace cursor
}
// namespace saga

#endif
// Z_SAGA_CURSOR_SET_UNION_HPP_INCLUDED
