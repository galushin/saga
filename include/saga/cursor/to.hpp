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

#ifndef Z_SAGA_CURSOR_TO_HPP_INCLUDED
#define Z_SAGA_CURSOR_TO_HPP_INCLUDED

/** @file saga/cursor/to.hpp
 @brief Средства для преобразования курсоров в контейнер
*/

#include <saga/algorithm.hpp>
#include <saga/container/reserve_if_supported.hpp>

namespace saga
{
namespace cursor
{
    namespace detail
    {
        template <class Container, class InputCursor>
        void may_reserve(Container &, InputCursor const &, std::input_iterator_tag)
        {}

        template <class Container, class InputCursor>
        void may_reserve(Container & container, InputCursor const & cur
                         , std::random_access_iterator_tag)
        {
            saga::reserve_if_supported(container, cur.size());
        }

        template <class Container, class IndirectReadable>
        using detect_push_back_indirect
            = decltype(std::declval<Container>().push_back(*std::declval<IndirectReadable>()));

        template <class Container, class IndirectReadable>
        using detect_insert_indirect
            = decltype(std::declval<Container>().insert(std::declval<Container>().begin()
                                                       ,*std::declval<IndirectReadable>()));

        template <class Cont, class IndirectReadable>
        using detect_insert_after_indirect
            = decltype(std::declval<Cont>().insert_after(std::declval<Cont>().before_begin()
                                                        ,*std::declval<IndirectReadable>()));

        template <class Type, class SFINASE = void>
        struct is_input_cursor
         : std::false_type
        {};

        // @todo Улучшить диагностику
        template <class Type>
        struct is_input_cursor<Type, std::void_t<typename Type::cursor_category>>
         : std::true_type
        {};

        template <template <class...> class Container_template, class InputCursor, class... Args>
        using deduce_container_t
            = decltype(Container_template(std::declval<saga::cursor_value_t<InputCursor> const *>()
                                         ,std::declval<saga::cursor_value_t<InputCursor> const *>()
                                         ,std::declval<Args>()...));
    }

    template <class Container, class InputCursor, class... Args>
    auto to(InputCursor cur, Args &&... args)
    -> std::enable_if_t<detail::is_input_cursor<InputCursor>{}, Container>
    {
        Container result(std::forward<Args>(args)...);

        detail::may_reserve(result, cur, saga::cursor_category_t<InputCursor>{});

        if constexpr(saga::is_detected<detail::detect_push_back_indirect, Container, InputCursor>{})
        {
            saga::copy(std::move(cur), saga::back_inserter(result));
        }
        else if constexpr(saga::is_detected<detail::detect_insert_indirect, Container
                                           , InputCursor>{})
        {
            saga::copy(std::move(cur), saga::inserter(result, result.end()));
        }
        else if constexpr(saga::is_detected<detail::detect_insert_after_indirect, Container
                                           , InputCursor>{})
        {
            auto pos = result.before_begin();

            for(; !!cur; ++ cur)
            {
                pos = result.insert_after(pos, *cur);
            }
        }
        else
        {
            static_assert(sizeof(Container) > 0, "Unsuported container");
        }

        return result;
    }

    template <template <class...> class Container_template, class InputCursor, class... Args>
    auto to(InputCursor cur, Args && ... args)
    -> std::enable_if_t<detail::is_input_cursor<InputCursor>{}
                       ,detail::deduce_container_t<Container_template, InputCursor, Args...>>
    {
        using Container = detail::deduce_container_t<Container_template, InputCursor, Args...>;

        return saga::cursor::to<Container>(std::move(cur), std::forward<Args>(args)...);
    }

    template <class Container, class... Args>
    auto to(Args &&... args)
    {
        auto fun = [t_args = std::forward_as_tuple(std::forward<Args>(args)...)](auto cur)
        {
            auto inner = [&cur](Args &&... inner_args)
            {
                return saga::cursor::to<Container>(std::move(cur)
                                                  , std::forward<Args>(inner_args)...);
            };

            return std::apply(inner, t_args);
        };

        return saga::make_pipeable(std::move(fun));
    }

    template <template <class...> class Container_template, class... Args>
    auto to(Args && ... args)
    {
        auto fun = [t_args = std::forward_as_tuple(std::forward<Args>(args)...)](auto cur)
        {
            auto inner = [&cur](Args &&... inner_args)
            {
                using Container
                    = detail::deduce_container_t<Container_template, decltype(cur), Args...>;

                return saga::cursor::to<Container>(std::move(cur)
                                                  , std::forward<Args>(inner_args)...);
            };

            return std::apply(inner, t_args);
        };

        return saga::make_pipeable(std::move(fun));
    }
}
// namespace cursor
}
// namespace saga

#endif
// Z_SAGA_CURSOR_TO_HPP_INCLUDED
