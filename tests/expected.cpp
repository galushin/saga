/* (c) 2020 Галушин Павел Викторович, galushin@gmail.com

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
#include <saga/expected.hpp>

// Тестовая инфраструктура
#include "./saga_test.hpp"
#include <catch/catch.hpp>
#include <type_traits>

// Вспомогательне файлы
#include <stdexcept>
#include <set>

// Тесты

// @todo 5. Шаблон класса unexpected
static_assert(std::is_copy_constructible<saga::unexpected<int>>{}, "");
static_assert(std::is_move_constructible<saga::unexpected<int>>{}, "");

// Есть конструктор с in_place_t и любым числом аргументов
static_assert(std::is_constructible<std::vector<int>, std::size_t, int>{}, "");
static_assert(std::is_constructible<saga::unexpected<std::vector<int>>,
                                    saga::in_place_t, std::size_t, int>{}, "");

// Конструктор с in_place_t является explicit:
static_assert(std::is_constructible<std::vector<int>>{}, "");
static_assert(std::is_constructible<saga::unexpected<std::vector<int>>, saga::in_place_t>{}, "");
static_assert(!std::is_convertible<saga::in_place_t, saga::unexpected<std::vector<int>>>{}, "");

// Конструктор с in_place_t не участвует в разрешение перегрузки, когда его нельзя вызывать
static_assert(!std::is_constructible<int, std::vector<int>>{}, "");
static_assert(!std::is_constructible<saga::unexpected<int>,
                                     saga::in_place_t, std::vector<int>>{}, "");

static_assert(!std::is_constructible<int, std::initializer_list<int>&, int>{}, "");
static_assert(!std::is_constructible<saga::unexpected<int>,
                                     saga::in_place_t, std::initializer_list<int>&, int>{}, "");

TEST_CASE("unexpected: placement constructor")
{
    {
        constexpr saga::unexpected<int> err0(saga::in_place_t{});

        static_assert(err0.value() == 0, "");
        static_assert(noexcept(err0.value()), "");

        constexpr int value = 42;
        constexpr saga::unexpected<int> err1(saga::in_place_t{}, value);
        static_assert(err1.value() == value, "");
    }

    saga_test::property_checker << [](saga_test::container_size<std::size_t> num, int filler)
    {
        std::vector<int> const expected(num, filler);

        saga::unexpected<std::vector<int>> const actual(saga::in_place_t{}, num, filler);

        REQUIRE(actual.value() == expected);
    };
}

TEST_CASE("unexpected: placement constructor with initializer list")
{
    saga_test::property_checker << [](int value1, int value2)
    {
        std::vector<int> const expected{value1, value2};

        saga::unexpected<std::vector<int>> const actual(saga::in_place_t{}, {value1, value2});

        REQUIRE(actual.value() == expected);
    };
}

TEST_CASE("unexpected: placement constructor with initializer list and more args")
{
    {
        struct initializer_list_consumer
        {
            constexpr initializer_list_consumer(std::initializer_list<int> inits, int arg)
             : value(arg)
            {
                for(auto const & each : inits)
                {
                    value += each;
                }
            }

            int value = 0;
        };

        constexpr saga::unexpected<initializer_list_consumer> unex(saga::in_place_t{}, {1, 2, 3, 4}, 5);
    }

    saga_test::property_checker << [](int value1, int value2)
    {
        using Compare = bool(*)(int const &, int const &);
        using Container = std::set<int, Compare>;

        auto const cmp = Compare([](int const & x, int const & y) { return x < y; });

        Container const expected({value1, value2}, cmp);

        saga::unexpected<Container> const actual(saga::in_place_t{}, {value1, value2}, cmp);

        REQUIRE(actual.value() == expected);
    };
}

TEST_CASE("unexpected : copy constructor")
{
    {
        constexpr int value = 42;
        constexpr saga::unexpected<int> err1(saga::in_place_t{}, value);
        static_assert(err1.value() == value, "");

        constexpr saga::unexpected<int> err2(err1);

        static_assert(err1.value() == err2.value(), "");
    }

    saga_test::property_checker << [](saga_test::container_size<std::size_t> num, int filler)
    {
        saga::unexpected<std::vector<int>> const obj1(saga::in_place_t{}, num, filler);

        saga::unexpected<std::vector<int>> const obj2(obj1);

        REQUIRE(obj1.value() == obj2.value());
    };
}

// 6. bad_expected_access
namespace
{
    static_assert(std::is_base_of<saga::bad_expected_access<void>,
                                  saga::bad_expected_access<int>>{}, "");
    static_assert(std::is_base_of<saga::bad_expected_access<void>,
                                  saga::bad_expected_access<std::string>>{}, "");
    static_assert(std::is_convertible<saga::bad_expected_access<int> *,
                                      saga::bad_expected_access<void> *>{}, "");
    static_assert(std::is_convertible<saga::bad_expected_access<std::string> *,
                                      saga::bad_expected_access<void> *>{}, "");
    static_assert(std::is_constructible<saga::bad_expected_access<int>, int>{}, "");
    static_assert(!std::is_convertible<int, saga::bad_expected_access<int>>{}, "");
    static_assert(std::is_constructible<saga::bad_expected_access<std::string>, std::string>{}, "");
    static_assert(!std::is_convertible<std::string, saga::bad_expected_access<std::string>>{}, "");
}

TEST_CASE("bad_expected_access<std::string>")
{
    using Value = std::string;
    saga_test::property_checker << [](Value const & value)
    {
        using Exception = saga::bad_expected_access<Value>;
        Exception exc(value);

        REQUIRE_THAT(std::string(exc.what()), Catch::Matchers::Contains("expected"));

        Exception const & c_ref = exc;

        static_assert(std::is_same<decltype(exc.error()), Value &>{}, "");
        static_assert(std::is_same<decltype(c_ref.error()), Value const &>{}, "");
        static_assert(std::is_same<decltype(std::move(exc).error()), Value &&>{}, "");

        REQUIRE(exc.error() == value);
        REQUIRE(c_ref.error() == value);
        REQUIRE(std::addressof(c_ref.error()) == std::addressof(exc.error()));

        auto const old_value = exc.error();

        auto sink = std::move(exc).error();

        REQUIRE(sink == old_value);
    };
}

TEST_CASE("bad_expected_access<std::string> const &&")
{
    using Value = std::string;
    saga_test::property_checker << [](Value const & value)
    {
        using Exception = saga::bad_expected_access<Value>;
        Exception const exc(value);

        static_assert(std::is_same<decltype(std::move(exc).error()), Value const &&>{}, "");

        REQUIRE(exc.error() == value);

        Value const && sink = std::move(exc).error();

        REQUIRE(std::addressof(sink) == std::addressof(exc.error()));
    };
}

// 7. bad_expected_access<void>
namespace
{
    static_assert(std::is_base_of<std::exception, saga::bad_expected_access<void>>{}, "");
    static_assert(std::is_convertible<saga::bad_expected_access<void> *, std::exception *>{}, "");

    static_assert(std::is_default_constructible<saga::bad_expected_access<void>>{}, "");
    static_assert(std::is_nothrow_default_constructible<saga::bad_expected_access<void>>{}, "");
}

TEST_CASE("bad_expected_access<void>")
{
    saga::bad_expected_access<void> const exc{};

    REQUIRE(std::string(exc.what()) == std::string(std::exception().what()));
}

// 8. Тэг unexpect
namespace
{
    static_assert(std::is_trivially_default_constructible<saga::unexpect_t>{}, "");
    static_assert(std::is_nothrow_default_constructible<saga::unexpect_t>{}, "");

    constexpr auto tag1 = saga::unexpect_t{};
    constexpr auto tag2 = saga::unexpect;

    static_assert(std::is_same<decltype(tag1), saga::unexpect_t const>{}, "");
    static_assert(std::is_same<decltype(tag2), saga::unexpect_t const>{}, "");
}
