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

#ifndef Z_SAGA_PIPES_TRANSFORM_HPP_INCLUDED
#define Z_SAGA_PIPES_TRANSFORM_HPP_INCLUDED

namespace saga
{
    namespace pipes
    {
        template <class OutputCursor, class UnaryFunction>
        class transform_output_cursor
        {
        public:
            // Создание, копирование, уничтожение
            constexpr explicit transform_output_cursor(OutputCursor cur, UnaryFunction fun)
             : cur_(std::move(cur))
             , fun_(std::move(fun))
            {}

            // Курсор вывода
            constexpr bool operator!() const
            {
                return !this->cur_;
            }

            template <class Arg
                     , class = decltype(std::declval<OutputCursor>()
                                        << std::declval<saga::invoke_result_t<UnaryFunction const, Arg>>())>
            constexpr transform_output_cursor &
            operator<<(Arg && arg)
            {
                this->cur_ << saga::invoke(this->fun_, std::forward<Arg>(arg));

                return *this;
            }

        private:
            OutputCursor cur_;
            UnaryFunction fun_;
        };

        template <class OutputCursor, class UnaryFunction>
        constexpr transform_output_cursor<OutputCursor, UnaryFunction>
        transform(OutputCursor cur, UnaryFunction fun)
        {
            using Result = transform_output_cursor<OutputCursor, UnaryFunction>;
            return Result(std::move(cur), std::move(fun));
        }
    }
}
// namespace saga

#endif
// Z_SAGA_PIPES_TRANSFORM_HPP_INCLUDED
