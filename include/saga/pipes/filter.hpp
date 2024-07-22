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

#ifndef Z_SAGA_PIPES_FILTER_HPP_INCLUDED
#define Z_SAGA_PIPES_FILTER_HPP_INCLUDED

/** @file saga/pipes/filter.hpp
 @brief Умный курсор вывода, отсеивающий элементы, не удовлетворяющие предикату
*/

namespace saga
{
    namespace pipes
    {
        template <class OutputCursor, class UnaryPredicate>
        class filter_output_cursor
        {
        public:
            // Создание, копирование, уничтожение
            constexpr explicit filter_output_cursor(OutputCursor cur, UnaryPredicate pred)
             : data_(std::move(cur), std::move(pred))
            {}

            // Курсор вывода
            constexpr bool operator!() const
            {
                return !std::get<0>(this->data_);
            }

            template <class Arg
                     , class = decltype(std::declval<OutputCursor &>() << std::declval<Arg>())>
            constexpr filter_output_cursor &
            operator<<(Arg && arg)
            {
                if(saga::invoke(std::get<1>(this->data_), arg))
                {
                    std::get<0>(this->data_) << std::forward<Arg>(arg);
                }

                return *this;
            }

            // Адаптер курсора
            OutputCursor base() &&
            {
                return std::get<0>(std::move(this->data_));
            }

        private:
            std::tuple<OutputCursor, UnaryPredicate> data_;
        };

        template <class OutputCursor, class UnaryPredicate>
        constexpr filter_output_cursor<OutputCursor, UnaryPredicate>
        filter(OutputCursor cur, UnaryPredicate pred)
        {
            using Result = filter_output_cursor<OutputCursor, UnaryPredicate>;
            return Result(std::move(cur), std::move(pred));
        }
    }
    // namespace pipes
}
// namespace saga

#endif
// Z_SAGA_PIPES_FILTER_HPP_INCLUDED
