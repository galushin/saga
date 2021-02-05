/* (c) 2020-2021 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef POLYNOMIAL_HPP_INCLUDED
#define POLYNOMIAL_HPP_INCLUDED

/** @file saga/numeric/polynomial.hpp
 @brief Многочлены и алгоритмы для них
*/

#include <saga/detail/static_empty_const.hpp>

#include <iterator>

namespace saga
{
    struct polynomial_horner_fn
    {
    public:
        template <class InputCursor, class T, class Result>
        Result operator()(InputCursor cur, T const & arg, Result const & zero) const
        {
            auto result = zero;

            for(; !!cur; ++ cur)
            {
                result = std::move(result) * arg + *cur;
            }

            return result;
        }
    };

    namespace
    {
        constexpr auto const & polynomial_horner
            = saga::detail::static_empty_const<polynomial_horner_fn>::value;
    }
}
// namespace saga

#endif
// POLYNOMIAL_HPP_INCLUDED
