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
#include <saga/container/reserve_if_supported.hpp>

// Инфраструктура тестирования
#include "../saga_test.hpp"
#include <catch/catch.hpp>

// Вспомогательные файлы
#include <list>
#include <vector>

// Тесты
TEST_CASE("reserve_if_supported: list")
{
    saga_test::property_checker << [](std::list<int> const & container_old
                                      , saga_test::container_size<> const & new_capacity)
    {
        auto container = container_old;

        saga::reserve_if_supported(container, new_capacity);

        REQUIRE(container == container_old);
    };
}

TEMPLATE_TEST_CASE("reserve_if_supported: vector-like", "", (std::vector<int>), std::string)
{
    using Container = TestType;

    saga_test::property_checker << [](Container const & container_old
                                      , saga_test::container_size<> const & new_capacity)
    {
        auto container = container_old;

        saga::reserve_if_supported(container, new_capacity);

        REQUIRE(container == container_old);
        REQUIRE(container.capacity() >= new_capacity);
    };
}
