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
        using value_type = typename Container::value_type;
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

    namespace detail
    {
        // @todo В библиотеку
        struct increment_action
        {
        public:
            template <class Arg>
            Arg & operator()(Arg & arg) const
            {
                ++ arg;
                return arg;
            }
        };

        // @todo В библиотеку
        struct decrement_action
        {
        public:
            template <class Arg>
            Arg & operator()(Arg & arg) const
            {
                -- arg;
                return arg;
            }
        };

        template <class Objective, class Compare, class Argument, class Index, class ObjectiveValue,
                  class FailsCounter, class Action, class UndoAction>
        void LS_probe(Objective objective, Compare cmp, Argument & x_cur, Index pos,
                      ObjectiveValue & y_current, FailsCounter & fails_left,
                      Action action, UndoAction undo_action)
        {
            action(x_cur[pos]);

            auto y_new = objective(x_cur);

            if(cmp(y_new, y_current))
            {
                y_current = y_new;
                fails_left = x_cur.size();
            }
            else
            {
                undo_action(x_cur[pos]);
            }
        }
    }

    template <class SearchSpace, class Objective, class Argument, class Compare = std::less<>>
    auto local_search_integer(SearchSpace const & space, Objective objective, Argument x_init,
                              Compare cmp = Compare())
    -> evaluated_solution<Argument, decltype(objective(x_init))>
    {
        assert(static_cast<std::size_t>(space.dim()) == x_init.size());

        auto const dim = space.dim();

        auto y_current = objective(x_init);

        auto pos = 0*dim;

        // @todo Использовать pos = saga::cursor::make_cycled(saga::cursor::indicies_of(x_init))
        // @todo Выделить класс счётчика: инициализация, достиг конца, сброс?
        for(auto fails_left = dim; fails_left > 0; pos = (pos + 1) % dim)
        {
            -- fails_left;

            if(x_init[pos] != space[pos].max)
            {
                detail::LS_probe(objective, cmp, x_init, pos, y_current, fails_left,
                                 detail::increment_action{}, detail::decrement_action{});
            }

            if(fails_left != dim && x_init[pos] != space[pos].min)
            {
                detail::LS_probe(objective, cmp, x_init, pos, y_current, fails_left,
                                 detail::decrement_action{}, detail::increment_action{});
            }
        }

        return {std::move(x_init), std::move(y_current)};
    }
}
// namespace saga

#endif
// Z_SAGA_OPTIMIZATION_LOCAL_SEARCH_HPP_INCLUDED
