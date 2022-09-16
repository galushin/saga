/* (c) 2021 Галушин Павел Викторович, galushin@gmail.com

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
#include <saga/cursor/istream_cursor.hpp>

// Инфраструктура тестирования
#include "../saga_test.hpp"
#include <catch2/catch_amalgamated.hpp>

// Вспомогательные файлы
#include <sstream>

// Тесты
static_assert(std::is_move_constructible<saga::istream_cursor<int, std::istream>>{}, "");
static_assert(std::is_move_assignable<saga::istream_cursor<int, std::istream>>{}, "");

static_assert(!std::is_copy_constructible<saga::istream_cursor<int, std::istream>>{}, "");
static_assert(!std::is_copy_assignable<saga::istream_cursor<int, std::istream>>{}, "");

TEST_CASE("istream_cursor")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & values)
    {
        // Создание потока ввода с заданным содержимым
        auto input_stream = saga_test::make_istringstream_from_range(values);

        // Чтение всей последовательности через курсор
        std::vector<Value> result;
        for(auto cur = saga::make_istream_cursor<Value>(input_stream); !!cur; ++ cur)
        {
            result.push_back(*cur);
        }

        REQUIRE(result == values);
    };
}
