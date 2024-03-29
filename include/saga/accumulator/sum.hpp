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

#ifndef Z_SAGA_ACCUMULATOR_SUM_HPP_INCLUDED
#define Z_SAGA_ACCUMULATOR_SUM_HPP_INCLUDED

/** @file saga/accumulator/sum.hpp
 @brief Функциональный объект-накопитель для вычисления суммы
*/

namespace saga
{
    template <class T>
    class sum_accumulator
    {
    public:
        sum_accumulator & operator()(T const & x)
        {
            this->sum_ += x;
            return *this;
        }

        T const & sum() const
        {
            return this->sum_;
        }

    private:
        T sum_{0};
    };
}
// namespace saga

#endif
// Z_SAGA_ACCUMULATOR_SUM_HPP_INCLUDED
