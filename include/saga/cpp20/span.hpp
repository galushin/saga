#ifndef Z_SAGA_CPP20_SPAN_HPP_INCLUDED
#define Z_SAGA_CPP20_SPAN_HPP_INCLUDED

/** @file saga/cpp20/span.hpp
 @brief Реализация std::span из C++20, описанный в:
  www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/n4741.pdf
  www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1024r2.pdf
*/

#include <cstddef>
#include <iterator>
#include <type_traits>

namespace saga
{
    // @todo Должно быть inline
    constexpr std::ptrdiff_t dynamic_extent = -1;

    namespace detail
    {
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

        // Подинтервалы
        // Свойства
        constexpr index_type size() const noexcept
        {
            return this->size_;
        }

        // Доступ к элементам
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
