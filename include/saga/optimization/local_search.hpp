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

#ifndef Z_SAGA_OPTIMIZATION_LOCAL_SEARCH_HPP_INCLUDED
#define Z_SAGA_OPTIMIZATION_LOCAL_SEARCH_HPP_INCLUDED

/** @file saga/optimization/local_search.hpp
 @brief Методы локального поиска
*/

#include <saga/cursor/cycle.hpp>
#include <saga/cursor/subrange.hpp>
#include <saga/optimization/evaluated_solution.hpp>

#include <functional>
#include <vector>

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
        auto pos = saga::cursor::make_cycled(saga::cursor::all(x_init));

        auto const dim = x_init.size();

        auto y_current = objective(x_init);

        for(auto fails = dim; fails > 0; ++ pos)
        {
            *pos = !*pos;
            auto y_new = objective(x_init);

            if(cmp(y_new, y_current))
            {
                fails = dim;
                y_current = y_new;
            }
            else
            {
                -- fails;
                *pos = !*pos;
            }
        }

        return {std::move(x_init), std::move(y_current)};
    }

    template <class Integer>
    class search_space_integer
    {
        struct integer_variable_limits
        {
            integer_variable_limits(Integer min, Integer max)
             : min(std::move(min))
             , max(std::move(max))
            {
                assert(min <= max);
            }

            Integer min;
            Integer max;
        };

        using Container = std::vector<integer_variable_limits>;

    public:
        using const_iterator = typename Container::const_iterator;
        using index_type = typename Container::difference_type;

        void add(Integer min, Integer max)
        {
            assert(min <= max);

            this->variables_.emplace_back(min, max);
        }

        const_iterator begin() const
        {
            return this->variables_.begin();
        }

        const_iterator end() const
        {
            return this->variables_.end();
        }

        index_type dim() const
        {
            return this->variables_.size();
        }

        integer_variable_limits const & operator[](index_type index) const
        {
            return this->variables_[index];
        }

    private:
        Container variables_;
    };

    /** @todo Унифицировать с булевым
    @todo Использовать циклический индекс?
    @todo Функция должна помещаться на экран
    */
    template <class SearchSpace, class Objective, class Argument>
    auto local_search_integer(SearchSpace const & space, Objective objective, Argument x_init)
    -> evaluated_solution<Argument, decltype(objective(x_init))>
    {
        auto const dim = space.dim();

        auto y_current = objective(x_init);

        auto pos = 0*dim;

        for(auto fails_left = dim; fails_left > 0; pos = (pos + 1) % dim)
        {
            -- fails_left;

            if(x_init[pos] < space[pos].max)
            {
                x_init[pos] += 1;

                auto y_new = objective(x_init);

                if(y_new < y_current)
                {
                    y_current = y_new;
                    fails_left = dim;
                }
                else
                {
                    x_init[pos] -= 1;
                }
            }

            if(fails_left != dim && x_init[pos] > space[pos].min)
            {
                x_init[pos] -= 1;

                auto y_new = objective(x_init);

                if(y_new < y_current)
                {
                    y_current = y_new;
                    fails_left = dim;
                }
                else
                {
                    x_init[pos] += 1;
                }
            }
        }

        return {std::move(x_init), std::move(y_current)};
    }
}
// namespace saga

#endif
// Z_SAGA_OPTIMIZATION_LOCAL_SEARCH_HPP_INCLUDED
