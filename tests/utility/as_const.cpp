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
#include <saga/utility/as_const.hpp>

// Тестовая инфраструктура
#include "../saga_test.hpp"
#include <catch/catch.hpp>

// Вспомогательные файлы

// Тесты
TEST_CASE("as_const")
{
    using Type = int;

    {
        constexpr Type const obj = 42;

        static_assert(obj == saga::as_const(obj), "");
    }

    saga_test::property_checker <<
    [](Type obj)
    {
        static_assert(!std::is_const<Type>{}, "");

        auto && c_obj = saga::as_const(obj);

        static_assert(noexcept(saga::as_const(obj)));
        static_assert(std::is_same<decltype(c_obj), Type const &>{}, "");

        REQUIRE(std::addressof(c_obj) == std::addressof(obj));

        auto && cc_obj = saga::as_const(c_obj);
        REQUIRE(std::addressof(cc_obj) == std::addressof(obj));
    };
}

namespace
{
    template <class T, class SFINAE = void>
    struct as_const_is_applicable
     : std::false_type
    {};

    template <class T>
    struct as_const_is_applicable<T, saga::void_t<decltype(::saga::as_const(std::declval<T>()))>>
     : std::true_type
    {};
}

static_assert(!::as_const_is_applicable<double>{}, "");
static_assert(::as_const_is_applicable<double &>{}, "");
static_assert(::as_const_is_applicable<double const &>{}, "");
