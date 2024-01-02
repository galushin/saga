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

#ifndef Z_SAGA_NUMERIC_PRIMES_CURSOR_HPP_INCLUDED
#define Z_SAGA_NUMERIC_PRIMES_CURSOR_HPP_INCLUDED

#include <saga/cursor/cursor_facade.hpp>
#include <saga/numeric/is_prime.hpp>

namespace saga
{
    template <class IntType>
    class primes_cursor
     : saga::cursor_facade<primes_cursor<IntType>, std::vector<IntType> const &>
    {
        using Container = std::vector<IntType>;

    public:
        // Типы
        using cursor_category = std::input_iterator_tag;
        using difference_type = typename Container::difference_type;
        using reference = Container const &;

        // Создание, копирование, уничтожение
        primes_cursor()
         : primes_(1, IntType(2))
        {}

        primes_cursor(primes_cursor const &) = delete;
        primes_cursor(primes_cursor &&) = default;

        primes_cursor & operator=(primes_cursor const &) = delete;
        primes_cursor & operator=(primes_cursor &&) = default;

        // Курсор ввода
        bool operator!() const
        {
            return false;
        }

        reference front() const
        {
            return this->primes_;
        }

        Container const * operator->() const
        {
            return std::addressof(this->primes_);
        }

        void drop_front()
        {
            auto num = this->primes_.back() + 1 + this->primes_.back() % 2;

            for(;;)
            {
                if(saga::is_prime_sorted(num, this->primes_, saga::unsafe_tag_t{}))
                {
                    this->primes_.push_back(num);
                    break;
                }

                num += ((num % 6 == 1) ? 4 : 2);
            }
        }

    private:
        Container primes_;
    };
}

#endif
// Z_SAGA_NUMERIC_PRIMES_CURSOR_HPP_INCLUDED
