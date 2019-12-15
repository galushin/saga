#ifndef Z_SAGA_OPTIMIZATION_LOCAL_SEARCH_HPP_INCLUDED
#define Z_SAGA_OPTIMIZATION_LOCAL_SEARCH_HPP_INCLUDED

/** @file saga/optimization/local_search.hpp
 @brief Методы локального поиска
*/

#include <functional>

namespace saga
{
    /** @brief Алгоритм локального поиска псевдо-булевой оптимизации с изменением одной компоненты
    и с переходом по первому улучшению.
    @param objective целевая функция
    @param x_init начальная точка поиска
    @return локальный минимум, найденный алгоритмом, запущенным из точки @c x_init
    @todo Закон "полезного возврата": мы могли бы возвращать также значение целевой функции
    в найденной точке
    */
    template <class Objective, class Argument, class Compare = std::less<>>
    Argument local_search_boolean(Objective const & objective, Argument x_init,
                                  Compare cmp = Compare())
    {
        auto const first = x_init.begin();
        auto const last = x_init.end();
        auto const dim = x_init.size();

        auto y_current = objective(x_init);
        auto pos = x_init.begin();
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

        return x_init;
    }
}
// namespace saga

#endif
// Z_SAGA_OPTIMIZATION_LOCAL_SEARCH_HPP_INCLUDED
