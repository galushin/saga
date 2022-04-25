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

// Тестируемый файл
#include <saga/cursor/reverse.hpp>

// Инфраструктура тестирования
#include "../saga_test.hpp"
#include <catch/catch.hpp>

// Используемые файлы
#include <saga/algorithm.hpp>
#include <saga/cursor/subrange.hpp>

#include <forward_list>
#include <list>

TEST_CASE("reverse_cursor: value_type and cursor_category")
{
    using Value = int;

    std::forward_list<Value> const c_fwd;
    std::list<Value> const c_bi;
    std::vector<Value> const c_ra;

    auto const r_fwd = saga::cursor::reverse(saga::cursor::all(c_fwd));
    auto const r_bi = saga::cursor::reverse(saga::cursor::all(c_bi));
    auto const r_ra = saga::cursor::reverse(saga::cursor::all(c_ra));

    static_assert(std::is_same<decltype(r_fwd)::value_type, Value>{}, "");
    static_assert(std::is_same<decltype(r_fwd)::cursor_category, std::forward_iterator_tag>{}, "");

    static_assert(std::is_same<decltype(r_bi)::value_type, Value>{}, "");
    static_assert(std::is_same<decltype(r_bi)::cursor_category
                              , std::bidirectional_iterator_tag>{}, "");

    static_assert(std::is_same<decltype(r_ra)::value_type, Value>{}, "");
    static_assert(std::is_same<decltype(r_ra)::cursor_category
                              , std::random_access_iterator_tag>{}, "");
}

TEST_CASE("cursor::reverse of reverse_cursor")
{
    saga_test::property_checker << [](std::list<int> const & data)
    {
        auto const cur = saga_test::random_subcursor_of(saga::cursor::all(data));
        auto const rev = saga::cursor::reverse(cur);
        auto const rev_rev = saga::cursor::reverse(rev);

        static_assert(std::is_same<decltype(cur), decltype(rev_rev)>{}, "");
        REQUIRE(rev_rev == cur);
    };

    {
        static constexpr int data[] = {1, 3, 7, 11};

        constexpr auto cur = saga::cursor::all(data);
        constexpr auto rev = saga::cursor::reverse(cur);
        constexpr auto rev_rev = saga::cursor::reverse(rev);

        static_assert(std::is_same<decltype(cur), decltype(rev_rev)>{}, "");
        static_assert(rev_rev == cur, "");
    }
}

TEST_CASE("reverse reverse_cursor")
{
    using Value = int;

    saga_test::property_checker << [](std::list<Value> const & data_old)
    {
        auto data = data_old;

        saga::reverse(saga::cursor::all(data));

        saga::reverse(saga::cursor::reverse(saga::cursor::all(data)));

        REQUIRE(data == data_old);
    };
}

TEST_CASE("reverse_cursor: dropped_front and dropped_back")
{
    using Value = int;

    saga_test::property_checker << [](std::list<Value> const & data)
    {
        auto const cur = saga_test::random_subcursor_of(saga::cursor::all(data));

        auto const rev_cur = saga::cursor::reverse(cur);

        REQUIRE(rev_cur.dropped_front().base() == cur.dropped_back());
        REQUIRE(rev_cur.dropped_back().base() == cur.dropped_front());
    };
}

TEST_CASE("reverse_cursor: exhaust_front and exhaust_back")
{
    using Value = int;

    saga_test::property_checker
    << [](std::list<Value> const & data)
    {
        auto cur = saga_test::random_subcursor_of(saga::cursor::all(data));

        auto rev_cur = saga::cursor::reverse(cur);

        rev_cur.exhaust_front();
        cur.exhaust_back();

        REQUIRE(rev_cur.base() == cur);
    }
    << [](std::list<Value> const & data)
    {
        auto cur = saga_test::random_subcursor_of(saga::cursor::all(data));

        auto rev_cur = saga::cursor::reverse(cur);

        rev_cur.exhaust_back();
        cur.exhaust_front();

        REQUIRE(rev_cur.base() == cur);
    };
}

TEST_CASE("reverse_cursor: forget_front and forget_back")
{
    using Value = int;

    saga_test::property_checker
    << [](std::list<Value> const & data)
    {
        auto cur = saga_test::random_subcursor_of(saga::cursor::all(data));

        auto rev_cur = saga::cursor::reverse(cur);

        rev_cur.forget_front();
        cur.forget_back();

        REQUIRE(rev_cur.base() == cur);
    }
    << [](std::list<Value> const & data)
    {
        auto cur = saga_test::random_subcursor_of(saga::cursor::all(data));

        auto rev_cur = saga::cursor::reverse(cur);

        rev_cur.forget_back();
        cur.forget_front();

        REQUIRE(rev_cur.base() == cur);
    };
}

TEST_CASE("reverse_cursor: rewind_front and rewind_back")
{
    using Value = int;

    saga_test::property_checker
    << [](std::list<Value> const & data)
    {
        auto cur = saga_test::random_subcursor_of(saga::cursor::all(data));

        auto rev_cur = saga::cursor::reverse(cur);

        rev_cur.rewind_front();
        cur.rewind_back();

        REQUIRE(rev_cur.base() == cur);
    }
    << [](std::list<Value> const & data)
    {
        auto cur = saga_test::random_subcursor_of(saga::cursor::all(data));

        auto rev_cur = saga::cursor::reverse(cur);

        rev_cur.rewind_back();
        cur.rewind_front();

        REQUIRE(rev_cur.base() == cur);
    };
}

TEST_CASE("reverse_cursor: size and random access")
{
      using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & data)
    {
        auto cur = saga_test::random_subcursor_of(saga::cursor::all(data));

        auto rev_cur = saga::cursor::reverse(cur);

        REQUIRE(rev_cur.size() == cur.size());

        if(!!cur)
        {
            auto const index = saga_test::random_uniform(0, cur.size() - 1);
            auto const rev_index = cur.size() - index - 1;

            REQUIRE(rev_cur[index] == cur[rev_index]);
            REQUIRE(std::addressof(rev_cur[index]) == std::addressof(cur[rev_index]));

            REQUIRE(rev_cur[rev_index] == cur[index]);
            REQUIRE(std::addressof(rev_cur[rev_index]) == std::addressof(cur[index]));
        }
    };
}

TEST_CASE("reverse_cursor: drop_front(n) and drop_back(n)")
{
    using Value = int;

    saga_test::property_checker
    << [](std::vector<Value> const & data)
    {
        auto cur = saga_test::random_subcursor_of(saga::cursor::all(data));

        auto rev_cur = saga::cursor::reverse(cur);

        auto const num = saga_test::random_uniform(0, saga::cursor::size(cur));

        rev_cur.drop_front(num);
        cur.drop_back(num);

        REQUIRE(rev_cur.base() == cur);
    }
    << [](std::vector<Value> const & data)
    {
        auto cur = saga_test::random_subcursor_of(saga::cursor::all(data));

        auto rev_cur = saga::cursor::reverse(cur);

        auto const num = saga_test::random_uniform(0, saga::cursor::size(cur));

        rev_cur.drop_back(num);
        cur.drop_front(num);

        REQUIRE(rev_cur.base() == cur);
    };
}

TEST_CASE("reverse_cursor: splice")
{
    using Value = int;

    saga_test::property_checker
    << [](std::vector<Value> const & data)
    {
        auto const cur = saga_test::random_subcursor_of(saga::cursor::all(data));

        auto const rev_cur = saga::cursor::reverse(cur);

        auto res1 = cur.dropped_front();
        res1.splice(cur);

        auto res2 = rev_cur;
        res2.splice(rev_cur.dropped_back());

        REQUIRE(res2.base() == res1);
    }
    << [](std::vector<Value> const & data)
    {
        auto const cur = saga_test::random_subcursor_of(saga::cursor::all(data));

        auto const rev_cur = saga::cursor::reverse(cur);

        auto res1 = cur;
        res1.splice(cur.dropped_back());

        auto res2 = rev_cur.dropped_front();
        res2.splice(rev_cur);

        REQUIRE(res2.base() == res1);
    };
}
