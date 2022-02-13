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

#ifndef Z_SAGA_ITERATOR_OSTREAM_JOINER_HPP_INCLUDED
#define Z_SAGA_ITERATOR_OSTREAM_JOINER_HPP_INCLUDED

/** @file saga/iterator/ostream_joiner.hpp
 @brief Курсор (и итератор) вывода, записывающий значения в поток вывода и разделяющих их заданным
 значением-разделителем.
*/

#include <cassert>
#include <iterator>

namespace saga
{
    template <class Delim, class OStream>
    class ostream_joiner
    {
    public:
        // Типы
        using char_type = typename OStream::char_type;
        using traits_type = typename OStream::traits_type;
        using ostream_type = OStream;
        using iterator_category = std::output_iterator_tag;
        using value_type = void;
        using difference_type = std::ptrdiff_t;
        using pointer = void;
        using reference = void;

        // Создание, копирование, уничтожение
        ostream_joiner(OStream & os, Delim const & delim)
         : os_(std::addressof(os))
         , delim_(delim)
        {}

        ostream_joiner(OStream & os, Delim && delim);

        // Итератор вывода
        ostream_joiner & operator*() noexcept
        {
            return *this;
        }

        ostream_joiner & operator++() noexcept
        {
            return *this;
        }

        template <class Arg>
        ostream_joiner &
        operator=(Arg const & arg)
        {
            assert(this->os_ != nullptr);

            if(!first_element)
            {
                *this->os_ << this->delim_;
            }

            first_element = false;
            *this->os_ << arg;

            return *this;
        }

        [[nodiscard]] ostream_joiner & operator++(int) noexcept
        {
            return *this;
        }

        // Курсор вывода
        bool operator!() const noexcept
        {
            return this->os_ == nullptr || !*this->os_;
        }

        template <class Arg
                 , class = decltype(std::declval<OStream&>() << std::declval<Arg const&>())>
        ostream_joiner &
        operator<<(Arg const & arg)
        {
            return *this = arg;
        }

    private:
        OStream * os_ = nullptr;
        Delim delim_;
        bool first_element = true;
    };

    template <class OStream, class Delim>
    ostream_joiner(OStream &, Delim) -> ostream_joiner<Delim, OStream>;
}
// namespace saga

#endif
// Z_SAGA_ITERATOR_OSTREAM_JOINER_HPP_INCLUDED
