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

#ifndef Z_SAGA_OPTIMIZATION_EVALUATED_SOLUTION_HPP_INCLUDED
#define Z_SAGA_OPTIMIZATION_EVALUATED_SOLUTION_HPP_INCLUDED

/** @file optimization/evaluated_solution.hpp
 @brief Структура, объединяющая решение задачи оптимизации и значение целевой функции на нём
*/

namespace saga
{
    template <class Solution, class ObjectiveValue>
    struct evaluated_solution
    {
    public:
        // Типы
        using solution_type = Solution;
        using objective_value_type = ObjectiveValue;

        // Значения
        Solution solution;
        ObjectiveValue objective_value;
    };

    template <class Compare>
    struct comparer_by_objective_value
    {
    public:
        explicit comparer_by_objective_value(Compare comp)
         : cmp(std::move(comp))
        {}

        template <class Solution, class ObjValue>
        bool operator()(evaluated_solution<Solution, ObjValue> const & lhs,
                        evaluated_solution<Solution, ObjValue> const & rhs) const
        {
            return this->cmp(lhs.objective_value, rhs.objective_value);
        }

        Compare cmp;
    };

    template <class Compare>
    auto compare_by_objective_value(Compare cmp)
    {
        return comparer_by_objective_value<Compare>(std::move(cmp));
    }
}
// namespace saga

#endif
// Z_SAGA_OPTIMIZATION_EVALUATED_SOLUTION_HPP_INCLUDED
