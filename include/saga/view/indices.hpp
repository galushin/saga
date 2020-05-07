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

#ifndef Z_SAGA_VIEW_INDICES_HPP_INCLUDED
#define Z_SAGA_VIEW_INDICES_HPP_INCLUDED

#include <saga/detail/static_empty_const.hpp>
#include <saga/iterator.hpp>

#include <utility>

namespace saga
{
    namespace detail
    {
        template <class Incrementable>
        class iota_iterator
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
            constexpr bool operator!=(iota_iterator<Incrementable> const & lhs,
                                      iota_iterator<Incrementable2> const & rhs)
            {
                return !(lhs == rhs);
            }

            template <class Incrementable2>
            friend
            constexpr bool operator<(iota_iterator<Incrementable> const & lhs,
                                     iota_iterator<Incrementable2> const & rhs)
            {
                return *lhs < *rhs;
            }

            template <class Incrementable2>
            friend
            constexpr bool operator>(iota_iterator<Incrementable> const & lhs,
                                     iota_iterator<Incrementable2> const & rhs)
            {
                return rhs < lhs;
            }

            template <class Incrementable2>
            friend
            constexpr bool operator<=(iota_iterator<Incrementable> const & lhs,
                                      iota_iterator<Incrementable2> const & rhs)
            {
                return !(lhs > rhs);
            }

            template <class Incrementable2>
            friend
            constexpr bool operator>=(iota_iterator<Incrementable> const & lhs,
                                      iota_iterator<Incrementable2> const & rhs)
            {
                return !(lhs < rhs);
            }


        public:
            // Типы
            using iterator_category = std::random_access_iterator_tag;
            using value_type = Incrementable;
            using difference_type = decltype(std::declval<value_type>() - std::declval<value_type>());
            using pointer = Incrementable const *;
            using reference = Incrementable const &;

            // Создание, копирование, уничтожение
            constexpr explicit iota_iterator(Incrementable value)
             : value_(std::move(value))
            {}

            template <class OtherIncrementable,
                      typename = std::enable_if_t<std::is_constructible<Incrementable, OtherIncrementable const &>{}>>
            constexpr iota_iterator(iota_iterator<OtherIncrementable> const & other)
             : value_{*other}
            {}

            template <class OtherIncrementable,
                      typename = std::enable_if_t<std::is_constructible<Incrementable, OtherIncrementable const &>{}
                                                && std::is_assignable<Incrementable&, OtherIncrementable const &>{}>>
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

            // Двусторонний итератор
            constexpr iota_iterator &operator--()
            {
                -- this->value_;
                return *this;
            }

            // Итератор приозвольного доступа
            constexpr iota_iterator operator+(difference_type num) const
            {
                auto result = *this;
                result += num;
                return result;
            }

            constexpr iota_iterator & operator+=(difference_type num)
            {
                this->value_ += num;
                return *this;
            }

            constexpr iota_iterator operator-(difference_type num) const
            {
                auto result = *this;
                result -= num;
                return result;
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

        template <class Incrementable>
        class iota_view
        {
        public:
            // Типы
            using iterator = saga::detail::iota_iterator<Incrementable>;

            // Создание, копирование, уничтожение
            iota_view(Incrementable from, Incrementable to)
             : from_(std::move(from))
             , to_(std::move(to))
            {}

            // Итераторы
            iterator begin() const
            {
                return iterator{this->from_};
            }

            iterator end() const
            {
                return iterator{this->to_};
            }

        private:
            Incrementable from_ = {};
            Incrementable to_ = {};
        };
    }

    namespace view
    {
        struct indices_fn
        {
        public:
            template <class Incrementable>
            auto operator()(Incrementable from, Incrementable to) const
            -> saga::detail::iota_view<Incrementable>
            {
                return {std::move(from), std::move(to)};
            }

            template <class Incrementable>
            auto operator()(Incrementable num) const
            -> saga::detail::iota_view<Incrementable>
            {
                return (*this)(Incrementable(), num);
            }
        };

        struct indices_of_fn
        {
        public:
            template <class SizedContainer>
            auto operator()(SizedContainer const & container) const
            {
                return saga::view::indices_fn{}(saga::size(container));
            }
        };

        namespace
        {
            constexpr auto const & indices = detail::static_empty_const<indices_fn>::value;
            constexpr auto const & indices_of = detail::static_empty_const<indices_of_fn>::value;
        }
    }
    // namespace view
}
// namespace saga

#endif
// Z_SAGA_VIEW_INDICES_HPP_INCLUDED
