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

#ifndef Z_SAGA_TEST_RANDOM_ENGINE_HPP_INCLUDED
#define Z_SAGA_TEST_RANDOM_ENGINE_HPP_INCLUDED

#include <saga/utility/as_const.hpp>
#include <saga/cursor/cursor_traits.hpp>

#include <algorithm>
#include <iterator>
#include <random>

namespace saga_test
{
    using random_engine_type = std::minstd_rand;

    random_engine_type & random_engine();

    template <class IntType1, class IntType2
             , std::enable_if_t<std::is_integral<IntType1>{}, std::nullptr_t> = nullptr
             , std::enable_if_t<std::is_integral<IntType2>{}, std::nullptr_t> = nullptr>
    auto random_uniform(IntType1 lower, IntType2 upper) -> std::common_type_t<IntType1, IntType2>
    {
        using IntType = std::common_type_t<IntType1, IntType2>;

        std::uniform_int_distribution<IntType> distr(std::move(lower), std::move(upper));
        return distr(saga_test::random_engine());
    }

    template <class RealType,
              std::enable_if_t<std::is_floating_point<RealType>{}, std::nullptr_t> = nullptr>
    RealType random_uniform(RealType lower, RealType upper)
    {
        std::uniform_real_distribution<RealType> distr(std::move(lower), std::move(upper));
        return distr(saga_test::random_engine());
    }

    template <class Container>
    auto random_index_of(Container const & container)
    {
        assert(!container.empty());

        auto num = std::distance(container.begin(), container.end());
        return ::saga_test::random_uniform(0, std::move(num) - 1);
    }

    template <class Container>
    auto random_position_of(Container const & container)
    {
        return ::saga_test::random_uniform(0, std::distance(container.begin(), container.end()));
    }

    template <class Container>
    typename Container::iterator
    random_iterator_of(Container & container)
    {
        return std::next(container.begin(), ::saga_test::random_position_of(container));
    }

    template <class Container>
    typename Container::const_iterator
    random_iterator_of(Container const & container)
    {
        return std::next(container.begin(), ::saga_test::random_position_of(container));
    }

    template <class Container>
    void random_iterator_of(Container && container) = delete;

    template <class Container>
    typename Container::const_iterator
    random_const_iterator_of(Container & container)
    {
        return ::saga_test::random_iterator_of(saga::as_const(container));
    }

    template <class Container>
    typename Container::const_iterator
    random_const_iterator_of(Container const & container)
    {
        return ::saga_test::random_iterator_of(container);
    }

    template <class Container>
    void random_const_iterator_of(Container && container) = delete;

    namespace detail
    {
        template <class Cursor>
        Cursor random_subcursor_of(Cursor cur, std::forward_iterator_tag)
        {
            auto const num = saga::cursor::size(cur);

            auto const pos1 = saga_test::random_uniform(0, num);
            auto const pos2 = saga_test::random_uniform(0, num);

            auto after = saga::cursor::drop_front_n(std::move(cur), std::max(pos1, pos2));
            cur = after.dropped_front();
            after.exhaust_back();
            cur.splice(after);

            return saga::cursor::drop_front_n(std::move(cur), std::min(pos1, pos2));
        }

        template <class Cursor>
        Cursor random_subcursor_of(Cursor cur, std::bidirectional_iterator_tag)
        {
            auto const num = saga::cursor::size(cur);

            auto const pos1 = saga_test::random_uniform(0, num);
            auto const pos2 = saga_test::random_uniform(0, num);

            cur = saga::cursor::drop_front_n(std::move(cur), std::min(pos1, pos2));
            return saga::cursor::drop_back_n(std::move(cur), num - std::max(pos1, pos2));
        }
    }
    // namespace detail

    template <class Cursor>
    Cursor random_subcursor_of(Cursor cur)
    {
        cur.forget_front();
        cur.forget_back();

        return detail::random_subcursor_of(std::move(cur), saga::cursor_category_t<Cursor>{});
    }
}
// namespace saga_test

#endif
// Z_SAGA_TEST_RANDOM_ENGINE_HPP_INCLUDED
