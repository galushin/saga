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

#ifndef Z_SAGA_NUMERIC_IS_PRIME_HPP_INCLUDED
#define Z_SAGA_NUMERIC_IS_PRIME_HPP_INCLUDED

#include <saga/algorithm.hpp>
#include <saga/cursor/subrange.hpp>
#include <saga/cursor/take_while.hpp>
#include <saga/defs.hpp>
#include <saga/math.hpp>

namespace saga
{
    /** @brief Определяет, является ли число @c num простым
    @pre primes содержит все простые числа, не превосходящие <tt> sqrt(num) </tt>, упорядоченные
    по возрастанию
    */
    template <class IntType, class Container>
    bool is_prime_sorted(IntType const num, Container const & primes, saga::unsafe_tag_t)
    {
        SAGA_ASSERT_AUDIT(saga::is_sorted(saga::cursor::all(primes)));

        assert(num >= 0);

        auto cur = saga::cursor::all(primes)
                 | saga::cursor::take_while([&](auto const & arg){return saga::square(arg)<=num;});

        return saga::none_of(std::move(cur), [&](auto const & arg){ return num % arg == 0;});
    }
}

#endif
// Z_SAGA_NUMERIC_IS_PRIME_HPP_INCLUDED
