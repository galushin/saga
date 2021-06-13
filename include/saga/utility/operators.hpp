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

#ifndef Z_SAGA_UTILITY_OPERATORS_HPP_INCLUDED
#define Z_SAGA_UTILITY_OPERATORS_HPP_INCLUDED

/** @file saga/utility/operators
 @brief Средства для упрощения определения итераторов
*/

#include <type_traits>

namespace saga
{
    namespace detail
    {
        template <class Base, class Tag>
        struct inherit
         : public Base
        {};

        template <class Tag>
        struct inherit<void, Tag>
        {};
    }
    // namespace detail

    namespace operators
    {
        template <class T, class Base = void>
        struct equality_comparable
         : detail::inherit<Base, equality_comparable<T, Base>>
        {
            friend
            constexpr bool
            operator!=(T const & lhs, T const & rhs) noexcept(noexcept(!(lhs == rhs)))
            {
                return !(lhs == rhs);
            }
        };

        template <class T, class Base = void>
        struct less_than_comparable
         : detail::inherit<Base, less_than_comparable<T, Base>>
        {
            friend
            constexpr bool
            operator>(T const & lhs, T const & rhs) noexcept(noexcept(!(lhs < rhs)))
            {
                return rhs < lhs;
            }

            friend
            constexpr bool
            operator<=(T const & lhs, T const & rhs) noexcept(noexcept(!(rhs < lhs)))
            {
                return !(rhs < lhs);
            }

            friend
            bool operator>=(T const & lhs, T const & rhs) noexcept(noexcept(!(lhs < rhs)))
            {
                return !(lhs < rhs);
            }
        };

        template <class T, class Base = void>
        struct incrementable
         : detail::inherit<Base, incrementable<T, Base>>
        {
            friend constexpr T operator++(T & obj, int)
            {
                auto old_value = obj;
                ++ obj;
                return old_value;
            }
        };

        template <class T, class Base = void>
        struct decrementable
         : detail::inherit<Base, decrementable<T, Base>>
        {
            friend constexpr T operator--(T & obj, int)
            {
                auto old_value = obj;
                -- obj;
                return old_value;
            }
        };

        template <class T, class U, class Base = void>
        struct addable_with
         : detail::inherit<Base, addable_with<T, U, Base>>
        {
            friend constexpr T operator+(T lhs, U const & rhs)
            {
                lhs += rhs;
                return lhs;
            }

            friend constexpr T operator+(U const & lhs, T rhs)
            {
                rhs += lhs;
                return rhs;
            }
        };

        template <class T, class U, class Base = void>
        struct subtractable_with
         : detail::inherit<Base, subtractable_with<T, U, Base>>
        {
            friend constexpr T operator-(T lhs, U const & rhs)
            {
                lhs -= rhs;
                return lhs;
            }
        };

        template <class T, class Base = void>
        struct totally_ordered
         : equality_comparable<T, less_than_comparable<T, Base>>
        {};

        template <class T, class Base = void>
        struct unit_steppable
         : incrementable<T, decrementable<T, Base>>
        {};

        template <class T, class U, class Base = void>
        struct additive_with
         : addable_with<T, U, subtractable_with<T, U, Base>>
        {};
    }
    //namespace operators

    /* В отличие от классов пространства имён operators позволяет легко определить шаблоны
    операторов
    */
    namespace rel_ops
    {
        template <class Tag, class Base = void>
        struct enable_adl
         : detail::inherit<Base, Tag>
        {};

        template <class T, class U>
        constexpr auto operator!=(T const & lhs, U const & rhs) noexcept(noexcept(!(lhs == rhs)))
        -> std::enable_if_t<std::is_same<decltype(lhs == rhs), bool>{}, bool>
        {
            return !(lhs == rhs);
        }

        template <class T, class U>
        constexpr auto operator>(T const & lhs, U const & rhs) noexcept(noexcept(rhs < lhs))
        -> std::enable_if_t<std::is_same<decltype(rhs < lhs), bool>{}, bool>
        {
            return rhs < lhs;
        }

        template <class T, class U>
        constexpr auto operator<=(T const & lhs, U const & rhs) noexcept(noexcept(!(rhs < lhs)))
        -> std::enable_if_t<std::is_same<decltype(rhs < lhs), bool>{}, bool>
        {
            return !(rhs < lhs);
        }

        template <class T, class U>
        constexpr auto operator>=(T const & lhs, U const & rhs) noexcept(noexcept(!(lhs < rhs)))
        -> std::enable_if_t<std::is_same<decltype(lhs < rhs), bool>{}, bool>
        {
            return !(lhs < rhs);
        }
    }
    //namespace rel_ops
}
//namespace saga

#endif
// Z_SAGA_UTILITY_OPERATORS_HPP_INCLUDED
