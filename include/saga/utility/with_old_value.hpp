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

#ifndef Z_SAGA_UTILITY_WITH_OLD_VALUE_HPP_INCLUDED
#define Z_SAGA_UTILITY_WITH_OLD_VALUE_HPP_INCLUDED

/** @file saga/utility/with_old_value.hpp
 @brief Обёртка для значения, которая также хранит "старое" значение и позволяет к нему вернуться.
*/

#include <saga/utility/operators.hpp>

#include <utility>
#include <tuple>

namespace saga
{
    template <class T>
    class with_old_value
    : saga::rel_ops::enable_adl<with_old_value<T>>
    {
        constexpr friend
        bool operator==(with_old_value const & lhs, with_old_value const & rhs)
        {
            return std::tie(lhs.value(), lhs.old_value())
                    == std::tie(rhs.value(), rhs.old_value());
        }

        constexpr friend
        bool operator==(with_old_value const & lhs, T const & rhs)
        {
            return lhs.value() == rhs;
        }

        constexpr friend
        bool operator==(T const & lhs, with_old_value const & rhs)
        {
            return lhs == rhs.value();
        }

        constexpr friend
        bool operator<(with_old_value const & lhs, T const & rhs)
        {
            return lhs.value() < rhs;
        }

        constexpr friend
        bool operator<(T const & lhs, with_old_value const & rhs)
        {
            return lhs < rhs.value();
        }

    public:
        // Создание, копирование, уничтожение
        with_old_value() = default;

        constexpr explicit with_old_value(T value)
         : value_(std::move(value))
         , old_value_(this->value_)
        {}

        constexpr with_old_value & operator=(T value)
        {
            this->value_ = std::move(value);

            return *this;
        }

        // Немодифицирующие операции
        constexpr T & value()
        {
            return this->value_;
        }

        constexpr T const & value() const
        {
            return this->value_;
        }

        constexpr T const & old_value() const
        {
            return this->old_value_;
        }

        // Принятие/сброс изменений
        constexpr void commit()
        {
            this->old_value_ = this->value_;
        }

        constexpr void revert()
        {
            this->value_ = this->old_value_;
        }

    private:
        T value_{};
        T old_value_{};
    };
}

#endif
// Z_SAGA_UTILITY_WITH_OLD_VALUE_HPP_INCLUDED
