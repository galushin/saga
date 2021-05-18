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

// Тестируемый заголовочный файл
#include <saga/algorithm.hpp>

// Инфраструктура тестирования
#include "saga_test.hpp"
#include <catch/catch.hpp>

// Вспомогательные файлы
#include <saga/cursor/subrange.hpp>
#include <saga/cursor/istream_cursor.hpp>

#include <list>
#include <string>

// Тесты
// @todo Аналогичные тесты для forward_list
TEST_CASE("random_position_of: vector")
{
    saga_test::property_checker << [](std::vector<int> const & src)
    {
        auto const pos = saga_test::random_position_of(src);

        REQUIRE(0 <= pos);
        REQUIRE(pos <= src.size());
    };
}

TEST_CASE("random_subrange_of: vector")
{
    saga_test::property_checker << [](std::vector<int> const & src)
    {
        auto const result = saga_test::random_subrange_of(src);

        auto const pos1 = result.begin() - src.begin();
        auto const pos2 = result.end() - src.begin();

        REQUIRE(0 <= pos1);
        REQUIRE(pos1 <= src.size());

        REQUIRE(0 <= pos2);
        REQUIRE(pos2 <= src.size());

        REQUIRE(pos1 <= pos2);
    };
}

TEST_CASE("copy")
{
    using Value = int;
    saga_test::property_checker << [](std::vector<Value> const & src,
                                      std::vector<Value> const & dest_old)
    {
        auto dest = dest_old;

        // Взять подинтервалы контейнеров
        auto const src_subrange = saga_test::random_subrange_of(src);
        auto const dest_subrange = saga_test::random_subrange_of(dest);

        auto const src_cur = saga::cursor::all(src_subrange);
        auto const dest_cur = saga::cursor::all(dest_subrange);

        auto const dest_prefix_size = (dest_cur.begin() - dest.begin());

        assert(0 <= dest_prefix_size && static_cast<size_t>(dest_prefix_size) <= dest.size());

        auto const result = saga::copy(src_cur, dest_cur);

        // Проверка содержимого dest
        auto const n = std::min(src_cur.size(), dest_cur.size());

        REQUIRE(std::equal(dest.begin(), dest.begin() + dest_prefix_size,
                           dest_old.begin(), dest_old.begin() + dest_prefix_size));

        REQUIRE(std::equal(dest.begin() + dest_prefix_size, dest.begin() + dest_prefix_size + n,
                           src_cur.begin(), src_cur.begin() + n));

        REQUIRE(std::equal(dest.begin() + dest_prefix_size + n, dest.end(),
                           dest_old.begin() + dest_prefix_size + n, dest_old.end()));

        // Проверяем возвращаемое значение
        REQUIRE((!result.in || !result.out));
        REQUIRE(!result.in == (src_cur.size() <= dest_cur.size()));

        REQUIRE(result.in.begin() == src_cur.begin() + n);
        REQUIRE(result.in.end() == src_cur.end());

        REQUIRE(result.out.begin() == dest_cur.begin() + n);
        REQUIRE(result.out.end() == dest_cur.end());

        // @todo Проверить begin_orig и end_orig (имена предварительные)
    };
}

TEST_CASE("copy: container to back_inserter")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        std::vector<Value> dest;

        auto const result = saga::copy(saga::cursor::all(src), saga::back_inserter(dest));

        REQUIRE(!result.in);
        REQUIRE(!!result.out);

        REQUIRE(dest == src);
    };
}

// @todo Тест copy c минималистичными типами: из istream_cursor в back_inserter или ostream_joiner

TEST_CASE("reverse")
{
    using Container = std::list<int>;

    // @todo Выбрать заданный подинтервал, а не весь контейнер
    saga_test::property_checker << [](Container const & src)
    {
        auto src_std = src;
        std::reverse(src_std.begin(), src_std.end());

        auto src_saga = src;
        saga::reverse(saga::cursor::all(src_saga));

        REQUIRE(src_std == src_saga);
    };
}

TEST_CASE("reverse_copy : to back_inserter")
{
    using Element = int;
    using Container = std::list<Element>;

    saga_test::property_checker << [](Container const & src)
    {
        auto const src_sub = saga_test::random_subrange_of(src);

        // std
        Container out_std;
        std::reverse_copy(src_sub.begin(), src_sub.end(), std::back_inserter(out_std));

        // saga
        Container out_saga;
        saga::reverse_copy(saga::cursor::all(src_sub), saga::back_inserter(out_saga));

        // Сравнение результатов
        REQUIRE(out_std == out_saga);
    };
}

TEST_CASE("reverse_copy : subcontainer to subcontainer")
{
    saga_test::property_checker << [](std::vector<int> const & src_container,
                                      std::vector<long> dest_container)
    {
        static_assert(!std::is_same<decltype(src_container), decltype(dest_container)>{}, "");

        auto const src_range = saga_test::random_subrange_of(src_container);
        auto const dest_range = saga_test::random_subrange_of(dest_container);

        auto const src = saga::cursor::all(src_range);
        auto const dest = saga::cursor::all(dest_range);

        auto dest_container_std = dest_container;

        auto const result = saga::reverse_copy(src, dest);

        static_assert(std::is_same<decltype(saga::reverse_copy(src, dest))
                                  ,saga::in_out_result<std::remove_cv_t<decltype(src)>
                                                      ,std::remove_cv_t<decltype(dest)>>>{},"");

        auto const n_common = std::min(src.size(), dest.size());

        std::reverse_copy(src.end() - n_common, src.end(),
                          dest_container_std.begin() + (dest.begin() - dest_container.begin()));

        CAPTURE(src_container);

        REQUIRE(dest_container == dest_container_std);

        REQUIRE(result.in.begin() == src.begin());
        REQUIRE(result.in.end() == src.end() - n_common);

        REQUIRE(result.out.begin() == dest.begin() + n_common);
        REQUIRE(result.out.end() == dest.end());
    };
}

TEST_CASE("starts_with : prefix")
{
    saga_test::property_checker << [](std::string const & str)
    {
        auto const prefix = str.substr(0, str.size() / 2);

        REQUIRE(::saga::starts_with(str, prefix));
    };
}

TEST_CASE("starts_with : common")
{
    saga_test::property_checker << [](std::string const & str, std::string const & test)
    {
        auto const expected = test.size() <= str.size() && test == str.substr(0, test.size());

        CAPTURE(str, test);

        REQUIRE(::saga::starts_with(str, test) == expected);
    };
}

TEST_CASE("ends_with : suffix")
{
    saga_test::property_checker << [](std::string const & str)
    {
        auto const suffix = str.substr(str.size() / 2);

        REQUIRE(::saga::ends_with(str, suffix));
    };
}

TEST_CASE("ends_with : common")
{
    saga_test::property_checker << [](std::string const & str, std::string const & test)
    {
        auto const expected = test.size() <= str.size()
                            && test == str.substr(str.size() - test.size());

        CAPTURE(str, test);

        REQUIRE(::saga::ends_with(str, test) == expected);
    };
}
