/* (c) 2020-2022 Галушин Павел Викторович, galushin@gmail.com

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

#include <saga/iterator/ostream_joiner.hpp>
#include <saga/type_traits.hpp>

#include <cassert>
#include <iterator>

namespace saga
{
    namespace detail
    {
        template <class T, bool to_object>
        struct incrementable_traits_pointer
        {};

        template <class T>
        struct incrementable_traits_pointer<T, true>
        {
            using difference_type = std::ptrdiff_t;
        };

        template <class T, class SFINAE = void>
        struct has_difference_type
         : std::false_type
        {};

        template <class T>
        struct has_difference_type<T, std::void_t<typename T::difference_type>>
         : std::true_type
        {};

        template <class T, class SFINAE = void>
        struct incrementable_traits_not_pointer
        {};

        template <class T>
        struct incrementable_traits_not_pointer<T, std::enable_if_t<has_difference_type<T>{}>>
        {
            using difference_type = typename T::difference_type;
        };

        template <class T, class SFINAE = void>
        struct has_integral_difference
         : std::false_type
        {};

        template <class T>
        struct has_integral_difference<T, std::void_t<decltype(std::declval<T const&>() - std::declval<T const&>())>>
         : std::is_integral<decltype(std::declval<T const&>() - std::declval<T const&>())>
        {};

        template <class T>
        struct incrementable_traits_not_pointer<T, std::enable_if_t<!has_difference_type<T>{} && has_integral_difference<T>{}>>
        {
            using difference_type = std::make_signed_t<decltype(std::declval<T const&>() - std::declval<T const&>())>;
        };
    }

    template <class T>
    struct incrementable_traits
     : detail::incrementable_traits_not_pointer<T, void>
    {};

    template <class T>
    struct incrementable_traits<T*>
     : detail::incrementable_traits_pointer<T*, std::is_object<T>{}>
    {};

    template <class T>
    struct incrementable_traits<T const>
     : incrementable_traits<T>
    {};

    template <class T>
    using incrementable_difference_t = typename incrementable_traits<T>::difference_type;

namespace detail
{
    void begin() = delete;
    void end() = delete;
    void rbegin() = delete;
    void rend() = delete;

    struct begin_fn
    {
        template <class Range>
        constexpr auto operator()(Range && rng) const
        {
            using std::begin;
            return begin(std::forward<Range>(rng));
        }
    };

    struct end_fn
    {
        template <class Range>
        constexpr auto operator()(Range && rng) const
        {
            using std::end;
            return end(std::forward<Range>(rng));
        }
    };

    struct rbegin_fn
    {
        template <class Range>
        auto operator()(Range && rng) const
        {
            using std::rbegin;
            return rbegin(std::forward<Range>(rng));
        }
    };

    struct rend_fn
    {
        template <class Range>
        auto operator()(Range && rng) const
        {
            using std::rend;
            return rend(std::forward<Range>(rng));
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
        std::size_t operator()(const T(&)[N]) const noexcept
        {
            return N;
        }
    };
}
// namespace detail

    struct back_insert_fn
    {
    public:
        template <class Container>
        void operator()(Container & container, typename Container::value_type const & value) const
        {
            container.push_back(value);
        }

        template <class Container>
        void operator()(Container & container, typename Container::value_type && value) const
        {
            container.push_back(std::move(value));
        }
    };

    struct front_insert_fn
    {
        template <class Container>
        void operator()(Container & container, typename Container::value_type const & value) const
        {
            container.push_front(value);
        }

        template <class Container>
        void operator()(Container & container, typename Container::value_type && value) const
        {
            container.push_front(std::move(value));
        }
    };

    template <class Iterator>
    struct insert_fn
    {
    public:
        explicit insert_fn(Iterator it)
         : position(std::move(it))
        {}

        template <class Container>
        void operator()(Container & container, typename Container::value_type const & value)
        {
            position = container.insert(this->position, value);
            ++ position;
        }

        template <class Container>
        void operator()(Container & container, typename Container::value_type && value)
        {
            position = container.insert(this->position, std::move(value));
            ++ position;
        }

        Iterator position;
    };

    struct front_emplace_fn
    {
    public:
        template <class Container, class Arg>
        void operator()(Container & container, Arg && arg) const
        {
            container.emplace_front(std::forward<Arg>(arg));
        }
    };

    struct back_emplace_fn
    {
    public:
        template <class Container, class Arg>
        void operator()(Container & container, Arg && arg) const
        {
            container.emplace_back(std::forward<Arg>(arg));
        }
    };

    template <class Iterator>
    struct emplace_fn
    {
    public:
        explicit emplace_fn(Iterator it)
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
     : private BackEnd
    {
        using Value = typename Container::value_type;
    public:
        // Типы
        using iterator_category = std::output_iterator_tag;
        using cursor_category = iterator_category;
        using value_type = void;
        using difference_type = void;
        using pointer = void;
        using reference = void;
        using container_type = Container;

        // Создание, копирование, уничтожение
        generic_container_output_iterator() = delete;

        template <class... Args>
        explicit generic_container_output_iterator(Container & container, Args &&... args)
         : BackEnd(std::forward<Args>(args)...)
         , container_(std::addressof(container))
        {}

        template <class Arg,
                  std::enable_if_t<std::is_constructible<Value, Arg>{}, std::nullptr_t> = nullptr>
        generic_container_output_iterator & operator=(Arg && arg)
        {
            assert(this->container() != nullptr);
            this->back_end()(*this->container(), std::forward<Arg>(arg));
            return *this;
        }

        // Итератор и курсор вывода
        constexpr bool operator!() const
        {
            return false;
        }

        generic_container_output_iterator & operator++()
        {
            return *this;
        }

        generic_container_output_iterator & operator*()
        {
            return *this;
        }

        template <class Arg,
                  std::enable_if_t<std::is_constructible<Value, Arg>{}, std::nullptr_t> = nullptr>
        generic_container_output_iterator & operator<<(Arg && arg)
        {
            *this = std::forward<Arg>(arg);
            ++*this;

            return *this;
        }

        // Свойства
        Container * container() const
        {
            return this->container_;
        }

    private:
        BackEnd & back_end()
        {
            return *this;
        }

        Container * container_ = nullptr;
    };

    template <class Container>
    using back_insert_iterator = generic_container_output_iterator<Container, back_insert_fn>;

    template <class Container>
    using front_insert_iterator = generic_container_output_iterator<Container, front_insert_fn>;

    template <class Container>
    using insert_iterator
        = generic_container_output_iterator<Container
                                           , insert_fn<typename Container::const_iterator>>;

    template <class Container>
    using back_emplace_iterator = generic_container_output_iterator<Container, back_emplace_fn>;

    template <class Container>
    using front_emplace_iterator = generic_container_output_iterator<Container, front_emplace_fn>;

    template <class Container>
    using emplace_iterator
        = generic_container_output_iterator<Container
                                           , emplace_fn<typename Container::const_iterator>>;

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
    auto emplacer(Container & container, typename Container::const_iterator pos)
    {
        return emplace_iterator<Container>(container, std::move(pos));
    }

    template <class Container>
    auto back_inserter(Container & container)
    {
        return back_insert_iterator<Container>(container);
    }

    template <class Container>
    auto front_inserter(Container & container)
    {
        return front_insert_iterator<Container>(container);
    }

    template <class Container>
    auto inserter(Container & container, typename Container::const_iterator pos)
    {
        return insert_iterator<Container>(container, std::move(pos));
    }

    inline constexpr auto const begin = detail::begin_fn{};
    inline constexpr auto const end = detail::end_fn{};
    inline constexpr auto const rbegin = detail::rbegin_fn{};
    inline constexpr auto const rend = detail::rend_fn{};
    inline constexpr auto const size = detail::size_fn{};
}
// namespace saga

#endif
// Z_SAGA_ITERATOR_HPP_INCLUDED
