/* (c) 2022-2025 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_PE_SEIVE_HPP_INCLUDED
#define Z_PE_SEIVE_HPP_INCLUDED

#include <saga/cursor/indices.hpp>
#include <saga/cursor/subrange.hpp>
#include <saga/math.hpp>
#include <saga/numeric.hpp>

#include <vector>

namespace saga
{
namespace experimental
{
    /**
    @pre first <= last
    @pre <tt> old_primes.empty() || old_primes.back() < first </tt>
    @pre Между primes.back() и first нет простых чисел
    @todo Можно ли унифицировать эту функцию с saga::copy_primes_below?
    */
    template <class IntType>
    std::vector<IntType>
    PE_051_primes(std::vector<IntType> const & old_primes, IntType first, IntType last)
    {
        assert(first <= last);
        assert(old_primes.empty() || old_primes.back() < first);
        assert(old_primes.empty() || (old_primes.front() == IntType(2)));
        assert(!old_primes.empty() || (first <= IntType(2)));

        if(first == last)
        {
            return {};
        }

        first += saga::is_even(first);

        // num = first + index * 2
        // index = (num - first) / 2
        std::vector<unsigned short> seive((last - first) / 2, true);

        auto const cur = saga::cursor::all(seive);

        // Просеиваем уже найденные простые
        for(auto prime : old_primes)
        {
            // Двойки мы пропускаем и так!
            if(prime == 2)
            {
                continue;
            }

            auto num = first + prime - (first % prime);

            num += prime * saga::is_even(num);

            saga::mark_eratosthenes_seive(saga::cursor::drop_front_n(cur, (num - first)/2)
                                          , prime);
        }

        // Просеиваем остальные простые
        for(auto index : saga::cursor::indices(cur.size()))
        {
            if(cur[index])
            {
                auto new_prime = first + 2*index;
                auto const square_index = (saga::square(new_prime) - first)/2;

                if(square_index >= cur.size())
                {
                    break;
                }

                saga::mark_eratosthenes_seive(saga::cursor::drop_front_n(cur, square_index)
                                              , new_prime);
            }
        }

        // Выделяем простые числа
        std::vector<IntType> primes;

        if(old_primes.empty())
        {
            primes.emplace_back(2);
        }

        for(auto index : saga::cursor::indices_of(seive))
        {
            if(seive[index])
            {
                primes.push_back(first + 2 * index);
            }
        }

        return primes;
    }
}
}

#endif
// Z_PE_SEIVE_HPP_INCLUDED
