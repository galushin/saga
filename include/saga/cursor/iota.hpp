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

#ifndef Z_SAGA_CURSOR_IOTA_HPP_INCLUDED
#define Z_SAGA_CURSOR_IOTA_HPP_INCLUDED

/** @file saga/cursor/iota.hpp
 @brief Бесконечный курсор в виде последовательности целых чисел, начинающихся с заданного
*/

#include <saga/cursor/cursor_traits.hpp>
#include <saga/cursor/cursor_facade.hpp>
#include <saga/iterator.hpp>
#include <saga/utility/operators.hpp>

#include <iterator>
#include <limits>

namespace saga
{
    namespace detail
    {
        template <class T, class SFINAE = void>
        struct iota_iterator_category
        {};

        template <class T>
        struct iota_iterator_category<T,
            std::enable_if_t<!std::is_same<T, bool>{}
                             && (std::is_integral<T>{} ||(std::numeric_limits<T>::is_specialized
                                                          && std::numeric_limits<T>::is_integer))>>
        {
            using type = std::random_access_iterator_tag;
        };

        template <class T>
        struct iota_iterator_category<T, std::void_t<typename std::iterator_traits<T>::iterator_category>>
        {
            using type = typename std::iterator_traits<T>::iterator_category;
        };
    }
    // namespace detail

    template <class Incrementable>
    class iota_iterator
     : operators::additive_with<iota_iterator<Incrementable>
                                , incrementable_difference_t<Incrementable>
     , operators::unit_steppable<iota_iterator<Incrementable>
     , rel_ops::enable_adl<iota_iterator<Incrementable>>>>
    {
        template <class Incrementable2>
        friend
        constexpr bool operator==(iota_iterator<Incrementable> const & lhs,
                                  iota_iterator<Incrementable2> const & rhs)
        {
            return *lhs == *rhs;
        }

        template <class Incrementable2>
        friend
        constexpr bool operator<(iota_iterator<Incrementable> const & lhs,
                                 iota_iterator<Incrementable2> const & rhs)
        {
            return *lhs < *rhs;
        }

    public:
        // Типы
        using iterator_category = typename detail::iota_iterator_category<Incrementable>::type;

        using value_type = Incrementable;

        using difference_type = saga::incrementable_difference_t<Incrementable>;

        using pointer = Incrementable const *;
        using reference = Incrementable const &;

        // Создание, копирование, уничтожение
        constexpr explicit iota_iterator(Incrementable value)
         : value_(std::move(value))
        {}

        template <class OtherIncrementable,
                  typename = std::enable_if_t<std::is_constructible<Incrementable,
                                                                    OtherIncrementable const &>{}>>
        constexpr iota_iterator(iota_iterator<OtherIncrementable> const & other)
         : value_{*other}
        {}

        template <class OtherIncrementable,
                  typename = std::enable_if_t<std::is_constructible<Incrementable,
                                                                    OtherIncrementable const &>{}
                                            && std::is_assignable<Incrementable&,
                                                                  OtherIncrementable const &>{}>>
        constexpr iota_iterator & operator=(iota_iterator<OtherIncrementable> const & other)
        {
            this->value_ = *other;
            return *this;
        }

        // Итератор
        constexpr iota_iterator & operator++()
        {
            ++ this->value_;
            return *this;
        }

        constexpr reference operator*() const
        {
            return this->value_;
        }

        constexpr pointer operator->() const
        {
            return std::addressof(**this);
        }

        // Двусторонний итератор
        constexpr iota_iterator &operator--()
        {
            -- this->value_;
            return *this;
        }

        // Итератор приозвольного доступа
        constexpr iota_iterator & operator+=(difference_type num)
        {
            this->value_ += num;
            return *this;
        }

        constexpr iota_iterator & operator-=(difference_type num)
        {
            this->value_ -= num;
            return *this;
        }

        constexpr value_type operator[](difference_type num) const
        {
            return this->value_ + num;
        }

        template <class Incrementable2>
        friend
        constexpr auto operator-(iota_iterator<Incrementable> const & lhs,
                                 iota_iterator<Incrementable2> const & rhs)
        -> decltype(std::declval<Incrementable>() - std::declval<Incrementable2>())
        {
            return *lhs - *rhs;
        }

    private:
        Incrementable value_;
    };

    struct unreachable_sentinel
    {
        template <class T>
        friend constexpr bool operator==(T const &, unreachable_sentinel)
        {
            return false;
        }

        template <class T>
        friend constexpr bool operator==(unreachable_sentinel, T const &)
        {
            return false;
        }
    };

    template <class Incrementable, class Sentinel = Incrementable>
    class iota_cursor
     : saga::cursor_facade<iota_cursor<Incrementable, Sentinel>, Incrementable const &>
    {
    public:
        // Типы
        using cursor_category = std::random_access_iterator_tag;
        using cursor_cardinality
            = saga::conditional_t<std::is_same<Sentinel, saga::unreachable_sentinel>{}
                                 , saga::infinite_cursor_cardinality_tag
                                 , saga::finite_cursor_cardinality_tag>;

        using difference_type = Incrementable;
        using reference = Incrementable const &;
        using value_type = Incrementable;

        using iterator = saga::iota_iterator<Incrementable>;
        using sentinel = saga::conditional_t<std::is_same<Sentinel, saga::unreachable_sentinel>{}
                                            , saga::unreachable_sentinel
                                            , saga::iota_iterator<Sentinel>>;

        // Создание, копирование, уничтожение
        constexpr explicit iota_cursor(Incrementable first, Sentinel last)
         : cur_(std::move(first))
         , cur_old_(this->cur_)
         , last_(std::move(last))
        {}

        // Вид
        constexpr iterator begin() const
        {
            return iterator(this->cur_);
        }

        constexpr sentinel end() const
        {
            return sentinel(this->last_);
        }

        // Курсор ввода
        constexpr bool operator!() const
        {
            return this->cur_ == this->last_;
        }

        constexpr reference front() const
        {
            return this->cur_;
        }

        constexpr void drop_front()
        {
            ++ cur_;
        }

        // Прямой курсор
        iota_cursor<Incrementable, Incrementable>
        dropped_front() const
        {
            return iota_cursor<Incrementable, Incrementable>(this->cur_old_, this->cur_);
        }

        void forget_front()
        {
            this->cur_old_ = this->cur_;
        }

        // Курсор произвольного доступа
        constexpr void drop_front(difference_type num)
        {
            this->cur_ += std::move(num);
        }

        difference_type size() const
        {
            return this->last_ - this->cur_;
        }

    private:
        Incrementable cur_{};
        Incrementable cur_old_{};
        Sentinel last_{};
    };

    namespace cursor
    {
        template <class Incrementable>
        constexpr iota_cursor<Incrementable, saga::unreachable_sentinel>
        iota(Incrementable num)
        {
            using Cursor = iota_cursor<Incrementable, unreachable_sentinel>;
            return Cursor(std::move(num), unreachable_sentinel{});
        }
    }
    // namespace cursor
}
// namespace saga

#endif
// Z_SAGA_CURSOR_IOTA_HPP_INCLUDED
