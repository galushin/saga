/* (c) 2021 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_NUMERIC_DIGITS_OF_HPP_INCLUDED
#define Z_SAGA_NUMERIC_DIGITS_OF_HPP_INCLUDED

/** @file saga/numeric/digits_of.hpp
 @brief Курсор последовательности цифр неотрицательного целого числа
*/

#include <saga/cursor/cursor_facade.hpp>

#include <cassert>
#include <iterator>
#include <utility>

namespace saga
{
    template <class IntType>
    class digits_cursor
     : saga::cursor_facade<digits_cursor<IntType>, IntType const &>
    {
    public:
        // Типы
        using cursor_category = std::input_iterator_tag;
        using value_type = IntType;
        using reference = IntType const &;

        // Создание, копирование, уничтожение
        /**
        @pre <tt>base >= 2</tt>
        @pre <tt>num >= 0</tt>
        */
        constexpr explicit digits_cursor(IntType num, IntType base = IntType(10))
         : num_(std::move(num))
         , base_(std::move(base))
         , cur_(this->num_ % this->base_)
        {
            assert(this->base_ >= 2);
        }

        // Курсор ввода
        constexpr bool operator!() const
        {
            return !(this->num_ > 0);
        }

        constexpr reference front() const
        {
            return this->cur_;
        }

        constexpr void drop_front()
        {
            this->num_ /= this->base_;
            this->cur_ = this->num_ % this->base_;
        }

    private:
        IntType num_ = 0;
        IntType base_ = 2;
        IntType cur_ = 0;
    };

    namespace cursor
    {
        template <class IntType>
        constexpr auto digits_of(IntType num, IntType base) -> saga::digits_cursor<IntType>
        {
            return digits_cursor<IntType>(num, base);
        }

        template <class IntType>
        constexpr auto digits_of(IntType num) -> saga::digits_cursor<IntType>
        {
            return digits_of(num, IntType(10));
        }
    }
    // namespace cursor
}
// namespace saga

#endif
// Z_SAGA_NUMERIC_DIGITS_OF_HPP_INCLUDED
