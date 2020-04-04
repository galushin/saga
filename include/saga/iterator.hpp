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

#include <iterator>
#include <saga/detail/static_empty_const.hpp>

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

    template <class Container>
    class back_emplace_iterator
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
        back_emplace_iterator() = default;

        explicit back_emplace_iterator(Container & container)
         : container_(std::addressof(container))
        {}

        template <class Arg,
                  std::enable_if_t<std::is_constructible<Value, Arg>{}, std::nullptr_t> = nullptr>
        back_emplace_iterator & operator=(Arg && arg)
        {
            this->container().emplace_back(std::forward<Arg>(arg));
            return *this;
        }

        // Итератор
        back_emplace_iterator & operator++()
        {
            return *this;
        }

        back_emplace_iterator & operator*()
        {
            return *this;
        }

        // Свойства
        Container & container() const
        {
            return *this->container_;
        }

    private:
        Container * container_ = nullptr;
    };

    template <class Container>
    auto back_emplacer(Container & container)
    {
        return back_emplace_iterator<Container>(container);
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
