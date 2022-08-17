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

#ifndef Z_SAGA_ALGORITHM_COPY_HPP_INCLUDED
#define Z_SAGA_ALGORITHM_COPY_HPP_INCLUDED

/** @file saga/algorithm/copy.hpp
 @brief Алгоритм копирования
*/

#include <saga/algorithm/result_types.hpp>

#include <utility>

namespace saga
{
    struct copy_fn
    {
        template <class InputCursor, class OutputCursor>
        constexpr
        in_out_result<InputCursor, OutputCursor>
        operator()(InputCursor in, OutputCursor out) const
        {
            for(; !!in && !!out; ++in)
            {
                out << *in;
            }

            return {std::move(in), std::move(out)};
        }
    };

    inline constexpr auto const copy = copy_fn{};
}
// namespace saga

#endif
// Z_SAGA_ALGORITHM_COPY_HPP_INCLUDED
