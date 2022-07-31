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

#ifndef Z_SAGA_FLAT_SET_HPP_INCLUDED
#define Z_SAGA_FLAT_SET_HPP_INCLUDED

/** @file saga/flat_set.hpp
 @brief Аналог std::set на основе контейнера с (предположительно) непрерывной памятью
*/

#include <saga/algorithm.hpp>
#include <saga/cursor/subrange.hpp>

namespace saga
{
    template <class Key, class Compare = std::less<Key>>
    class flat_set
     : private Compare
    {
        using Container = std::vector<Key>;
    public:
        // Типы
        using key_type = Key;
        using value_type = Key;
        using value_compare = Compare;
        using size_type = std::size_t;
        using iterator = typename Container::iterator;
        using const_iterator = typename Container::const_iterator;

        // Создание, копирование, уничтожение
        flat_set() = default;

        // Размер и ёмкость
        size_type size() const
        {
            return this->data_.size();
        }

        // Итераторы
        const_iterator begin() const
        {
            return this->data_.begin();
        }

        const_iterator end() const
        {
            return this->data_.end();
        }

        // Модифицирующие операции
        iterator insert(const_iterator, value_type && value)
        {
            // Находим место
            auto pos = this->lower_bound(value);

            // Если !pos, то *pos >= value
            if(pos == this->end() || this->cmp_ref()(value, *pos))
            {
                return this->data_.insert(pos, std::move(value));
            }
            else
            {
                return pos;
            }
        }

        // Операции множества
        iterator lower_bound(key_type const & key)
        {
            return saga::lower_bound(saga::cursor::all(this->data_)
                                     , key, std::ref(this->cmp_ref())).begin();
        }

    private:
        value_compare & cmp_ref()
        {
            return *this;
        }

        Container data_;
    };
}
// namespace saga

#endif
// Z_SAGA_FLAT_SET_HPP_INCLUDED
