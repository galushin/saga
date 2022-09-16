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

// Тестируемый файл
#include <saga/random/any_distribution.hpp>

// Тестовая инфраструктура
#include "../saga_test.hpp"
#include <catch2/catch_amalgamated.hpp>

// Используются в тестах
#include <saga/algorithm.hpp>

// Тесты
TEST_CASE("any_distribution : types")
{
    using Result = int;
    using Random_engine = std::minstd_rand0;

    using Distribution = saga::any_distribution<Result, Random_engine>;

    static_assert(std::is_copy_constructible<Distribution>{}, "Must be CopyConstructible");
    static_assert(std::is_copy_assignable<Distribution>{}, "Must be CopyAssignable");

    static_assert(std::is_same<Distribution::result_type, Result>{}, "Must be same");
    static_assert(std::is_same<Distribution::engine_type, Random_engine>{}, "Must be same");
    // К параметру распределение вернёмся позже
}

TEST_CASE("any_distribution : constructor")
{
    using Distribution = std::uniform_int_distribution<int>;
    Distribution distr(2, 12);

    using Random_engine = std::minstd_rand0;
    using AnyDistribution = saga::any_distribution<Distribution::result_type, Random_engine>;

    AnyDistribution a_distr(distr);

    // Одинаковые свойства
    CHECK(a_distr.min() == distr.min());
    CHECK(a_distr.max() == distr.max());

    // Порождает одинаковые значения
    auto const seed = 20042020;
    Random_engine rnd1(seed);
    Random_engine rnd2(seed);

    saga::for_n(100, [&] { REQUIRE(a_distr(rnd1) == distr(rnd2)); });
}

TEST_CASE("any_distribution : equality")
{
    using Result = int;

    using D_1 = std::uniform_int_distribution<Result>;
    using D_2 = std::geometric_distribution<Result>;

    static_assert(!std::is_same<D_1, D_2>{}, "Must be different");

    using Random_engine = std::minstd_rand0;

    using AnyDistribution = saga::any_distribution<Result, Random_engine>;

    D_1 d_1(2, 13);
    D_1 d_2(-5, 12);
    D_2 d_3(0.7);
    D_2 d_4(d_3);

    REQUIRE(d_1 != d_2);
    REQUIRE(d_3 == d_4);

    AnyDistribution ad_1(d_1);
    AnyDistribution ad_2(d_2);
    AnyDistribution ad_3(d_3);
    AnyDistribution ad_4(d_4);

    CHECK(ad_1 == ad_1);
    CHECK(ad_1 != ad_2);
    CHECK(ad_1 != ad_3);
    CHECK(ad_1 != ad_4);

    CHECK(ad_1 != ad_2);
    CHECK(ad_2 == ad_2);
    CHECK(ad_2 != ad_3);
    CHECK(ad_2 != ad_4);

    CHECK(ad_3 != ad_1);
    CHECK(ad_3 != ad_2);
    CHECK(ad_3 == ad_3);
    CHECK(ad_3 == ad_4);

    CHECK(ad_4 != ad_1);
    CHECK(ad_4 != ad_2);
    CHECK(ad_4 == ad_3);
    CHECK(ad_4 == ad_4);
}

TEST_CASE("any_distribution : copy constructor")
{
    using Result = int;

    using Distribution = std::uniform_int_distribution<Result>;
    using Random_engine = std::minstd_rand0;

    using AnyDistribution = saga::any_distribution<Result, Random_engine>;

    Distribution distr(12, 3);
    AnyDistribution ad_1(distr);
    AnyDistribution ad_2(ad_1);

    CHECK(ad_1 == ad_2);
}

TEST_CASE("any_distribution : assignment")
{
    using Result = int;

    using D_1 = std::uniform_int_distribution<Result>;
    using D_2 = std::geometric_distribution<Result>;

    static_assert(!std::is_same<D_1, D_2>{}, "Must be different");

    using Random_engine = std::minstd_rand0;

    using AnyDistribution = saga::any_distribution<Result, Random_engine>;

    D_1 d_1(2, 13);
    D_2 d_2(0.7);

    AnyDistribution ad_1(d_1);
    AnyDistribution ad_2(d_2);

    REQUIRE(ad_1 != ad_2);

    ad_1 = ad_2;

    REQUIRE(ad_1 == ad_2);
}
