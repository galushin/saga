/* (c) 2020 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_ITERATOR_HPP_INCLUDED
#define Z_SAGA_ITERATOR_HPP_INCLUDED

/** @file saga/iterator.hpp
 @brief Функциональность, связанная с итераторома
*/

#include <saga/detail/static_empty_const.hpp>

#include <cassert>
#include <iterator>

namespace saga
{
namespace detail
{
    struct begin_fn
    {
        template <class Range>
        auto operator()(Range && rng) const
        {
            using std::begin;
            return begin(std::forward<Range>(rng));
        }
    };

    struct end_fn
    {
        template <class Range>
        auto operator()(Range && rng) const
        {
            using std::end;
            return end(std::forward<Range>(rng));
        }
    };

    struct size_fn
    {
        template <class SizedRange>
        auto operator()(SizedRange const & sr) const
        -> decltype(sr.size())
        {
            return sr.size();
        }

        template <class T, std::size_t N>
        std::size_t operator()(const T(&arr)[N]) const noexcept
        {
            return N;
        }
    };
}
// namespace detail

    struct front_emplace_fn
    {
    public:
        template <class Container, class Arg>
        void operator()(Container & container, Arg && arg)
        {
            container.emplace_front(std::forward<Arg>(arg));
        }
    };

    struct back_emplace_fn
    {
    public:
        template <class Container, class Arg>
        void operator()(Container & container, Arg && arg)
        {
            container.emplace_back(std::forward<Arg>(arg));
        }
    };

    template <class Iterator>
    struct emplace_fn
    {
    public:
        emplace_fn(Iterator it)
         : position(std::move(it))
        {}

        template <class Container, class Arg>
        void operator()(Container & container, Arg && arg)
        {
            position = container.emplace(this->position, std::forward<Arg>(arg));
            ++ position;
        }

        Iterator position;
    };

    template <class Container, class BackEnd>
    class generic_container_output_iterator
    {
        using Value = typename Container::value_type;
    public:
        // Типы
        using iterator_category = std::output_iterator_tag;
        using value_type = void;
        using difference_type = void;
        using pointer = void;
        using reference = void;
        using container_type = Container;

        // Создание, копирование, уничтожение
        generic_container_output_iterator() = default;

        template <class... Args>
        explicit generic_container_output_iterator(Container & container, Args &&... args)
         : back_end_(std::forward<Args>(args)...)
         , container_(std::addressof(container))
        {}

        template <class Arg,
                  std::enable_if_t<std::is_constructible<Value, Arg>{}, std::nullptr_t> = nullptr>
        generic_container_output_iterator & operator=(Arg && arg)
        {
            assert(this->container() != nullptr);
            this->back_end_(*this->container(), std::forward<Arg>(arg));
            return *this;
        }

        // Итератор
        generic_container_output_iterator & operator++()
        {
            return *this;
        }

        generic_container_output_iterator & operator*()
        {
            return *this;
        }

        // Свойства
        Container * container() const
        {
            return this->container_;
        }

    private:
        // @todo оптмизация пустого возвращаемого значения
        BackEnd back_end_ {};
        Container * container_ = nullptr;
    };

    template <class Container>
    using back_emplace_iterator = generic_container_output_iterator<Container, back_emplace_fn>;

    template <class Container>
    using front_emplace_iterator = generic_container_output_iterator<Container, front_emplace_fn>;

    template <class Container>
    using emplace_iterator
        = generic_container_output_iterator<Container, emplace_fn<typename Container::iterator>>;

    template <class Container>
    auto back_emplacer(Container & container)
    {
        return back_emplace_iterator<Container>(container);
    }

    template <class Container>
    auto front_emplacer(Container & container)
    {
        return front_emplace_iterator<Container>(container);
    }

    template <class Container>
    auto emplacer(Container & container, typename Container::iterator pos)
    {
        return emplace_iterator<Container>(container, std::move(pos));
    }

    namespace
    {
        constexpr auto const & begin = detail::static_empty_const<detail::begin_fn>::value;
        constexpr auto const & end = detail::static_empty_const<detail::end_fn>::value;

        constexpr auto const & size = detail::static_empty_const<detail::size_fn>::value;
    }
}
// namespace saga

#endif
// Z_SAGA_ITERATOR_HPP_INCLUDED
