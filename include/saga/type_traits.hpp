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

    // void_t
    template <typename... Types>
    struct declare_void
    {
        using type = void;
    };

    template <typename... Types>
    using void_t = typename declare_void<Types...>::type;

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
        struct detector<saga::void_t<Op<Args...>>, Default, Op, Args...>
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
            using std::swap;

            template <class T, class U>
            using swap_void_t = void_t<decltype(swap(std::declval<T>(), std::declval<U>())),
                                        decltype(swap(std::declval<U>(), std::declval<T>()))>;

            template <class T, class U>
            struct is_swappable_with
             : saga::is_detected<swap_void_t, T, U>
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
    // namespace detail

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
        template <class T>
        using equality_op_result = decltype(std::declval<T const>() == std::declval<T const>());
    }
    // namespace detail

    template <class T>
    struct is_equality_comparable
     : is_detected_exact<bool, detail::equality_op_result, T>
    {};

    template <class Type, template <class...> class Generic>
    struct is_specialization_of
     : std::false_type
    {};

    template <template <class...> class Generic, class... Args>
    struct is_specialization_of<Generic<Args...>, Generic>
     : std::true_type
    {};

    // is_nothrow_invocable
    namespace detail
    {
        template <class Enable, class Fn, class... Args>
        struct is_nothrow_invocable
         : std::false_type
        {};

        // @todo Полная реализация
    }
    // namespace detail

    template <class Fn, class... Args>
    struct is_nothrow_invocable
     : detail::is_nothrow_invocable<void, Fn, Args...>
    {};

    // invoke_result, invoke_result_t
    namespace detail
    {
        template <class T>
        struct invoke_impl
        {
            template <class F, class... Args>
            static auto call(F && fun, Args&&... args)
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
            static auto get(T && t) noexcept -> T &&;

            template <class T, class Td = std::decay_t<T>
                     , class = std::enable_if_t<is_specialization_of<Td, std::reference_wrapper>{}>>
            static auto get(T && t) noexcept(noexcept(t.get())) -> decltype(t.get());

            template <class T, class Td = std::decay_t<T>
                     , class = std::enable_if_t<!is_specialization_of<Td, std::reference_wrapper>{}>
                     , class = std::enable_if_t<!std::is_base_of<C, std::remove_reference_t<Td>>{}>>
            static auto get(T && t) noexcept(noexcept(*t)) -> decltype(*t);

            template <class T, class... Args, class MF
                      , class = std::enable_if_t<std::is_function<MF>{}>>
            static auto call(MF C::*pmf, T && obj, Args &&... args)
            noexcept(noexcept((invoke_impl::get(std::forward<T>(obj)).*pmf)(std::forward<Args>(args)...)))
            -> decltype((invoke_impl::get(std::forward<T>(obj)).*pmf)(std::forward<Args>(args)...));

            template <class Fn, class T>
            static auto call(Fn pmv, T && obj)
            noexcept(noexcept(invoke_impl::get(std::forward<T>(obj)).*pmv))
            -> decltype(invoke_impl::get(std::forward<T>(obj)).*pmv);
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
    }
    // namespace detail

    template <class F, class... Args>
    struct invoke_result
     : detail::invoke_result<void, F, Args...>
    {};

    template <class F, class... Args>
    using invoke_result_t = typename invoke_result<F, Args...>::type;

    // invoke
    template <class F, class... Args>
    invoke_result_t<F, Args...>
    invoke(F && fun, Args &&... args) noexcept(saga::is_nothrow_invocable<F, Args...>{})
    {
        using Invoker = detail::invoke_impl<std::decay_t<F>>;
        return Invoker::call(std::forward<F>(fun), std::forward<Args>(args)...);
    }
}
// namespace saga

#endif
// Z_SAGA_TYPE_TRAITS_HPP_INCLUDED
