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

#ifndef Z_SAGA_OPTIMIZATION_GA_SELECTION_ANY_SELECTION_HPP_INCLUDED
#define Z_SAGA_OPTIMIZATION_GA_SELECTION_ANY_SELECTION_HPP_INCLUDED

/** @file saga/optimization/ga/selection/any_selection.hpp
 @brief Реализация селекции со стёртым типом
*/

#include <saga/cpp20/span.hpp>
#include <saga/random/any_distribution.hpp>

#include <functional>

namespace saga
{
    template <class UniformRandomBitGenerator>
    class any_selection
    {
    public:
        // Типы
        using objective_value_type = double;
        using argument_type = saga::span<objective_value_type const>;
        using compare = std::function<bool(objective_value_type const &, objective_value_type const &)>;

        using distribution_type = saga::any_distribution<typename argument_type::difference_type,
                                                         UniformRandomBitGenerator>;

        // Создание, копирование, уничтожение
        any_selection() = default;

        template <class Selection>
        explicit any_selection(Selection selection)
         : backend_([selection](argument_type obj_values, compare const & cmp)
                    { return distribution_type(selection.build_distribution(obj_values, cmp)); })
        {}

        // Селекция
        distribution_type build_distribution(argument_type obj_values, compare const & cmp) const
        {
            return this->backend_(obj_values, cmp);
        }

    private:
        std::function<distribution_type(argument_type, compare const &)> backend_;
    };
}
// namespace saga

#endif
// Z_SAGA_OPTIMIZATION_GA_SELECTION_ANY_SELECTION_HPP_INCLUDED
