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
#include <saga/cursor/istream_cursor.hpp>

#include <forward_list>
#include <list>
#include <sstream>

// Тесты
TEST_CASE("equal: input sequence")
{
    saga_test::property_checker << [](std::vector<int> const & xs_src,
                                      std::vector<int> const & ys_src)
    {
        auto xs_is = saga_test::make_istringstream_from_range(xs_src);
        auto ys_is = saga_test::make_istringstream_from_range(ys_src);

        auto xs_cur = saga::make_istream_cursor<int>(xs_is);
        auto ys_cur = saga::make_istream_cursor<int>(ys_is);

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

TEST_CASE("iterator rebase")
{
    using Container1 = std::list<int>;
    using Container2 = std::forward_list<long>;

    saga_test::property_checker
    << [](Container1 const & src, Container2 dest)
    {
        dest.insert_after(dest.before_begin(), src.begin(), src.end());

        auto const pos = saga_test::random_iterator_of(src);

        auto const result_const
            = saga::rebase_iterator(pos, src, saga::as_const(dest), saga::unsafe_tag_t{});

        auto const result_mutable
            = saga::rebase_iterator(pos, src, dest, saga::unsafe_tag_t{});

        static_assert(std::is_same<decltype(result_const), Container2::const_iterator const>{}, "");
        static_assert(std::is_same<decltype(result_mutable), Container2::iterator const>{}, "");

        REQUIRE(std::distance(dest.cbegin(), result_const) == std::distance(src.begin(), pos));
        REQUIRE(std::distance(dest.begin(), result_mutable) == std::distance(src.begin(), pos));
    };
}

TEST_CASE("subrange cursor rebase")
{
    saga_test::property_checker << [](std::list<int> const & src, std::vector<int> dest)
    {
        dest.insert(dest.end(), src.begin(), src.end());

        auto const cur = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const result_mutable = saga::rebase_cursor(cur, dest);
        auto const result_const = saga::rebase_cursor(cur, saga::as_const(dest));

        static_assert(std::is_same<decltype(result_mutable)
                                  ,decltype(saga::cursor::all(dest)) const>{}, "");
        static_assert(std::is_same<decltype(result_const)
                                  ,decltype(saga::cursor::all(saga::as_const(dest))) const>{}, "");

        REQUIRE(result_const.begin()
                == saga::rebase_iterator(cur.begin(), src, dest, saga::unsafe_tag_t{}));
        REQUIRE(result_const.end()
                == saga::rebase_iterator(cur.end(), src, dest, saga::unsafe_tag_t{}));

        REQUIRE(result_const.dropped_front().begin() == dest.begin());
        REQUIRE(result_const.dropped_back().end() == dest.end());

        REQUIRE(result_mutable.begin()
                == saga::rebase_iterator(cur.begin(), src, dest, saga::unsafe_tag_t{}));
        REQUIRE(result_mutable.end()
                == saga::rebase_iterator(cur.end(), src, dest, saga::unsafe_tag_t{}));

        REQUIRE(result_mutable.dropped_front().begin() == dest.begin());
        REQUIRE(result_mutable.dropped_back().end() == dest.end());
    };
}
