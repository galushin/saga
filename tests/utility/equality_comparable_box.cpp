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

// Тестируемый заголовочный файл
#include <saga/utility/equality_comparable_box.hpp>

// Тестовая инфраструктура
#include "../saga_test.hpp"
#include <catch/catch.hpp>

// Вспомогательные файлы
#include <saga/type_traits.hpp>
#include <string>
#include <functional>

// Тесты
namespace
{
    template <class T>
    class without_equality
    {
        T value;
    };

    static_assert(!saga::is_equality_comparable<without_equality<int>>{}, "");
}

static_assert(std::is_same<saga::equality_comparable_box<int>::value_type, int>{}, "");
static_assert(std::is_same<saga::equality_comparable_box<std::string>::value_type, std::string>{}, "");
static_assert(std::is_same<saga::equality_comparable_box<std::greater<int>>::value_type,
                                                         std::greater<int>>{}, "");
static_assert(std::is_same<saga::equality_comparable_box<::without_equality<int>>::value_type,
                                                         ::without_equality<int>>{}, "");

namespace
{
    template <class Value>
    void check_equality_comparable_box_own_compare(Value const & value_1, Value const & value_2)
    {
        saga::equality_comparable_box<Value> const box_1(value_1);
        saga::equality_comparable_box<Value> const box_2(value_2);

        REQUIRE(box_1.value() == value_1);
        REQUIRE(box_2.value() == value_2);

        REQUIRE((box_1 == box_1));
        REQUIRE((box_2 == box_2));

        REQUIRE((box_1 == box_2) == (value_1 == value_2));
        REQUIRE((box_1 != box_2) == !(box_1 == box_2));
    }

    struct empty_with_equality
    {
        friend bool operator==(empty_with_equality const & lhs, empty_with_equality const & rhs)
        {
            return std::addressof(lhs) == std::addressof(rhs);
        }
    };
}

TEST_CASE("equality_comparable_box : with own equality")
{
    {
        using Value = int;
        constexpr auto const value_1 = Value(17);
        constexpr auto const value_2 = Value(42);

        constexpr saga::equality_comparable_box<Value> box_1(value_1);
        constexpr saga::equality_comparable_box<Value> box_2(value_2);

        static_assert(box_1.value() == value_1, "");
        static_assert(box_2.value() == value_2, "");

        static_assert(box_1 == box_1, "");
        static_assert(box_2 == box_2, "");

        static_assert(value_1 != value_2, "");
        static_assert(!(box_1 == box_2), "");
        static_assert(box_1 != box_2, "");
    }

    {
        constexpr saga::equality_comparable_box<empty_with_equality> const box_1{};
        constexpr saga::equality_comparable_box<empty_with_equality> const box_2{};

        REQUIRE((box_1 == box_1));
        REQUIRE((box_2 == box_2));
        REQUIRE(!(box_1 == box_2));
        REQUIRE((box_1 != box_2));
    }

    saga_test::property_checker
        << check_equality_comparable_box_own_compare<int>
        << check_equality_comparable_box_own_compare<std::string>;
}

namespace
{
    template <class Value>
    void check_equality_comparable_box_empty()
    {
        static_assert(std::is_empty<Value>{}, "");
        static_assert(!saga::is_equality_comparable<Value>{}, "");

        constexpr auto const value_1 = Value{};
        constexpr auto const value_2 = Value{};

        constexpr saga::equality_comparable_box<Value> box_1(value_1);
        constexpr saga::equality_comparable_box<Value> box_2(value_2);

        static_assert(box_1 == box_1, "");
        static_assert(box_2 == box_2, "");
        static_assert(box_1 == box_2, "");
        static_assert(!(box_1 != box_2), "");
    }
}

TEST_CASE("equality_comparable_box : empty")
{
    check_equality_comparable_box_empty<std::greater<>>();
    // @todo check_equality_comparable_box_empty<saga::equality_comparable_box<std::greater<>>>();
}

// @todo Пустые типы с оператором равно

// @todo Непустные типы без оператора ==
