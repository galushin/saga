/* (c) 2019 Галушин Павел Викторович, galushin@gmail.com

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

#include <saga/random/iid_distribution.hpp>

#include <catch/catch.hpp>

#include "../saga_test.hpp"
#include "../random_engine.hpp"

#include <random>

TEST_CASE("iid_distribution<bernoully_distribution> simple sample")
{
    saga_test::property_checker << [](saga_test::container_size<int> const dim)
    {
        saga::iid_distribution<std::bernoulli_distribution> distr(dim);

        auto const value = distr(saga_test::random_engine());

        using Value = std::remove_cv_t<decltype(value)>;

        static_assert(std::is_same<Value, std::vector<bool>>::value, "Must be same!");

        REQUIRE(value.size() == dim);
    };
}

#include <valarray>

TEST_CASE("iid_distribution<bernoully_distribution, std::valarray>")
{
    saga_test::property_checker << [](saga_test::container_size<int> const dim)
    {
        saga::iid_distribution<std::bernoulli_distribution, std::valarray<bool>> distr(dim);

        auto const value = distr(saga_test::random_engine());

        using Value = std::remove_cv_t<decltype(value)>;

        static_assert(std::is_same<Value, std::valarray<bool>>::value, "Must be same!");

        REQUIRE(value.size() == dim);
    };
}

TEST_CASE("iid_distribution<normal_distribution>, std::valarray>")
{
    saga_test::property_checker
    << [](saga_test::container_size<int> const dim)
    {
        // @todo Подумать, как перенести это в параметры свойства
        auto const mean = saga_test::random_uniform(-10.0, 10.0);
        auto const sigma = saga_test::random_uniform(0.1, 10.0);

        using Normal = std::normal_distribution<double>;
        Normal unit_distr(mean, sigma);

        saga::iid_distribution<Normal, std::valarray<double>> distr(dim, unit_distr);

        auto const value = distr(saga_test::random_engine());

        using Value = std::remove_cv_t<decltype(value)>;

        static_assert(std::is_same<Value, std::valarray<double>>::value, "Must be same!");

        REQUIRE(value.size() == dim);

        REQUIRE(distr.dim() == dim);

        REQUIRE_THAT(distr.component_distribution().mean(), Catch::WithinULP(unit_distr.mean(), 1));
        REQUIRE_THAT(distr.component_distribution().stddev(), Catch::WithinULP(unit_distr.stddev(), 1));
    };

    // @todo Статистический тест распределения?
}
