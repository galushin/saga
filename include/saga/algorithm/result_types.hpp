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

#ifndef Z_SAGA_ALGORITHM_RESULT_TYPES_HPP_INCLUDED
#define Z_SAGA_ALGORITHM_RESULT_TYPES_HPP_INCLUDED

namespace saga
{
    template <class Input, class Function>
    struct in_fun_result
    {
        Input in;
        Function fun;
    };

    template <class Input, class Output>
    struct in_out_result
    {
        Input in;
        Output out;
    };

    template <class Input1, class Input2>
    struct in_in_result
    {
        Input1 in1;
        Input2 in2;
    };

    template <class Input1, class Input2, class Output>
    struct in_in_out_result
    {
        Input1 in1;
        Input2 in2;
        Output out;
    };

    template <class Input, class Output1, class Output2>
    struct in_out_out_result
    {
        Input in;
        Output1 out1;
        Output2 out2;
    };
}
// namespace saga

#endif
// Z_SAGA_ALGORITHM_RESULT_TYPES_HPP_INCLUDED
