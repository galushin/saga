/* (c) 2020-2021 Галушин Павел Викторович, galushin@gmail.com

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

// Тестируемый файл
#include <saga/cursor/subrange.hpp>

// Тестовая инфраструктура
#include "./../saga_test.hpp"
#include <catch/catch.hpp>

// Используются при тестировании
#include <saga/algorithm.hpp>

#include <forward_list>
#include <sstream>

// Тесты
TEST_CASE("equal: input sequence")
{
    saga_test::property_checker << [](std::vector<int> const & xs_src,
                                      std::vector<int> const & ys_src)
    {
        auto xs_is = saga_test::make_istringstream_from_range(xs_src);
        auto ys_is = saga_test::make_istringstream_from_range(ys_src);

        auto xs_cur = saga::make_subrange_cursor(std::istream_iterator<int>(xs_is),
                                                 std::istream_iterator<int>());

        auto ys_cur = saga::make_subrange_cursor(std::istream_iterator<int>(ys_is),
                                                 std::istream_iterator<int>());

        REQUIRE(saga::equal(std::move(xs_cur), std::move(ys_cur)) == (xs_src == ys_src));
    };
}

TEST_CASE("equal: forward sequence")
{
    saga_test::property_checker << [](std::forward_list<int> const & xs,
                                      std::forward_list<int> const & ys)
    {
        REQUIRE(saga::equal(saga::cursor::all(xs), saga::cursor::all(xs)));
        REQUIRE(saga::equal(saga::cursor::all(xs), saga::cursor::all(ys)) == (xs == ys));
    };
}
