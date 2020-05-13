/* (c) 2019 Галушин Павел Викторович, galushin@gmail.com

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

#include <iterator>
#include <random>

namespace saga_test
{
    using random_engine_type = std::mt19937;

    random_engine_type & random_engine();

    template <class IntType,
              std::enable_if_t<std::is_integral<IntType>{}, std::nullptr_t> = nullptr>
    IntType random_uniform(IntType lower, IntType upper)
    {
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
    auto random_position_of(Container const & container)
    {
        return ::saga_test::random_uniform(0*container.size(), container.size());
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
        return ::saga_test::random_iterator_of(static_cast<Container const &>(container));
    }

    template <class Container>
    typename Container::const_iterator
    random_const_iterator_of(Container const & container)
    {
        return ::saga_test::random_iterator_of(container);
    }

    template <class Container>
    void random_const_iterator_of(Container && container) = delete;
}
// namespace saga_test

#endif
// Z_SAGA_TEST_RANDOM_ENGINE_HPP_INCLUDED
