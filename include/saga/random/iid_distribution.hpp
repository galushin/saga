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

#ifndef Z_SAGA_RANDOM_IID_DISTRIBUTION_HPP_INCLUDED
#define Z_SAGA_RANDOM_IID_DISTRIBUTION_HPP_INCLUDED

/** @file saga/random/iid_distribution.hpp
 @brief Многомерное распределение, у которого компоненты распределены независимо и одинаково
*/

#include <saga/algorithm.hpp>
#include <saga/cursor/subrange.hpp>
#include <saga/iterator.hpp>
#include <saga/type_traits.hpp>

#include <algorithm>
#include <vector>

namespace saga
{
    /** @brief Многомерное распределение, у которого компоненты распределены независимо и одинаково
    @tparam Distribution распределение компонент
    @tparam Container тип контейнера для хранения элементов
    */
    template <class Distribution, class Container = std::vector<typename Distribution::result_type>>
    class iid_distribution
    {
    public:
        // Типы
        /// @brief Тип порождаемых значений
        using result_type = Container;

        /// @brief Тип размерности
        using size_type = typename size_type<result_type>::type;

        // Создание, копирование, уничтожение
        /** @brief Конструктор с параметром, задающим размерность порождаемых значений
        @param dim размерность
        @param unit_distribution распределение каждого из компонентов, если не задано пользователем,
        то создаётся с помощью конструктора по умолчанию класса @c Distribution
        @post <tt>this->dim() == dim</tt>
        @post <tt>this->component_distribution() == unit_distribution</tt>
        */
        explicit iid_distribution(size_type dim, Distribution unit_distribution = Distribution{})
         : dim_(std::move(dim))
         , distr_(std::move(unit_distribution))
        {}

        // Порождение
        /** @brief Порождение значений
        @param engine генератор равномерно распределённых бит, используемый для порождения значений
        @return вектор, порождённй в соответствии с данным распределением
        */
        template <class UniformRandomBitGenerator>
        result_type operator()(UniformRandomBitGenerator & engine)
        {
            result_type result(this->dim());

            saga::generate(saga::cursor::all(result), [&]{ return this->distr_(engine); });

            return result;
        }

        // Характеристики
        /** @brief Размерность порождаемых значений
        @return Размерность значений, порождаемых данным распределением
        */
        size_type const & dim() const
        {
            return this->dim_;
        }

        /** @brief Распределение компонент порождаемого значения
        @return Объект, задающий распределение компонент порождаемого значения
        */
        Distribution const & component_distribution() const
        {
            return this->distr_;
        }

    private:
        size_type dim_;
        Distribution distr_;
    };
}
// namespace saga

#endif
// Z_SAGA_RANDOM_IID_DISTRIBUTION_HPP_INCLUDED
