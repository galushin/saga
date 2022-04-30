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

#ifndef Z_SAGA_CURSOR_IOTA_HPP_INCLUDED
#define Z_SAGA_CURSOR_IOTA_HPP_INCLUDED

/** @file saga/cursor/iota.hpp
 @brief Бесконечный курсор в виде последовательности целых чисел, начинающихся с заданного
*/

#include <saga/cursor/cursor_traits.hpp>

#include <iterator>

namespace saga
{
    template <class Incrementable>
    class iota_cursor
    {
    public:
        // Типы
        using cursor_category = std::random_access_iterator_tag;
        using cursor_cardinality = saga::infinite_cursor_cardinality_tag;
        using difference_type = Incrementable;
        using reference = Incrementable const &;
        using value_type = Incrementable;

        // Создание, копирование, уничтожение
        constexpr explicit iota_cursor(Incrementable value)
         : cur_(std::move(value))
        {}

        // Курсор ввода
        constexpr bool operator!() const
        {
            return false;
        }

        constexpr reference front() const
        {
            return this->cur_;
        }

        void drop_front()
        {
            ++ cur_;
        }

        // Курсор произвольного доступа
        constexpr void drop_front(difference_type num)
        {
            this->cur_ += std::move(num);
        }

    private:
        Incrementable cur_;
    };

    namespace cursor
    {
        template <class Incrementable>
        constexpr iota_cursor<Incrementable>
        iota(Incrementable num)
        {
            return iota_cursor<Incrementable>(std::move(num));
        }
    }
    // namespace cursor
}
// namespace saga

#endif
// Z_SAGA_CURSOR_IOTA_HPP_INCLUDED
