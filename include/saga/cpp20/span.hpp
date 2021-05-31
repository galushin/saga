/* (c) 2019-2021 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_CPP20_SPAN_HPP_INCLUDED
#define Z_SAGA_CPP20_SPAN_HPP_INCLUDED

/** @file saga/cpp20/span.hpp
 @brief Реализация std::span из C++20, описанный в:
  www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/n4741.pdf
  www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1024r2.pdf
*/

#include <saga/algorithm.hpp>
#include <saga/detail/default_ctor_enabler.hpp>
#include <saga/cursor/subrange.hpp>
#include <saga/utility/operators.hpp>

#include <cassert>
#include <cstddef>

#include <algorithm>
#include <array>
#include <iterator>
#include <type_traits>

namespace saga
{
    // @todo Должно быть inline
    constexpr std::ptrdiff_t dynamic_extent = -1;

    namespace detail
    {
        template <class Pointer, std::ptrdiff_t extent>
        struct span_base
        {
            span_base() = default;

            constexpr span_base(Pointer data, std::ptrdiff_t size)
             : ptr(data)
            {
                assert(size == extent);
            }

            Pointer ptr = nullptr;
            static constexpr std::ptrdiff_t size = extent;
        };

        template <class Pointer>
        struct span_base<Pointer, dynamic_extent>
        {
            span_base() = default;

            constexpr span_base(Pointer data, std::ptrdiff_t size)
             : ptr(data)
             , size(size)
            {}

            Pointer ptr = nullptr;
            std::ptrdiff_t size = 0;
        };
    }
    // namespace detail

    template <class ElementType, std::ptrdiff_t Extent = dynamic_extent>
    class span
     : detail::default_ctor_enabler<(Extent <= 0)>
    {
        using default_ctor_enabler = detail::default_ctor_enabler<(Extent <= 0)>;

    public:
        // Константы и типы
        using element_type = ElementType;
        using value_type = std::remove_cv_t<element_type>;
        using index_type = std::ptrdiff_t;
        using difference_type = std::ptrdiff_t;
        using pointer = element_type *;
        using reference = element_type &;
        using iterator = element_type *;
        using const_iterator = element_type const *;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        static constexpr index_type extent = Extent;

        // Конструкторы, копирование и присваивание
        constexpr span() noexcept = default;

        // @todo Покрыть тестами, что этот конструктор должен быть constexpr
        span(pointer ptr, index_type count)
         : default_ctor_enabler(0)
         , base_(ptr, count)
        {}

        span(pointer first, pointer last)
         : span(first, last - first)
        {}

        /**
        @todo Реализовать ограничения типа
        */
        template <std::size_t N,
                  std::enable_if_t<extent == dynamic_extent || N == extent, bool> = true>
        span(element_type (&arr)[N]) noexcept
         : span(arr, N)
        {}

        /**
        @todo Реализовать ограничения типа
        */
        template <std::size_t N>
        span(std::array<value_type, N> & arr) noexcept
         : span(arr.data(), N)
        {}

        /**
        @todo Реализовать ограничения типа
        */
        template <std::size_t N>
        span(std::array<value_type, N> const & arr) noexcept
         : span(arr.data(), N)
        {}

        /**
        @todo Реализовать ограничения типа
        */
        template <class Container,
                  std::enable_if_t<!std::is_const<Container>{}, bool> = true,
                  std::enable_if_t<!std::is_array<Container>{}, bool> = true,
                  std::enable_if_t<std::is_convertible<typename Container::value_type(*)[], element_type(*)[]>::value, bool> = true>
        span(Container & cont)
         : span(cont.data(), cont.size())
        {}

        /**
        @todo Реализовать ограничения типа
        */
        template <class Container,
                  std::enable_if_t<!std::is_array<Container>{}, bool> = true,
                  std::enable_if_t<std::is_convertible<typename Container::value_type const(*)[], element_type(*)[]>::value, bool> = true>
        span(Container const & cont)
         : span(cont.data(), cont.size())
        {}

        span(span const & other) noexcept = default;

        /**
        @todo Реализовать ограничения типа
        */
        template <class OtherElementType,
                  std::enable_if_t<std::is_convertible<OtherElementType(*)[], element_type(*)[]>::value, bool> = true>
        span(span<OtherElementType> const & other) noexcept
         : span(other.data(), other.size())
        {}

        // Подинтервалы
        span<element_type, dynamic_extent>
        first(index_type count) const
        {
            assert(0 <= count && count <= this->size());

            return {this->data(), count};
        }

        span<element_type, dynamic_extent>
        last(index_type count) const
        {
            assert(0 <= count && count <= this->size());

            return {this->data() + (this->size() - count), count};
        }

        span<element_type, dynamic_extent>
        subspan(index_type offset, index_type count = dynamic_extent) const
        {
            assert(0 <= offset && offset <= this->size());
            assert(count == saga::dynamic_extent || (0 <= count && offset + count <= this->size()));

            auto const actual_count = (count == dynamic_extent) ? this->size() - offset : count;

            return {this->data() + offset, actual_count};
        }

        // Свойства
        constexpr index_type size() const noexcept
        {
            return this->base_.size;
        }

        constexpr index_type size_bytes() const noexcept
        {
            return this->size() * sizeof(element_type);
        }

        constexpr bool empty() const noexcept
        {
            return this->size() == 0;
        }

        // Доступ к элементам
        reference operator[](index_type index) const
        {
            assert(0 <= index && index < this->size());

            return *(this->data() + index);
        }

        constexpr pointer data() const noexcept
        {
            return this->base_.ptr;
        }

        // Итераторы
        iterator begin() const noexcept
        {
            return iterator(this->data());
        }

        iterator end() const noexcept
        {
            return this->begin() + this->size();
        }

        const_iterator cbegin() const noexcept
        {
            return const_iterator(this->data());
        }

        const_iterator cend() const noexcept
        {
            return this->cbegin() + this->size();
        }

        reverse_iterator rbegin() const noexcept
        {
            return reverse_iterator(this->end());
        }

        reverse_iterator rend() const noexcept
        {
            return reverse_iterator(this->begin());
        }

        const_reverse_iterator crbegin() const noexcept
        {
            return const_reverse_iterator(this->cend());
        }

        const_reverse_iterator crend() const noexcept
        {
            return const_reverse_iterator(this->cbegin());
        }

    private:
        detail::span_base<pointer, Extent> base_{};
    };
}
// namespace saga

#endif
// Z_SAGA_CPP20_SPAN_HPP_INCLUDED
