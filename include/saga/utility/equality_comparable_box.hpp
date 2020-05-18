/* (c) 2020 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_UTILITY_EQUALITY_COMPARABLE_BOX_HPP_INCLUDED
#define Z_SAGA_UTILITY_EQUALITY_COMPARABLE_BOX_HPP_INCLUDED

/** @file saga/utility/equality_comparable_box.hpp
 @brief Класс-обёртка, гарантирована добавляющая оператор равно.
*/

#include <saga/type_traits.hpp>
#include <saga/utility/operators.hpp>

#include <utility>

namespace saga
{
    namespace detail
    {
        struct generic_eqaulity
        {
        private:
            template <class T>
            constexpr static auto impl(T const & lhs, T const & rhs, priority_tag<1>)
            -> std::enable_if_t<saga::is_equality_comparable<T>{}, bool>
            {
                return lhs == rhs;
            }

            template <class T>
            constexpr static auto impl(T const &, T const &, priority_tag<0>)
            -> std::enable_if_t<std::is_empty<T>{}, bool>
            {
                return true;
            }

        public:
            template <class T>
            constexpr bool operator()(T const & lhs, T const & rhs) const
            {
                return this->impl(lhs, rhs, priority_tag<1>{});
            }
        };
    }
    // namespace detail

    // @todo Оптимизация пустого объекта
    template <class T>
    class equality_comparable_box
     : saga::operators::equality_comparable<equality_comparable_box<T>>
    {
    public:
        // Типы
        using value_type = T;

        // Конструктор
        constexpr equality_comparable_box()
         : value_()
        {}

        // @todo Условный explicit?
        constexpr explicit equality_comparable_box(T value)
         : value_(std::move(value))
        {}

        // Доступ к значению
        T & value() &;

        constexpr T const & value() const &
        {
            return this->value_;
        }

        T && value() &&;

        // Равенство и неравенство
        friend
        constexpr bool operator==(equality_comparable_box const & lhs,
                                  equality_comparable_box const & rhs)
        {
            return detail::generic_eqaulity{}(lhs.value(), rhs.value());
        }

    private:
        T value_;
    };
}
// namespace saga

#endif
// Z_SAGA_UTILITY_EQUALITY_COMPARABLE_BOX_HPP_INCLUDED
