/* (c) 2021 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_CURSOR_ISTREAM_CURSOR_HPP_INCLUDED
#define Z_SAGA_CURSOR_ISTREAM_CURSOR_HPP_INCLUDED

#include <cassert>
#include <memory>

namespace saga
{
    template <class Value, class IStream>
    class istream_cursor
    {
    public:
        // Создание, копирование, уничтожение
        istream_cursor(IStream & stream)
         : stream_(std::addressof(stream))
         , value_()
        {
            ++*this;
        }

        istream_cursor(istream_cursor const &) = delete;
        istream_cursor(istream_cursor &&) = default;

        istream_cursor & operator=(istream_cursor const &) = delete;
        istream_cursor & operator=(istream_cursor &&) = default;

        // Курсор ввода
        constexpr bool operator!() const
        {
            return this->stream_ == nullptr;
        }

        istream_cursor & operator++()
        {
            assert(!!*this);

            if(!(*this->stream_ >> this->value_))
            {
                this->stream_ = nullptr;
            }

            return *this;
        }

        Value const & operator*() const
        {
            assert(!!*this);

            return this->value_;
        }

    private:
        IStream * stream_;
        Value value_ {};
    };

    template <class Value, class IStream>
    auto make_istream_cursor(IStream & is)
    -> istream_cursor<Value, IStream>
    {
        return istream_cursor<Value, IStream>(is);
    }
}
// namespace saga

#endif
// Z_SAGA_CURSOR_ISTREAM_CURSOR_HPP_INCLUDED
