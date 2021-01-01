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

#ifndef Z_SAGA_OPTIMIATION_OPTIMIZATION_PROBLEM_HPP_INCLUDED
#define Z_SAGA_OPTIMIATION_OPTIMIZATION_PROBLEM_HPP_INCLUDED

/** @file saga/optimization/optimization_problem.hpp
 @brief Шаблоны классов для представления задач оптимизации
*/

#include <functional>

namespace saga
{
    template <class Objective, class Compare = std::less<>>
    struct optimization_problem_boolean
    {
    public:
        Objective objective{};
        int dimension = 0;
        Compare compare{};
    };

    template <class Objective, class Compare = std::less<>>
    auto make_optimization_problem_boolean(Objective objective, int dim, Compare cmp = Compare())
    -> optimization_problem_boolean<Objective, Compare>
    {
        return {std::move(objective), dim, std::move(cmp)};
    }
}
// namespace saga

#endif
// Z_SAGA_OPTIMIATION_OPTIMIZATION_PROBLEM_HPP_INCLUDED
