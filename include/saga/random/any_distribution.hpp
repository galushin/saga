/* (c) 2020 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_RANDOM_ANY_DISTRIBUTION_HPP_INCLUDED
#define Z_SAGA_RANDOM_ANY_DISTRIBUTION_HPP_INCLUDED

/** @file saga/random/any_distribution.hpp
 @brief Распределение вероятностей со стёртым типом
*/

#include <saga/utility/operators.hpp>
#include <saga/utility/copy_and_swap.hpp>

#include <memory>

namespace saga
{
    template <class Result, class UniformRandomBitGenerator>
    class any_distribution
     : saga::operators::equality_comparable<any_distribution<Result, UniformRandomBitGenerator>>
    {
        friend bool operator==(any_distribution const & lhs, any_distribution const & rhs)
        {
            return lhs.pimpl_->is_equal(*rhs.pimpl_);
        }

    public:
        // Типы
        using result_type = Result;
        using engine_type = UniformRandomBitGenerator;

        // Создание, копирование, уничтожение
        template <class Distribution>
        explicit any_distribution(Distribution distr)
         : pimpl_(std::make_unique<distribution_impl<Distribution>>(std::move(distr)))
        {}

        any_distribution(any_distribution const & other)
         : pimpl_(other.pimpl_->clone())
        {}

        any_distribution(any_distribution &&) = default;

        any_distribution & operator=(any_distribution const & rhs)
        {
            return saga::copy_and_swap(*this, rhs);
        }

        any_distribution & operator=(any_distribution &&) = default;

        void swap(any_distribution & other)
        {
            this->pimpl_.swap(other.pimpl_);
        }

        // Порождение значений
        result_type operator()(UniformRandomBitGenerator & urbg)
        {
            return this->pimpl_->generate(urbg);
        }

        // Свойства
        result_type min() const
        {
            return this->pimpl_->min();
        }

        result_type max() const
        {
            return this->pimpl_->max();
        }

    private:
        class distribution_interface
        {
        public:
            virtual ~distribution_interface() = default;

            virtual std::unique_ptr<distribution_interface> clone() const = 0;

            virtual result_type generate(engine_type & urbg) = 0;

            virtual result_type min() const = 0;
            virtual result_type max() const = 0;

            virtual bool is_equal(distribution_interface const &) const = 0;
        };

        std::unique_ptr<distribution_interface> pimpl_;

        template <class Distribution>
        class distribution_impl
         : public distribution_interface
        {
        public:
            explicit distribution_impl(Distribution distr)
             : distr_(std::move(distr))
            {}

            std::unique_ptr<distribution_interface> clone() const override
            {
                return std::make_unique<distribution_impl>(*this);
            }

            result_type generate(engine_type & urbg) override
            {
                return this->distr_(urbg);
            }

            result_type min() const override
            {
                return this->distr_.min();
            }

            result_type max() const override
            {
                return this->distr_.max();
            }

            bool is_equal(distribution_interface const & rhs) const override
            {
                if(auto * p_rhs = dynamic_cast<distribution_impl const *>(std::addressof(rhs)))
                {
                    return this->distr_ == p_rhs->distr_;
                }
                else
                {
                    return false;
                }
            }

        private:
            Distribution distr_;
        };
    };
}
// namespace saga

#endif
// Z_SAGA_RANDOM_ANY_DISTRIBUTION_HPP_INCLUDED
