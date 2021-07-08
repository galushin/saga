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

// Тестируемый файл
#include <saga/any.hpp>

// Тестовая инфраструктура
#include "./saga_test.hpp"
#include <catch/catch.hpp>

// Вспомогательные файлы

// Тесты
TEST_CASE("any - default constructor")
{
    saga::any const obj{};

    static_assert(noexcept(saga::any{}), "default ctor must be noexcept");

    REQUIRE(!obj.has_value());
    static_assert(noexcept(obj.has_value()), "has_value must be noexcept");

    REQUIRE(obj.type() == typeid(void));
    static_assert(noexcept(obj.type()), "type() must be noexcept");
    static_assert(std::is_same<decltype(obj.type()), std::type_info const &>{}, "");

    REQUIRE(saga::any_cast<std::string>(&obj) == nullptr);
    static_assert(std::is_same<decltype(saga::any_cast<long>(&obj)), long const *>{}, "");
    static_assert(noexcept(saga::any_cast<std::string>(&obj))
                  , "any_cast for pointer must be noexcept");

    saga::any var{};
    REQUIRE(var.has_value() == false);
    REQUIRE(var.type() == typeid(void));
    REQUIRE(saga::any_cast<double>(&var) == nullptr);
    static_assert(std::is_same<decltype(saga::any_cast<long>(&var)), long *>{}, "");
}

TEST_CASE("any_cast for nullptr")
{
    REQUIRE(saga::any_cast<double>(static_cast<saga::any*>(nullptr)) == nullptr);
    REQUIRE(saga::any_cast<std::vector<int>>((static_cast<saga::any const*>(nullptr))) == nullptr);
}

TEMPLATE_TEST_CASE("any - value constructor", "any", int, double, std::string, (std::vector<long>))
{
    using Value = TestType;

    saga_test::property_checker << [](Value const & value)
    {
        saga::any const obj(value);

        REQUIRE(obj.has_value());
        REQUIRE(obj.type() == typeid(Value));

        auto ptr = saga::any_cast<Value>(&obj);
        static_assert(std::is_same<decltype(ptr), Value const *>{}, "");

        REQUIRE(ptr != nullptr);
        REQUIRE(*ptr == value);
    };
}

static_assert(!std::is_constructible<saga::any, std::unique_ptr<int>>{}
              , "Must be Cpp17CopyConstructible");

TEST_CASE("any - value constructor performs move")
{
    using Container = std::vector<int>;
    saga_test::property_checker << [](Container const & value_old)
    {
        auto value = value_old;
        saga::any const obj(std::move(value));

        REQUIRE(value.empty());

        REQUIRE(obj.has_value());
        REQUIRE(obj.type() == typeid(Container));

        auto ptr = saga::any_cast<Container>(&obj);

        REQUIRE(ptr != nullptr);
        REQUIRE(*ptr == value_old);
    };
}
