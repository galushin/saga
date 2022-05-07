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

// Тестируемый заголовочный файл
#include <saga/cursor/subrange.hpp>
#include <saga/cursor/take.hpp>

// Инфраструктура тестирования
#include "../saga_test.hpp"
#include <catch/catch.hpp>

// Вспомогательные файлы
#include <saga/algorithm.hpp>
#include <saga/cursor/istream_cursor.hpp>

// Тесты
TEST_CASE("take_cursor: input")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & values)
    {
        auto src_in = saga_test::make_istringstream_from_range(values);

        auto const num = saga_test::random_uniform(0, 2*values.size());

        // Выполнение
        std::vector<Value> dest_saga;
        saga::copy(saga::cursor::take(saga::make_istream_cursor<Value>(src_in), num)
                   , saga::back_inserter(dest_saga));

        // Проверка
        auto const n_expected = std::min(values.size(), num);

        REQUIRE(dest_saga.size() == n_expected);

        REQUIRE(std::equal(dest_saga.begin(), dest_saga.end()
                           , values.begin(), values.begin() + n_expected));
    };
}

TEST_CASE("take_cursor: output")
{
    using Value = long;

    saga_test::property_checker
    << [](std::vector<Value> const & src, std::vector<Value> const & dest_old)
    {
        auto const num = saga_test::random_uniform(0, 2*dest_old.size());

        auto const input = saga::cursor::all(src);

        // Выполнение
        auto dest = dest_old;
        auto const output = saga::cursor::take(saga::cursor::all(dest), num);

        auto const result = saga::copy(input, output);

        // Проверка
        auto const n_expected = std::min({src.size(), dest.size(), num});

        REQUIRE(result.in == saga::cursor::drop_front_n(input, n_expected));
        REQUIRE(result.out == saga::cursor::drop_front_n(output, n_expected));
        REQUIRE(result.out.base() == saga::cursor::drop_front_n(output.base(), n_expected));

        REQUIRE(std::equal(dest.begin(), dest.begin() + n_expected
                           , src.begin(), src.begin() + n_expected));

        REQUIRE(std::equal(dest.begin() + n_expected, dest.end()
                           , dest_old.begin() + n_expected, dest_old.end()));

    };
}
