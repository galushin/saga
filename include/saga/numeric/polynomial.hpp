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

/** @file saga/numeric/polynomail.hpp
 @brief Многочлены и алгоритмы для них
*/

#include <iterator>

namespace saga
{
    /** @todo Возможность задания операций: умножения и сложения
    @todo Более точные ограничения типов
    @todo Возможность не указывать тип Result - выводить из типа значения InputCursor
    */
    template <class Result, class InputCursor, class T>
    Result
    polynomial_horner(InputCursor cur, T const & arg)
    {
        auto result = Result(0);

        for(; !!cur; ++ cur)
        {
            result = result * arg + *cur;
        }

        return result;
    }
}
// namespace saga

#endif
// POLYNOMIAL_HPP_INCLUDED
