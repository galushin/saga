/* (c) 2019 Галушин Павел Викторович, galushin@gmail.com

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

#include <cassert>
#include <cstddef>

#include <array>
#include <iterator>
#include <type_traits>

namespace saga
{
    // @todo Должно быть inline
    constexpr std::ptrdiff_t dynamic_extent = -1;

    namespace detail
    {
        // @todo Возможно, перенести в файл, так как это может быть полезно в других местах
        template <bool Enable>
        struct default_ctor_enabler
        {};

        template <>
        struct default_ctor_enabler<false>
        {
            default_ctor_enabler() = delete;
        };
    }
    // namespace details

    template <class ElementType, std::ptrdiff_t Extent = dynamic_extent>
    class span
     : detail::default_ctor_enabler<(Extent <= 0)>
    {
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
         : data_(ptr)
         , size_(count)
        {}

        span(pointer first, pointer last)
         : data_(first)
         , size_(last - first)
        {}

        template <std::size_t N>
        span(element_type (&arr)[N]) noexcept
         : span(arr, N)
        {}

        template <std::size_t N>
        span(std::array<value_type, N> & arr) noexcept
         : span(arr.data(), N)
        {}

        template <std::size_t N>
        span(std::array<value_type, N> const & arr) noexcept
         : span(arr.data(), N)
        {}

        /**
        @todo Реализовать ограничения типа
        */
        template <class Container>
        span(Container & cont)
         : span(cont.data(), cont.size())
        {}

        /**
        @todo Реализовать ограничения типа
        */
        template <class Container,
                  std::enable_if_t<std::is_convertible<typename Container::value_type const(*)[], element_type(*)[]>::value, bool> = true>
        span(Container const & cont)
         : span(cont.data(), cont.size())
        {}

        span(span const & other) noexcept = default;

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
            return this->size_;
        }

        constexpr index_type size_bytes() const
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
            return this->data_;
        }

    private:
        pointer data_ = nullptr;
        // @todo Может быть не хранить это, когда Extent >= 0
        index_type size_ = 0;
    };
}
// namespace saga

#endif
// Z_SAGA_CPP20_SPAN_HPP_INCLUDED
