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

#include <saga/algorithm.hpp>
#include <saga/container/reserve_if_supported.hpp>

namespace saga
{
namespace cursor
{
    template <class InputCursor>
    class cursor_iterator
    {
    public:
        // Типы
        using iterator_category = saga::cursor_category_t<InputCursor>;
        using value_type = saga::cursor_value_t<InputCursor>;
        using difference_type = saga::cursor_difference_t<InputCursor>;
        using pointer = typename InputCursor::pointer;
        using reference = saga::cursor_reference_t<InputCursor>;

        // Конструкторы
        cursor_iterator() = default;

        explicit cursor_iterator(InputCursor cur)
         : maybe_cur_(std::move(cur))
        {}

        // Итератор ввода
        cursor_iterator & operator++()
        {
            assert(this->maybe_cur_.has_value());

            this->maybe_cur_.value().drop_front();

            return *this;
        }

        reference operator*() const
        {
            assert(this->maybe_cur_.has_value());

            return this->maybe_cur_.value().front();
        }

        bool operator==(cursor_iterator const & rhs) const &
        {
            if(!this->maybe_cur_.has_value())
            {
                return !rhs.maybe_cur_.has_value()
                        || !rhs.maybe_cur_.value();
            }
            else if(!rhs.maybe_cur_.has_value())
            {
                return !this->maybe_cur_.has_value()
                        || !this->maybe_cur_.value();
            }
            else
            {
                return this->maybe_cur_.value() == rhs.maybe_cur_.value();
            }
        }

        // Итератор произвольного доступа
        difference_type operator-(cursor_iterator const & rhs) const &
        {
            assert(!this->maybe_cur_.has_value());
            assert(rhs.maybe_cur_.has_value());

            return rhs.maybe_cur_.value().size();
        }

    private:
        std::optional<InputCursor> maybe_cur_;
    };

    template <class InputCursor>
    bool operator!=(cursor_iterator<InputCursor> const & lhs
                    , cursor_iterator<InputCursor> const & rhs)
    {
        return !(lhs == rhs);
    }

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

        template <class Container, class InputCursor
                 , class = decltype(std::declval<Container>().push_back(*std::declval<InputCursor>()))>
        Container to_impl(InputCursor cur, saga::priority_tag<0>)
        {
            Container result;

            detail::may_reserve(result, cur, saga::cursor_category_t<InputCursor>{});

            saga::copy(std::move(cur), saga::back_inserter(result));

            return result;
        }

        template <class Container, class InputCursor
                 , class = decltype(std::declval<Container>().insert_after(std::declval<Container>().before_begin()
                                                                      , *std::declval<InputCursor>()))>
        Container to_impl(InputCursor cur, saga::priority_tag<1>)
        {
            Container result;

            auto pos = result.before_begin();

            for(; !!cur; ++ cur)
            {
                pos = result.insert_after(pos, *cur);
            }

            return result;
        }
    }

    template <class Container, class InputCursor>
    Container to(InputCursor cur)
    {
        return detail::to_impl<Container>(std::move(cur), saga::priority_tag<1>{});
    }

    template <template <class...> class Container_template, class InputCursor>
    Container_template<saga::cursor_value_t<InputCursor>>
    to(InputCursor cur)
    {
        using Container = Container_template<saga::cursor_value_t<InputCursor>>;
        return saga::cursor::to<Container>(std::move(cur));
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
        Container_template<saga::cursor_value_t<InputCursor>>
        operator|(InputCursor cur, to_template_fn<Container_template>)
        {
            using Container = Container_template<saga::cursor_value_t<InputCursor>>;

            return saga::cursor::to<Container>(std::move(cur));
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
