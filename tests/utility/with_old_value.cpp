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
#include <saga/utility/with_old_value.hpp>

// Тестовая инфраструктура
#include <catch2/catch_amalgamated.hpp>
#include "../saga_test.hpp"

// Тесты
namespace
{
    static_assert(saga::with_old_value<int>().value() == int{});
    static_assert(saga::with_old_value<int>().old_value() == int{});
}

TEMPLATE_TEST_CASE("with_old_value: default constructor", ""
                   , int, std::string, (std::pair<int, std::string>))
{
    auto const obj = saga::with_old_value<TestType>();

    REQUIRE(obj.value() == TestType());
    REQUIRE(obj.old_value() == TestType());
}

TEST_CASE("with_old_value: value constructor")
{
    using Value = std::string;

    saga_test::property_checker << [](Value const & value)
    {
        saga::with_old_value<Value> const obj(value);

        REQUIRE(obj.value() == value);
        REQUIRE(obj.old_value() == value);
    };
}

TEST_CASE("with_old_value: value assignment")
{
    using Value = std::string;

    saga_test::property_checker << [](Value const & value, Value const & new_value)
    {
        saga::with_old_value<Value> obj(value);

        REQUIRE(obj.value() == value);
        REQUIRE(obj.old_value() == value);

        obj = new_value;

        REQUIRE(obj.value() == new_value);
        REQUIRE(obj.old_value() == value);
    };
}

TEST_CASE("with_old_value: equality")
{
    using Value = std::string;

    saga_test::property_checker << [](Value const & value1, Value const & new_value1
                                      , Value const & value2, Value const & new_value2)
    {
        saga::with_old_value<Value> obj1(value1);
        saga::with_old_value<Value> obj2(value2);

        obj1 = new_value1;
        obj2 = new_value2;

        REQUIRE(obj1 == obj1);
        REQUIRE(obj2 == obj2);

        REQUIRE((obj1 == obj2) == (value1 == value2 && new_value1 == new_value2));
        REQUIRE((obj1 != obj2) == !(obj1 == obj2));
    };
}

TEST_CASE("with_old_value: modifying access")
{
    using Value = unsigned int;

    saga_test::property_checker << [](Value const & value)
    {
        saga::with_old_value<Value> obj(value);

        REQUIRE(obj.value() == value);
        REQUIRE(obj.old_value() == value);

        ++obj.value();

        REQUIRE(obj.value() == value + 1);
        REQUIRE(obj.old_value() == value);
    };
}

TEST_CASE("with_old_value: commit")
{
    using Value = std::string;

    saga_test::property_checker << [](Value const & value, Value const & new_value)
    {
        saga::with_old_value<Value> obj(value);

        REQUIRE(obj.value() == value);
        REQUIRE(obj.old_value() == value);

        obj = new_value;

        REQUIRE(obj.value() == new_value);
        REQUIRE(obj.old_value() == value);

        obj.commit();

        REQUIRE(obj.value() == new_value);
        REQUIRE(obj.old_value() == new_value);
    };
}

TEST_CASE("with_old_value: revert")
{
    using Value = std::string;

    saga_test::property_checker << [](Value const & value, Value const & new_value)
    {
        saga::with_old_value<Value> obj(value);

        REQUIRE(obj.value() == value);
        REQUIRE(obj.old_value() == value);

        obj = new_value;

        REQUIRE(obj.value() == new_value);
        REQUIRE(obj.old_value() == value);

        obj.revert();

        REQUIRE(obj.value() == value);
        REQUIRE(obj.old_value() == value);
    };
}
