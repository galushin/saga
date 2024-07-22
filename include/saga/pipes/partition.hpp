/* (c) 2022 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_PIPES_PARTITION_HPP_INCLUDED
#define Z_SAGA_PIPES_PARTITION_HPP_INCLUDED

/** @brief saga/pipes/partition.hpp
 @brief Выходной курсор, разделяющий элементы в два выходных курсора, в зависимости от истинности
 заданного предиката
*/

#include <saga/functional.hpp>

#include <cassert>
#include <utility>

namespace saga
{
    namespace pipes
    {
        template <class OutputCursor1, class OutputCursor2, class UnaryPredicate>
        class partition_pipe
        {
        public:
            // Конструктор
            explicit
            partition_pipe(OutputCursor1 out_true, OutputCursor2 out_false, UnaryPredicate pred)
             : data_(std::move(out_true), std::move(out_false), std::move(pred))
            {}

            // Курсор вывода
            bool operator!() const
            {
                return !std::get<0>(this->data_) || !std::get<1>(this->data_);
            }

            template <class Arg
                     , class = decltype(std::declval<OutputCursor1&>() << std::declval<Arg>())
                     , class = decltype(std::declval<OutputCursor2&>() << std::declval<Arg>())>
            partition_pipe &
            operator<<(Arg&& arg)
            {
                assert(!!*this);

                if(saga::invoke(std::get<2>(this->data_), std::forward<Arg>(arg)))
                {
                    std::get<0>(this->data_) << std::forward<Arg>(arg);
                }
                else
                {
                    std::get<1>(this->data_) << std::forward<Arg>(arg);
                }

                return *this;
            }

            // Адаптер курсора
            OutputCursor1 output_true() &&
            {
                return std::move(std::get<0>(this->data_));
            }

            OutputCursor2 output_false() &&
            {
                return std::move(std::get<1>(this->data_));
            }

        private:
            std::tuple<OutputCursor1, OutputCursor2, UnaryPredicate> data_;
        };

        template <class OutputCursor1, class OutputCursor2, class UnaryPredicate>
        partition_pipe<OutputCursor1, OutputCursor2, UnaryPredicate>
        partition(OutputCursor1 out_true, OutputCursor2 out_false, UnaryPredicate pred)
        {
            using Result = partition_pipe<OutputCursor1, OutputCursor2, UnaryPredicate>;

            return Result(std::move(out_true), std::move(out_false), std::move(pred));
        }
    }
    // namespace pipes
}
//namespace saga

#endif
// Z_SAGA_PIPES_PARTITION_HPP_INCLUDED
