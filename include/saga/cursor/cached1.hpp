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

#ifndef Z_SAGA_CURSOR_CACHED1_HPP_INCLUDED
#define Z_SAGA_CURSOR_CACHED1_HPP_INCLUDED

/** @file saga/cursor/cached1.hpp
@brief Адаптер курсора, кэширующий последнее значение базового курсора
*/

#include <saga/utility/pipeable.hpp>
#include <saga/cursor/cursor_facade.hpp>
#include <saga/cursor/cursor_traits.hpp>

#include <optional>

namespace saga
{
    template <class Cursor>
    class cached1_cursor
     : saga::cursor_facade<cached1_cursor<Cursor>, saga::cursor_value_t<Cursor> const &>
    {
    public:
        // Типы
        using value_type = saga::cursor_value_t<Cursor>;
        using reference = value_type const &;

        // Создание, копирование, уничтожение
        explicit cached1_cursor(Cursor cur)
         : base_(std::move(cur))
        {}

        // Однопроходный курсор
        bool operator!() const
        {
            return !this->base();
        }

        reference front()
        {
            assert(!!this->base());

            if(!this->cache_.has_value())
            {
                this->cache_ = this->base_.front();
            }

            assert(this->cache_.has_value());

            return *this->cache_;
        }

        void drop_front()
        {
            this->base_.drop_front();

            this->cache_.reset();
        }

        // Адаптер курсора
        Cursor const & base() const
        {
            return this->base_;
        }

    private:
        Cursor base_;
        std::optional<value_type> cache_;
    };

    namespace cursor
    {
        inline constexpr auto cached1
            = saga::make_pipeable([](auto cur)
                                  {return saga::cached1_cursor<decltype(cur)>(std::move(cur));});
    }
    // namespace cursor
}
// namespace saga

#endif
// Z_SAGA_CURSOR_CACHED1_HPP_INCLUDED
