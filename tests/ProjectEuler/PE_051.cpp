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

// Используемые возможности
#include "./seive.hpp"

#include <saga/algorithm.hpp>
#include <saga/cursor/indices.hpp>
#include <saga/cursor/subrange.hpp>
#include <saga/math.hpp>
#include <saga/numeric.hpp>
#include <saga/utility/functional_macro.hpp>

// Инфраструктура тестирования
#include "../saga_test.hpp"
#include <catch2/catch_amalgamated.hpp>

// PE 051 - Замена цифр в простых числах
namespace
{
    template <class Cursor>
    saga::cursor_difference_t<Cursor>
    PE_051_count(Cursor cur, std::string const & str, char old_digit, char first, char last)
    {
        saga::cursor_difference_t<Cursor> counter(0);

        for(auto new_digit : saga::cursor::indices(first, last))
        {
            auto new_str = str;
            saga::replace(saga::cursor::all(new_str), old_digit, new_digit);

            counter += (new_str != str) && saga::binary_search(cur, new_str);
        }

        return counter;
    }

    template <class Cursor>
    bool PE_051_check(Cursor cur, std::string const & str, std::ptrdiff_t family_size)
    {
        for(auto old_digit : saga::cursor::indices('0', '9' - family_size + 1))
        {
            if(::PE_051_count(cur.dropped_front(), str, old_digit, '0', old_digit) > 0)
            {
                continue;
            }

            if(::PE_051_count(cur, str, old_digit, old_digit + 1, '9' + 1) + 1 >= family_size)
            {
                return true;
            }
        }

        return false;
    }

    template <class Cursor>
    Cursor PE_051_find(Cursor cur, std::ptrdiff_t const family_size)
    {
        for(; !!cur;)
        {
            auto cur_next = cur;
            ++ cur_next;

            if(::PE_051_check(cur_next, *cur, family_size))
            {
                break;
            }

            cur = cur_next;
        }

        return cur;
    }

    template <class IntType>
    std::string PE_051(std::ptrdiff_t const family_size)
    {
        std::vector<IntType> old_primes{};
        std::vector<IntType> primes = saga::primes_below(IntType(10));

        for(auto limit = IntType(10);; limit *= 10)
        {
            std::vector<std::string> primes_str;
            saga::transform(saga::cursor::all(primes), saga::back_inserter(primes_str)
                            , SAGA_OVERLOAD_SET(std::to_string));

            auto const cur = ::PE_051_find(saga::cursor::all(primes_str), family_size);

            if(!!cur)
            {
                return *cur;
            }

            old_primes.insert(old_primes.end(), primes.begin(), primes.end());
            primes = saga::experimental::PE_051_primes(old_primes, limit, limit * 10);
        }
    }
}

TEST_CASE("PE_051_primes")
{
    REQUIRE(saga::experimental::PE_051_primes({}, 2, 2).empty());

    auto const primes1 = saga::primes_below(10);
    auto const primes2 = saga::experimental::PE_051_primes({}, 2, 10);

    REQUIRE(primes1 == primes2);
}

TEST_CASE("primes multistep")
{
    auto const limit1 = 10;
    auto const limit2 = 1000;

    REQUIRE(limit1 <= limit2);

    // Все сразу
    auto const primes_all = saga::primes_below(limit2);

    // В два этапа
    auto const primes1 = saga::primes_below(limit1);

    auto const primes2 = saga::experimental::PE_051_primes(primes1, limit1, limit2);

    REQUIRE(!primes2.empty());
    REQUIRE(saga::is_sorted(saga::cursor::all(primes2)));
    REQUIRE(primes2.front() >= limit1);
    REQUIRE(primes2.back() < limit2);

    // Сравниваем
    auto result = primes1;
    result.insert(result.end(), primes2.begin(), primes2.end());

    REQUIRE(result == primes_all);
}

TEST_CASE("PE 051")
{
    REQUIRE(::PE_051<long>(6) == "13");
    REQUIRE(::PE_051<long>(7) == "56003");
    REQUIRE(::PE_051<long>(8) == "121313");
}
