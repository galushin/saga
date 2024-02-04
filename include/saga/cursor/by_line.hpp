/* (c) 2024 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_CURSOR_BY_LINE_HPP_INCLUDED
#define Z_SAGA_CURSOR_BY_LINE_HPP_INCLUDED

/** @file saga/cursor/by_line.hpp
 @brief Курсор для построчного ввода из потока
*/

#include <saga/cursor/cursor_facade.hpp>

#include <cassert>
#include <string>

namespace saga
{
    template <class IStream, class Difference = std::ptrdiff_t>
    class by_line_cursor
     : saga::cursor_facade<by_line_cursor<IStream>
                          , std::basic_string<typename IStream::char_type
                                             ,typename IStream::traits_type> const &>
    {
    public:
        // Типы
        using char_type = typename IStream::char_type;

        using cursor_category = std::input_iterator_tag;
        using difference_type = Difference;
        using value_type = std::basic_string<char_type, typename IStream::traits_type>;
        using reference = value_type const &;

        // Создание, копирование, уничтожение
        explicit by_line_cursor(IStream & input, char_type delim)
         : is_(std::addressof(input))
         , delim_(delim)
        {
            this->drop_front();
        }

        // Курсор ввода
        bool operator!() const
        {
            assert(this->is_);

            return !*(this->is_);
        }

        void drop_front()
        {
            assert(this->is_);

            std::getline(*this->is_, this->str_, this->delim_);
        }

        reference front() const
        {
            return this->str_;
        }

    private:
        IStream * is_ = nullptr;
        char_type delim_ = '\n';
        value_type str_;
    };

    namespace cursor
    {
        template <class IStream>
        saga::by_line_cursor<IStream>
        by_line(IStream & input, typename IStream::char_type delim = '\n')
        {
            return saga::by_line_cursor<IStream>(input, delim);
        }
    }
    // namespace cursor
}
// namespace saga

#endif
// Z_SAGA_CURSOR_BY_LINE_HPP_INCLUDED
