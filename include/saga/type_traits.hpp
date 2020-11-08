/* (c) 2019-2020 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_TYPE_TRAITS_HPP_INCLUDED
#define Z_SAGA_TYPE_TRAITS_HPP_INCLUDED

/** @file saga/type_traits.hpp
 @brief Характеристики-типов
*/

#include <utility>

namespace saga
{
    // priority_tag
    /** @brief Тип-тэг для задания приоритета перегрузок
    @tparam P уровень приоритета: чем больше это значение, тем выше приоритет
    */
    template <std::size_t P>
    struct priority_tag
     : priority_tag<P-1>
    {};

    /// @brief Специализация для наименьшего приоритета
    template <>
    struct priority_tag<0>
    {};

    // bool_constant
    template <bool B>
    using bool_constant = std::integral_constant<bool, B>;
    /** @brief Определение типа для предсставления размера, используемого данным типом, например,
    контейнером

    Определяет вложенный тип @c type, определённый по следующему правилу.
    Если у @c T определён вложенный тип @c size_type, то <tt>type = typename T::size_type</tt>.
    Иначе @c type совпадает с типом, возвращаемым функцией-членом @c size типа @c T

    @tparam T тип (предположительно, тип контейнера)
    */
    template <class T>
    struct size_type
    {
    private:
        template <class U>
        static auto impl(priority_tag<0>) -> decltype(std::declval<U>().size());

        template <class U>
        static auto impl(priority_tag<1>) -> typename U::size_type;

    public:
        using type = decltype(size_type::impl<T>(priority_tag<1>{}));
    };

    // void_t
    template <typename... Types>
    struct declare_void
    {
        using type = void;
    };

    template <typename... Types>
    using void_t = typename declare_void<Types...>::type;

    // remove_cvref
    template <class T>
    struct remove_cvref
     : std::remove_cv<std::remove_reference_t<T>>
    {};

    template <class T>
    using remove_cvref_t = typename remove_cvref<T>::type;

    // is_swappable
    namespace detail
    {
        namespace swap_adl_ns
        {
            template <class T, class U, class SFINAE = void>
            struct is_swappable_with
             : std::false_type
            {};

            using std::swap;

            template <class T, class U>
            using swap_enabler = void_t<decltype(swap(std::declval<T>(), std::declval<U>())),
                                        decltype(swap(std::declval<U>(), std::declval<T>()))>;

            template <class T, class U>
            struct is_swappable_with<T, U, swap_enabler<T, U>>
             : std::true_type
            {};

            template <class T, class U, class SFINAE = void>
            struct is_nothrow_swappable_with
             : std::false_type
            {};

            template <class T, class U>
            struct is_nothrow_swappable_with<T, U, std::enable_if_t<is_swappable_with<T, U>::value>>
             : saga::bool_constant<noexcept(swap(std::declval<T>(), std::declval<U>()))
                                   && noexcept(swap(std::declval<U>(), std::declval<T>()))>
            {};
        }

        template <class T, class SFINAE = void>
        struct is_swappable
         : std::false_type
        {};

        template <class T>
        struct is_swappable<T, void_t<T&>>
         : detail::swap_adl_ns::is_swappable_with<T &, T &>
        {};

        template <class T, class SFINAE = void>
        struct is_nothrow_swappable
         : std::false_type
        {};

        template <class T>
        struct is_nothrow_swappable<T, void_t<T&>>
         : detail::swap_adl_ns::is_nothrow_swappable_with<T &, T &>
        {};

    }

    template <class T, class U>
    struct is_swappable_with
     : detail::swap_adl_ns::is_swappable_with<T, U>
    {};

    template <class T>
    struct is_swappable
     : detail::is_swappable<T>
    {};

    template <class T, class U>
    struct is_nothrow_swappable_with
     : detail::swap_adl_ns::is_nothrow_swappable_with<T, U>
    {};

    template <class T>
    struct is_nothrow_swappable
     : detail::is_nothrow_swappable<T>
    {};

    // is_equality_comparable
    namespace detail
    {
        template <class T, class SFINAE = void>
        struct is_equality_comparable
         : std::false_type
        {};

        template <class T>
        struct is_equality_comparable<T, void_t<decltype(std::declval<T const>() == std::declval<T const>())>>
         : std::is_same<bool, decltype(std::declval<T const>() == std::declval<T const>())>
        {};
    }
    // namespace detail

    template <class T>
    struct is_equality_comparable
     : detail::is_equality_comparable<T>
    {};

    // Идиома детектирования
    struct nonesuch
    {
        ~nonesuch() = delete;
        nonesuch(nonesuch const &) = delete;
        void operator=(nonesuch const &) = delete;
    };
}
// namespace saga

#endif
// Z_SAGA_TYPE_TRAITS_HPP_INCLUDED
