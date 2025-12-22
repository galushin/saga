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
#include <saga/action/sort.hpp>
#include <saga/algorithm.hpp>
#include <saga/cursor/iota.hpp>
#include <saga/numeric.hpp>

// Инфраструктура тестирования
#include <catch2/catch_amalgamated.hpp>

// PE 062 - Кубические перестановки
namespace
{
    template <class IntType>
    IntType PE_062(std::size_t const repeats)
    {
        assert(repeats > 0);

        std::map<std::string, std::pair<IntType, std::size_t>> data;

        auto current_length = std::size_t(1);

        for(auto const & number : saga::cursor::iota(IntType(1)))
        {
            if(number >= std::numeric_limits<IntType>::max() / number / number)
            {
                break;
            }

            auto const cube = saga::power_natural(number, 3);
            auto const str = std::to_string(cube) | saga::action::sort;

            if(str.size() > current_length)
            {
                auto pred = [&](auto const & item) { return item.second.second == repeats; };
                auto pos = saga::find_if(saga::cursor::all(data), pred);

                if(!!pos)
                {
                    return pos.front().second.first;
                }

                data.clear();
                ++ current_length;
            }

            auto & ref = data[str];

            if(ref.second == 0)
            {
                ref.first = cube;
            }

            ref.second += 1;
        }

        throw std::range_error("PE 062 : answer is not found");
    }
}

TEST_CASE("PE 062")
{
    using IntType = std::uint64_t;

    REQUIRE(::PE_062<IntType>(3) == 41063625);
    REQUIRE(::PE_062<IntType>(5) == 127035954683);

    REQUIRE(::PE_062<IntType>(6) == 1000600120008);

    REQUIRE_THROWS(::PE_062<int>(5));
}
