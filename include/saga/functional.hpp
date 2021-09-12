/* (c) 2021 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_FUNCTIONAL_HPP_INCLUDED
#define Z_SAGA_FUNCTIONAL_HPP_INCLUDED

/** @file saga/functional.hpp
 @brief Функциональные объекты
*/

#include <saga/type_traits.hpp>

#include <utility>
#include <type_traits>

namespace saga
{
    template <class Arg = void>
    struct increment
    {
        constexpr Arg & operator()(Arg & arg) const
        {
            ++ arg;
            return arg;
        }
    };

    template <>
    struct increment<void>
    {
        template <class Arg>
        constexpr Arg & operator()(Arg & arg) const
        {
            ++ arg;
            return arg;
        }
    };

    template <class Arg = void>
    struct decrement
    {
        constexpr Arg & operator()(Arg & arg) const
        {
            -- arg;
            return arg;
        }
    };

    template <>
    struct decrement<void>
    {
        template <class Arg>
        constexpr Arg & operator()(Arg & arg) const
        {
            -- arg;
            return arg;
        }
    };

    // invoke
    template <class F, class... Args>
    constexpr invoke_result_t<F, Args...>
    invoke(F && fun, Args &&... args) noexcept(saga::is_nothrow_invocable<F, Args...>{})
    {
        using Invoker = detail::invoke_impl<std::decay_t<F>>;
        return Invoker::call(std::forward<F>(fun), std::forward<Args>(args)...);
    }

    // not_fn
    namespace detail
    {
        template <class F>
        struct not_fn_t
        {
            F fun_;

            template <class... Args>
            auto operator()(Args&&... args) &
            noexcept(noexcept(!saga::invoke(fun_, std::forward<Args>(args)...)))
            -> decltype(!saga::invoke(fun_, std::forward<Args>(args)...))
            {
                return !saga::invoke(fun_, std::forward<Args>(args)...);
            }
        };
    }
    // namespace detail

    template <class F>
    auto not_fn(F && f) -> detail::not_fn_t<std::decay_t<F>>
    {
        return { std::forward<F>(f)};
    }
}

#endif
// Z_SAGA_FUNCTIONAL_HPP_INCLUDED
