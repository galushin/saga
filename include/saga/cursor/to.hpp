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

        template <class Container, class IndirectReadable>
        using detect_insert_after_indirect
            = decltype(std::declval<Container>().insert_after(std::declval<Container>().before_begin()
                                                              , *std::declval<IndirectReadable>()));
    }

    template <class Container, class InputCursor, class... Args>
    Container to(InputCursor cur, Args &&... args)
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
    {
        using Value = saga::cursor_value_t<InputCursor>;
        using Container = decltype(Container_template(std::declval<Value const *>()
                                                      ,std::declval<Value const *>()
                                                      , std::forward<Args>(args)...));
        return saga::cursor::to<Container>(std::move(cur), std::forward<Args>(args)...);
    }

    namespace detail
    {
        template <class Container>
        struct to_fn
        {};

        template <class InputCursor, class Container>
        Container operator|(InputCursor cur, to_fn<Container>)
        {
            return saga::cursor::to<Container>(std::move(cur));
        }

        template <template <class...> class Container_template>
        struct to_template_fn
        {};

        template <class InputCursor, template <class...> class Container_template>
        auto operator|(InputCursor cur, to_template_fn<Container_template>)
        {
            return saga::cursor::to<Container_template>(std::move(cur));
        }
    }
    // namespace detail

    template <class Container>
    detail::to_fn<Container> to()
    {
        return {};
    }

    template <template <class...> class Container_template>
    detail::to_template_fn<Container_template>
    to()
    {
        return {};
    }
}
// namespace cursor
}
// namespace saga

#endif
// Z_SAGA_CURSOR_TO_HPP_INCLUDED
