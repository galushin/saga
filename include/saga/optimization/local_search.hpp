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

#ifndef Z_SAGA_OPTIMIZATION_LOCAL_SEARCH_HPP_INCLUDED
#define Z_SAGA_OPTIMIZATION_LOCAL_SEARCH_HPP_INCLUDED

/** @file saga/optimization/local_search.hpp
 @brief Методы локального поиска
*/

#include <saga/optimization/evaluated_solution.hpp>

#include <functional>


namespace saga
{
    /** @brief Алгоритм локального поиска псевдо-булевой оптимизации с изменением одной компоненты
    и с переходом по первому улучшению.
    @param objective целевая функция
    @param x_init начальная точка поиска
    @param cmp функция сравнения, используемая для сравнения значений целевой функции
    @return Структуру, содержащую локальный минимум, найденный алгоритмом, запущенным из
    точки @c x_init, и значение целевой функции в этом минимуме
    */
    template <class Objective, class Argument, class Compare = std::less<>>
    auto local_search_boolean(Objective const & objective, Argument x_init,
                              Compare cmp = Compare())
    -> evaluated_solution<Argument, decltype(objective(x_init))>
    {
        using std::begin;
        using std::end;
        auto const first = begin(x_init);
        auto const last = end(x_init);
        auto const dim = x_init.size();

        auto y_current = objective(x_init);
        auto pos = first;
        for(auto fails = 0*dim; fails < dim;)
        {
            *pos = !*pos;
            auto y_new = objective(x_init);

            if(cmp(y_new, y_current))
            {
                fails = 0;
                y_current = y_new;
            }
            else
            {
                ++ fails;
                *pos = !*pos;
            }

            ++ pos;

            if(last == pos)
            {
                pos = first;
            }
        }

        return {std::move(x_init), std::move(y_current)};
    }
}
// namespace saga

#endif
// Z_SAGA_OPTIMIZATION_LOCAL_SEARCH_HPP_INCLUDED
