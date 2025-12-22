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
#include <saga/cursor/istream_cursor.hpp>
#include <saga/cursor/transform.hpp>
#include <saga/numeric.hpp>
#include <saga/utility/functional_macro.hpp>

#include <fstream>

// Инфраструктура тестирования
#include "../saga_test.hpp"
#include <catch2/catch_amalgamated.hpp>

// PE 099 - Наибольшая степенное выражение
namespace
{
    struct base_exponent_pair
    {
        long base = 0;
        long exponent = 0;
    };

    std::istream & operator>>(std::istream & input, base_exponent_pair & obj)
    {
        input >> obj.base;

        auto sep = input.get();

        input >> obj.exponent;

        if(input)
        {
            assert(sep == ',');
        }

        return input;
    }
}

TEST_CASE("PE 099")
{
    std::ifstream file("ProjectEuler/p099_base_exp.txt");

    auto bep_log = [](::base_exponent_pair const & obj) {return obj.exponent * std::log(obj.base);};

    auto input = saga::make_istream_cursor<::base_exponent_pair>(file)
               | saga::cursor::transform(bep_log);

    using ValueAndIndex = std::pair<double, int>;

    auto const result
        = saga::transform_reduce(std::move(input), saga::cursor::iota(1), ValueAndIndex{0, 0}
                                ,SAGA_OVERLOAD_SET(std::max), SAGA_OVERLOAD_SET(std::make_pair));

    REQUIRE(result.second == 709);
}
