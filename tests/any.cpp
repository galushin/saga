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
#include <set>
#include <saga/utility/as_const.hpp>

namespace
{
    using Value_types_list = std::tuple<int, double, std::string, std::vector<long>>;
}

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
    static_assert(noexcept(saga::any_cast<std::string>(&var)), "");
}

TEST_CASE("any_cast for nullptr")
{
    REQUIRE(saga::any_cast<double>(static_cast<saga::any*>(nullptr)) == nullptr);
    REQUIRE(saga::any_cast<std::vector<int>>((static_cast<saga::any const*>(nullptr))) == nullptr);
}

TEMPLATE_LIST_TEST_CASE("any - value constructor", "any", Value_types_list)
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

TEMPLATE_LIST_TEST_CASE("any_cast mutable pointer", "any", Value_types_list)
{
    using Value = TestType;

    REQUIRE(saga::any_cast<Value>(static_cast<saga::any*>(nullptr)) == nullptr);

    saga_test::property_checker << [](Value const & value)
    {
        saga::any var(value);

        REQUIRE(var.has_value());
        REQUIRE(var.type() == typeid(Value));

        auto ptr = saga::any_cast<Value>(&var);
        static_assert(std::is_same<decltype(ptr), Value *>{}, "");

        REQUIRE(ptr != nullptr);
        REQUIRE(*ptr == value);
    };
}

TEST_CASE("any: copy constructor from empty")
{
    saga::any const empty_src;

    saga::any const obj(empty_src);

    REQUIRE(obj.has_value() == false);
    REQUIRE(obj.type() == typeid(void));
    REQUIRE(saga::any_cast<std::vector<int>>(&obj) == nullptr);
}

TEMPLATE_LIST_TEST_CASE("any: copy constructor from not empty", "any", Value_types_list)
{
    using Value = TestType;

    saga_test::property_checker << [](Value const & value)
    {
        saga::any const src(value);

        saga::any const obj(src);

        REQUIRE(obj.has_value());
        REQUIRE(obj.type() == src.type());
        REQUIRE(saga::any_cast<Value>(&obj) != nullptr);
        REQUIRE(*saga::any_cast<Value>(&obj) == value);
    };
}

TEST_CASE("any: copy assignment")
{
    using Value1 = std::string;
    using Value2 = long;

    saga_test::property_checker << [](Value1 const & value_1, Value2 const & value_2)
    {
        saga::any const src_0{};
        saga::any const src_1(value_1);
        saga::any const src_2(value_2);

        saga::any var;

        static_assert(std::is_same<decltype(var = src_0), saga::any &>{}, "");

        auto const & res1 = (var = src_0);

        REQUIRE(var.has_value() == false);
        REQUIRE(var.type() == typeid(void));
        REQUIRE(saga::any_cast<Value1>(&var) == nullptr);
        REQUIRE(saga::any_cast<Value2>(&var) == nullptr);
        REQUIRE(std::addressof(res1) == std::addressof(var));

        auto const & res2 = (var = src_1);

        REQUIRE(var.has_value());
        REQUIRE(var.type() == typeid(Value1));
        REQUIRE(saga::any_cast<Value1>(&var) != nullptr);
        REQUIRE(saga::any_cast<Value2>(&var) == nullptr);
        REQUIRE(*saga::any_cast<Value1>(&var) == value_1);
        REQUIRE(std::addressof(res2) == std::addressof(var));

        auto const & res3 = (var = src_2);

        REQUIRE(var.has_value());
        REQUIRE(var.type() == typeid(Value2));
        REQUIRE(saga::any_cast<Value1>(&var) == nullptr);
        REQUIRE(saga::any_cast<Value2>(&var) != nullptr);
        REQUIRE(*saga::any_cast<Value2>(&var) == value_2);
        REQUIRE(std::addressof(res3) == std::addressof(var));

        auto const & res4 = (var = src_0);

        REQUIRE(var.has_value() == false);
        REQUIRE(var.type() == typeid(void));
        REQUIRE(saga::any_cast<Value1>(&var) == nullptr);
        REQUIRE(saga::any_cast<Value2>(&var) == nullptr);
        REQUIRE(std::addressof(res4) == std::addressof(var));
    };
}

static_assert(std::is_base_of<std::bad_cast, saga::bad_any_cast>{}, "");

TEST_CASE("bad_any_cast")
{
    saga::bad_any_cast obj;

    REQUIRE(std::string(obj.what()).empty() == false);
}

TEMPLATE_LIST_TEST_CASE("any_cast: from empty", "any", Value_types_list)
{
    using Value = TestType;

    saga::any obj;
    REQUIRE(obj.has_value() == false);

    REQUIRE_THROWS_AS(saga::any_cast<Value>(obj), saga::bad_any_cast);
    REQUIRE_THROWS_AS(saga::any_cast<Value>(saga::as_const(obj)), saga::bad_any_cast);
    REQUIRE_THROWS_AS(saga::any_cast<Value>(std::move(obj)), saga::bad_any_cast);
}

TEMPLATE_LIST_TEST_CASE("any_cast: value and reference", "any", Value_types_list)
{
    using Value = TestType;

    saga_test::property_checker << [](Value const & old_value, Value const & new_value)
    {
        saga::any obj(old_value);

        static_assert(std::is_same<decltype(saga::any_cast<Value>(obj)), Value>{}, "");
        static_assert(std::is_same<decltype(saga::any_cast<Value&>(obj)), Value&>{}, "");
        static_assert(std::is_same<decltype(saga::any_cast<Value const&>(saga::as_const(obj)))
                                  , Value const&>{}, "");

        REQUIRE(saga::any_cast<Value>(obj) == old_value);

        saga::any_cast<Value&>(obj) = new_value;

        REQUIRE(saga::any_cast<Value const &>(saga::as_const(obj)) == new_value);
    };
}

TEST_CASE("any_cast: rvalue reference")
{
    using Container = std::vector<int>;

    saga_test::property_checker << [](Container const & value)
    {
        saga::any obj(value);

        auto res = saga::any_cast<Container>(std::move(obj));

        REQUIRE(res == value);
        REQUIRE(saga::any_cast<Container const &>(obj).empty());
    };
}

TEST_CASE("any: copy of non-const any")
{
    using Value = std::string;

    saga_test::property_checker << [](Value const & value)
    {
        saga::any src(value);

        saga::any const dest(src);

        REQUIRE(saga::any_cast<Value const &>(dest) == value);
    };
}

TEST_CASE("any: in_place_type ctor no args")
{
    using Value = int;
    using Container = std::vector<Value>;

    saga::any const obj(saga::in_place_type<Container>);
    REQUIRE(saga::any_cast<const Container&>(obj) == Container{});
}

static_assert(std::is_constructible<saga::any, saga::in_place_type_t<int>>{}, "");
static_assert(!std::is_convertible<saga::in_place_type_t<int>, saga::any>{}, "");

static_assert(!std::is_constructible<int, std::string>{}, "");
static_assert(!std::is_constructible<saga::any, saga::in_place_type_t<int>, std::string>{}, "");

TEST_CASE("any: in_place_type ctor with args")
{
    using Value = int;
    using Container = std::vector<Value>;

    saga_test::property_checker << [](saga_test::container_size<std::size_t> num
                                      , Value const & value)
    {
        saga::any const obj(saga::in_place_type<Container>, num, value);

        REQUIRE(saga::any_cast<const Container&>(obj) == Container(num, value));
    };
}

TEST_CASE("any: placement constructor with initializer list")
{
    saga_test::property_checker << [](int value1, int value2)
    {
        using Container = std::vector<int>;
        Container const expected{value1, value2};

        saga::any const actual(saga::in_place_type<Container>, {value1, value2});

        REQUIRE(saga::any_cast<Container const &>(actual) == expected);
    };
}

TEST_CASE("any: placement constructor with initializer list and more args")
{
    saga_test::property_checker << [](int value1, int value2)
    {
        using Compare = bool(*)(int const &, int const &);
        using Container = std::set<int, Compare>;

        auto const cmp = Compare([](int const & x, int const & y) { return x < y; });

        Container const expected({value1, value2}, cmp);

        saga::any const obj(saga::in_place_type<Container>, {value1, value2}, cmp);

        REQUIRE(saga::any_cast<Container const &>(obj) == expected);
    };
}

static_assert(!std::is_constructible<int, std::initializer_list<int>&>{}, "");
static_assert(!std::is_constructible<saga::any, saga::in_place_type_t<int>
                                              , std::initializer_list<int>&>{}, "");

TEST_CASE("make_any: no initializer_list")
{
    using Value = int;
    using Container = std::vector<Value>;

    saga_test::property_checker << [](saga_test::container_size<std::size_t> num
                                      , Value const & value)
    {
        auto obj = saga::make_any<Container>(num, value);

        static_assert(std::is_same<decltype(obj), saga::any>{}, "");

        REQUIRE(saga::any_cast<const Container&>(obj) == Container(num, value));
    };
}

TEST_CASE("make_any: with initializer list and more args")
{
    saga_test::property_checker << [](int value1, int value2)
    {
        using Compare = bool(*)(int const &, int const &);
        using Container = std::set<int, Compare>;

        auto const cmp = Compare([](int const & x, int const & y) { return x < y; });

        Container const expected({value1, value2}, cmp);

        auto obj = saga::make_any<Container>({value1, value2}, cmp);
        static_assert(std::is_same<decltype(obj), saga::any>{}, "");

        REQUIRE(saga::any_cast<Container const &>(obj) == expected);
    };
}
