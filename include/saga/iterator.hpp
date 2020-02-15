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
}
// namespace detail

    namespace
    {
        constexpr auto const & begin = detail::static_empty_const<detail::begin_fn>::value;
        constexpr auto const & end = detail::static_empty_const<detail::end_fn>::value;
    }
}
// namespace saga

#endif
// Z_SAGA_ITERATOR_HPP_INCLUDED
