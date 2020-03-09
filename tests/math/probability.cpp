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

#include <saga/math/probability.hpp>

#include <catch/catch.hpp>

TEST_CASE("probabilty : default initialization")
{
    saga::probability<double> p;

    static_assert(std::is_same<decltype(p.value()), double const &>::value, "Must be same!");

    REQUIRE_THAT(p.value(), Catch::WithinULP(0.0, 1));
}

TEST_CASE("probabilty : initialization with correct value")
{
    // @todo Тест на основе свойств
    auto const p_value = 0.7;

    saga::probability<double> p(p_value);

    REQUIRE_THAT(p.value(), Catch::WithinULP(p_value, 1));
}

TEST_CASE("probabilty : initialization with incorrect value")
{
    // @todo Тест на основе свойств
    using Probability = saga::probability<double, saga::probability_policy_throw>;

    CHECK_THROWS_AS(Probability(-0.1), std::logic_error);
    CHECK_THROWS_AS(Probability(1.1), std::logic_error);
}
