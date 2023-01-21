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

// Инфраструктура тестирования
#include <catch2/catch_amalgamated.hpp>

// Используемые возможности
#include <saga/algorithm.hpp>
#include <saga/cursor/indices.hpp>
#include <saga/cursor/istream_cursor.hpp>
#include <saga/cursor/subrange.hpp>
#include <saga/cursor/to.hpp>

#include <fstream>
#include <sstream>

// PE 018: Путь наибольшей суммы (часть I)
namespace
{
    static std::string pe018_sample_data{
        "3\n"
        "7 4\n"
        "2 4 6\n"
        "8 5 9 3"};

    static std::string pe018_data{
        "75\n"
        "95 64\n"
        "17 47 82\n"
        "18 35 87 10\n"
        "20 04 82 47 65\n"
        "19 01 23 75 03 34\n"
        "88 02 77 73 07 63 67\n"
        "99 65 04 28 06 16 70 92\n"
        "41 41 26 56 83 40 80 70 33\n"
        "41 48 72 33 47 32 37 16 94 29\n"
        "53 71 44 65 25 43 91 52 97 51 14\n"
        "70 11 33 28 77 73 17 78 39 68 17 57\n"
        "91 71 52 38 17 14 91 43 58 50 27 29 48\n"
        "63 66 04 68 89 53 67 30 73 16 69 87 40 31\n"
        "04 62 98 27 23 09 70 98 73 93 38 53 60 04 23"};

    template <class Vector>
    void projectEuler_018_update(Vector & result, Vector & row)
    {
        if(row.size() > 1)
        {
            row.front() += result.front();
            row.back() += result.back();

            for(auto index : saga::cursor::indices(1u, result.size()))
            {
                row[index] += std::max(result[index-1], result[index]);
            }
        }

        result.swap(row);
    }

    template <class IntType>
    IntType projectEuler_018(std::istream & src_in)
    {
        std::vector<IntType> result;

        std::string line;
        std::vector<IntType> row;

        for(; src_in;)
        {
            std::getline(src_in, line);

            if(!src_in)
            {
                break;
            }

            std::istringstream line_in(line);

            row = saga::make_istream_cursor<IntType>(line_in)
                | saga::cursor::to<std::vector>();

            ::projectEuler_018_update(result, row);
        }

        assert(!result.empty());

        return saga::max_element(saga::cursor::all(result)).front();
    }


    template <class IntType>
    IntType projectEuler_018_string(std::string const & text)
    {
        std::istringstream istr(text);

        return ::projectEuler_018<IntType>(istr);
    }
}

TEST_CASE("PE 018")
{
    REQUIRE(projectEuler_018_string<int>(pe018_sample_data) == 23);
    REQUIRE(projectEuler_018_string<int>(pe018_data) == 1074);
}

// PE 067: Путь наибольшей суммы (часть II)
TEST_CASE("PE 067")
{
    std::ifstream file("ProjectEuler/p067_triangle.txt");

    REQUIRE(!!file);

    REQUIRE(::projectEuler_018<int>(file) == 7273);
}
