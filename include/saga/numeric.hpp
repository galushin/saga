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

#ifndef Z_SAGA_NUMERIC_HPP_INCLUDED
#define Z_SAGA_NUMERIC_HPP_INCLUDED

namespace saga
{
    class inner_product_fn
    {
    public:
        template <class InputCursor1, class InputCursor2, class T,
                  class BinaryOperator1, class BinaryOperator2>
        T operator()(InputCursor1 in1, InputCursor2 in2, T init_value,
                     BinaryOperator1 op1, BinaryOperator2 op2) const
        {
            for(; !!in1 && !!in2; ++ in1, (void)++in2)
            {
                init_value = op1(std::move(init_value), op2(*in1, *in2));
            }

            return init_value;
        }
    };

namespace
{
    constexpr auto const inner_product
        = saga::detail::static_empty_const<saga::inner_product_fn>::value;
}

}
// namespace saga

#endif
// Z_SAGA_NUMERIC_HPP_INCLUDED
