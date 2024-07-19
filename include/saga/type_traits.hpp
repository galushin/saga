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

#ifndef Z_SAGA_TYPE_TRAITS_HPP_INCLUDED
#define Z_SAGA_TYPE_TRAITS_HPP_INCLUDED

/** @file saga/type_traits.hpp
 @brief Характеристики-типов
*/

#include <type_traits>
#include <utility>

namespace saga
{
    // conditional_t
    namespace detail
    {
        template <bool B>
        struct conditional
        {
            template <class T, class F>
            using type = T;
        };

        template <>
        struct conditional<false>
        {
            template <class T, class F>
            using type = F;
        };
    }
    // namespace detail

    template <bool B, class T, class F>
    using conditional_t = typename detail::conditional<B>::template type<T, F>;

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

    // Идиома детектирования
    struct nonesuch
    {
        ~nonesuch() = delete;
        nonesuch(nonesuch const &) = delete;
        void operator=(nonesuch const &) = delete;
    };

    namespace detail
    {
        template <class SFINAE, class Default, template <class...> class Op, class... Args>
        struct detector
        {
            using value_t = std::false_type;

            using type = Default;
        };

        template <class Default, template <class...> class Op, class... Args>
        struct detector<std::void_t<Op<Args...>>, Default, Op, Args...>
        {
            using value_t = std::true_type;

            using type = Op<Args...>;
        };
    }
    // namespace detail

    template <class Default, template <class...> class Op, class... Args>
    using detected_or = saga::detail::detector<void, Default, Op, Args...>;

    template <template <class...> class Op, class... Args>
    using is_detected = typename detected_or<nonesuch, Op, Args...>::value_t;

    template <template <class...> class Op, class... Args>
    using detected_t = typename detected_or<nonesuch, Op, Args...>::type;

    template <class Default, template <class...> class Op, class... Args>
    using detected_or_t = typename saga::detected_or<Default, Op, Args...>::type;

    template <class Expected, template <class...> class Op, class... Args>
    using is_detected_exact = std::is_same<Expected, detected_t<Op, Args...>>;

    template <class Expected, template <class...> class Op, class... Args>
    using is_detected_convertible = std::is_convertible<saga::detected_t<Op, Args...>, Expected>;

#if __cpp_variable_templates >= 201304
    template <template <class...> class Op, class... Args>
    constexpr bool is_detected_v = is_detected<Op, Args...>::value;

    template <class Expected, template <class...> class Op, class... Args>
    constexpr bool is_detected_exact_v = is_detected_exact<Expected, Op, Args...>::value;

    template <class Expected, template <class...> class Op, class... Args>
    constexpr bool is_detected_convertible_v = is_detected_convertible<Expected, Op, Args...>::value;
#endif
// __cpp_variable_templates

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

    // remove_cvref
    template <class T>
    struct remove_cvref
     : std::remove_cv<std::remove_reference_t<T>>
    {};

    template <class T>
    using remove_cvref_t = typename remove_cvref<T>::type;

    // is_equality_comparable
    namespace detail
    {
        template <class T>
        using equality_op_result = decltype(std::declval<T const>() == std::declval<T const>());
    }
    // namespace detail

    // common_reference
    template <class... Types>
    struct common_reference
    {};

    template <class T>
    struct common_reference<T&, T&>
    {
        using type = T &;
    };

    template <class... Types>
    using common_reference_t = typename common_reference<Types...>::type;

    template <class T>
    struct is_equality_comparable
     : is_detected_exact<bool, detail::equality_op_result, T>
    {};

    template <class T, class U, class SFINAE = void>
    struct is_equality_comparable_with
     : std::false_type
    {};

    namespace detail
    {
        template <class T, class U>
        using common_reference_helper
            = saga::common_reference_t<std::remove_reference_t<T>&, std::remove_reference_t<U>&>;
    }
    // namespace detail

    template <class T, class U>
    struct is_equality_comparable_with<T, U, std::void_t<detail::common_reference_helper<T, U>>>
     : std::conjunction<saga::is_equality_comparable<T>
                       ,saga::is_equality_comparable<U>
                       ,saga::is_equality_comparable<detail::common_reference_helper<T, U>>>
    {};

    // is_totally_ordered
    namespace detail
    {
        template <class T, class U>
        using partial_order_compare_t = std::tuple<decltype(std::declval<T>() <  std::declval<U>())
                                                  ,decltype(std::declval<T>() >  std::declval<U>())
                                                  ,decltype(std::declval<T>() <= std::declval<U>())
                                                  ,decltype(std::declval<T>() >= std::declval<U>())
                                                  ,decltype(std::declval<U>() <  std::declval<T>())
                                                  ,decltype(std::declval<U>() >  std::declval<T>())
                                                  ,decltype(std::declval<U>() <= std::declval<T>())
                                                  ,decltype(std::declval<U>() >= std::declval<T>())
                                                  >;

        template <class T, class U, class SFINAE = void>
        struct is_partially_ordered_with
         : std::false_type
        {};

        template <class T, class U>
        struct is_partially_ordered_with<T, U, std::void_t<detail::partial_order_compare_t<T, U>>>
         : std::is_same<detail::partial_order_compare_t<T, U>
                       ,std::tuple<bool, bool, bool, bool, bool, bool, bool, bool>>
        {};
    }
    //namespace detail

    template <class T>
    struct is_totally_ordered
     : std::conjunction<saga::is_equality_comparable<T>
                       ,saga::detail::is_partially_ordered_with<T,T>>
    {};

    template <class T, class U, class SFINAE = void>
    struct is_totally_ordered_with
     : std::false_type
    {};

    template <class T, class U>
    struct is_totally_ordered_with<T, U, std::void_t<detail::common_reference_helper<T, U>>>
     : std::conjunction<saga::is_totally_ordered<T>
                       ,saga::is_totally_ordered<U>
                       ,saga::is_equality_comparable_with<T, U>
                       ,saga::is_totally_ordered<detail::common_reference_helper<T, U>>
                       ,saga::detail::is_partially_ordered_with<T, U>>
    {};

    // is_specialization
    template <class Type, template <class...> class Generic>
    struct is_specialization_of
     : std::false_type
    {};

    template <template <class...> class Generic, class... Args>
    struct is_specialization_of<Generic<Args...>, Generic>
     : std::true_type
    {};

    // invoke_result, invoke_result_t
    namespace detail
    {
        template <class T>
        struct invoke_impl
        {
            template <class F, class... Args>
            constexpr static auto call(F && fun, Args&&... args)
            noexcept(noexcept(std::forward<F>(fun)(std::forward<Args>(args)...)))
            -> decltype(std::forward<F>(fun)(std::forward<Args>(args)...))
            {
                return std::forward<F>(fun)(std::forward<Args>(args)...);
            }
        };

        template <class MP, class C>
        struct invoke_impl<MP C::*>
        {
            template <class T, class Td = std::decay_t<T>
                     , class = std::enable_if_t<std::is_base_of<C, std::remove_reference_t<Td>>{}>>
            constexpr static auto get(T && t) noexcept -> T &&
            {
                return std::forward<T>(t);
            }

            template <class T, class Td = std::decay_t<T>
                     , class = std::enable_if_t<is_specialization_of<Td, std::reference_wrapper>{}>>
            constexpr static auto get(T && t) noexcept(noexcept(t.get())) -> decltype(t.get())
            {
                return std::forward<T>(t).get();
            }

            template <class T, class Td = std::decay_t<T>
                     , class = std::enable_if_t<!is_specialization_of<Td, std::reference_wrapper>{}>
                     , class = std::enable_if_t<!std::is_base_of<C, std::remove_reference_t<Td>>{}>>
            constexpr static auto get(T && t) noexcept(noexcept(*t)) -> decltype(*t)
            {
                return *std::forward<T>(t);
            }

            template <class T, class... Args, class MF
                      , class = std::enable_if_t<std::is_function<MF>{}>>
            constexpr static auto call(MF C::*pmf, T && obj, Args &&... args)
            noexcept(noexcept((invoke_impl::get(std::forward<T>(obj)).*pmf)(std::forward<Args>(args)...)))
            -> decltype((invoke_impl::get(std::forward<T>(obj)).*pmf)(std::forward<Args>(args)...))
            {
                return (invoke_impl::get(std::forward<T>(obj)).*pmf)(std::forward<Args>(args)...);
            }

            template <class Fn, class T>
            constexpr static auto call(Fn pmv, T && obj)
            noexcept(noexcept(invoke_impl::get(std::forward<T>(obj)).*pmv))
            -> decltype(invoke_impl::get(std::forward<T>(obj)).*pmv)
            {
                return invoke_impl::get(std::forward<T>(obj)).*pmv;
            }
        };

        template <class AlwaysVoid, class Fun, class... Args>
        struct invoke_result
        {};

        template <class Fun, class... Args>
        struct invoke_result<decltype(void(invoke_impl<std::decay_t<Fun>>::call(std::declval<Fun>()
                                                                                , std::declval<Args>()...)))
                            , Fun, Args...>
        {
            using type = decltype(invoke_impl<std::decay_t<Fun>>::call(std::declval<Fun>()
                                                                       , std::declval<Args>()...));
        };

        template <class Fun, class... Args>
        using invoke_result_t = typename detail::invoke_result<void, Fun, Args...>::type;
    }
    // namespace detail

    template <class F, class... Args>
    struct invoke_result
     : detail::invoke_result<void, F, Args...>
    {};

    template <class F, class... Args>
    using invoke_result_t = typename invoke_result<F, Args...>::type;

    // is_invocable
    template <class Fun, class... Args>
    struct is_invocable
     : saga::is_detected<detail::invoke_result_t, Fun, Args...>
    {};

    // is_nothrow_invocable
    namespace detail
    {
        template <class Enable, class Fn, class... Args>
        struct is_nothrow_invocable
         : std::false_type
        {};

        template <class Fun, class... Args>
        struct is_nothrow_invocable<std::enable_if_t<is_invocable<Fun, Args...>{}>, Fun, Args...>
         : std::bool_constant<noexcept(invoke_impl<std::decay_t<Fun>>::call(std::declval<Fun>()
                                                                            , std::declval<Args>()...))>
        {};
    }
    // namespace detail

    template <class Fn, class... Args>
    struct is_nothrow_invocable
     : detail::is_nothrow_invocable<void, Fn, Args...>
    {};
}
// namespace saga

#endif
// Z_SAGA_TYPE_TRAITS_HPP_INCLUDED
