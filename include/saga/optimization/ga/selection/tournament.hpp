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

#ifndef Z_SAGA_OPTIMIZATION_GA_SELECTION_TOURNAMENT_HPP_INCLUDED
#define Z_SAGA_OPTIMIZATION_GA_SELECTION_TOURNAMENT_HPP_INCLUDED

/** @file saga/optimization/ga/selection/tournament.hpp
 @brief Реализация турнирной селекции
*/

#include <saga/utility/equality_comparable_box.hpp>

#include <cassert>

#include <algorithm>
#include <memory>
#include <random>

namespace saga
{
    template <class Container, class Compare>
    class selection_tournament_distribution
     : private saga::equality_comparable_box<Compare>
    {
        using compare_base = saga::equality_comparable_box<Compare>;
    public:
        using result_type = typename Container::difference_type;

        /**
        @pre <tt>objective_values.empty() == false</tt>
        @pre <tt>tournament_size > 0 </tt>
        @pre <tt>tournament_size < objective_values.size()</tt>
        */
        explicit selection_tournament_distribution(result_type tournament_size,
                                                   bool repeat,
                                                   Container objective_values,
                                                   Compare cmp)
         : compare_base(std::move(cmp))
         , tournament_(tournament_size)
         , repeat_(repeat)
         , obj_values_(std::move(objective_values))
        {
            assert(!this->obj_values_.empty());
            assert(0 < this->tournament_);
            assert(this->tournament_ <= static_cast<result_type>(this->obj_values_.size()));
        }

        template <class UniformRandomBitGenerator>
        result_type operator()(UniformRandomBitGenerator & rnd) const
        {
            if(this->repeat_)
            {
                return this->selection_repeat(rnd);
            }
            else
            {
                return this->selection_no_repeat(rnd);
            }
        }

        // Свойства
        result_type min() const
        {
            return 0;
        }

        result_type max() const
        {
            return this->obj_values_.size() - 1;
        }

        // Равенство и неравенство
        friend bool operator==(selection_tournament_distribution const & lhs,
                               selection_tournament_distribution const & rhs)
        {
            return lhs.members() == rhs.members();
        }

    private:
        auto members() const
        {
            return std::tie(this->tournament_, this->repeat_, this->obj_values_,
                            static_cast<compare_base const &>(*this));
        }

        Compare const & compare() const
        {
            return compare_base::value();
        }

        bool is_correct_index(result_type index) const
        {
            return 0 <= index && index < static_cast<result_type>(this->obj_values_.size());
        }

        template <class UniformRandomBitGenerator>
        result_type
        selection_repeat(UniformRandomBitGenerator & rnd) const
        {
            std::uniform_int_distribution<result_type> distr(0, this->obj_values_.size() - 1);

            auto best = distr(rnd);

            assert(this->is_correct_index(best));

            for(auto num = this->tournament_ - 1; num > 0; -- num)
            {
                auto cur = distr(rnd);

                assert(this->is_correct_index(cur));

                if(this->compare()(this->obj_values_[cur], this->obj_values_[best]))
                {
                   best = std::move(cur);
                }
            }

            return best;
        }

        template <class UniformRandomBitGenerator>
        result_type
        selection_no_repeat(UniformRandomBitGenerator & rnd) const
        {
            std::vector<result_type> selected;
            selected.reserve(this->tournament_);

            std::uniform_real_distribution<double> distr(0.0, 1.0);
            auto left = this->tournament_;

            for(auto num = this->obj_values_.size(); num > 0; -- num)
            {
                auto p = distr(rnd);

                if(p*num < left)
                {
                    selected.push_back(num - 1);
                    -- left;
                }
            }
            assert(selected.size() == static_cast<std::size_t>(this->tournament_));

            assert(std::all_of(selected.begin(), selected.end(),
                               [&](result_type const & each)
                               { return this->is_correct_index(each); }));

            auto index_cmp = [this](result_type const & x, result_type const & y)
            {
                return this->compare()(this->obj_values_[x], this->obj_values_[y]);
            };

            return *std::min_element(selected.begin(), selected.end(), std::move(index_cmp));
        }

        result_type tournament_ = 2;
        bool repeat_ = true;
        Container obj_values_{};
    };

    class selection_tournament
    {
    public:
        /** @pre <tt>tournament_size > 0</tt>
        */
        explicit selection_tournament(int tournament_size = 2, bool repeat = true)
         : tournament_(tournament_size)
         , repeat_(repeat)
        {
            assert(tournament_size > 0);
        }

        template <class Container, class Compare>
        selection_tournament_distribution<Container, Compare>
        build_distribution(Container const & obj_values, Compare cmp) const
        {
            using Distribution = selection_tournament_distribution<Container, Compare>;
            // @todo Избегать копирования obj_values?
            return Distribution(this->tournament_, this->repeat_, obj_values, std::move(cmp));
        }

    private:
        int tournament_ = 2;
        bool repeat_ = true;
    };
}
// namespace saga

#endif
// Z_SAGA_OPTIMIZATION_GA_SELECTION_TOURNAMENT_HPP_INCLUDED
