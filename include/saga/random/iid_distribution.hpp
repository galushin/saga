#ifndef Z_SAGA_RANDOM_IID_DISTRIBUTION_HPP_INCLUDED
#define Z_SAGA_RANDOM_IID_DISTRIBUTION_HPP_INCLUDED

/** @file saga/random/iid_distribution.hpp
 @brief Многомерное распределение, у которого компоненты распределены независимо и одинаково
*/

#include <saga/type_traits.hpp>

#include <algorithm>
#include <vector>

namespace saga
{
    /** @brief Многомерное распределение, у которого компоненты распределены независимо и одинаково
    @tparam Distribution распределение компонент
    @tparam Container шаблон контейнера для хранения элементов
    */
    template <class Distribution, template <class...> class Container = std::vector>
    class iid_distribution
    {
    public:
        // Типы
        /// @brief Тип порождаемых значений
        using result_type = Container<typename Distribution::result_type>;

        /// @brief Тип размерности
        using size_type = typename size_type<result_type>::type;

        // Создание, копирование, уничтожение
        /** @brief Конструктор с параметром, задающим размерность порождаемых значений
        @param dim размерность
        @post <tt>this->dim() == dim</tt>
        @todo Задокументировать пост-условие распределение компонент
        */
        explicit iid_distribution(size_type dim)
         : dim_(std::move(dim))
         , distr_()
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

            std::generate(std::begin(result), std::end(result),
                          [&]{ return this->distr_(engine); });

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

    private:
        size_type dim_;
        Distribution distr_;
    };
}
// namespace saga

#endif
// Z_SAGA_RANDOM_IID_DISTRIBUTION_HPP_INCLUDED
