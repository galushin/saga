#include <saga/random/iid_distribution.hpp>

#include <catch/catch.hpp>
#include "../random_engine.hpp"

TEST_CASE("iid_distribution<bernoully_distribution> simple sample")
{
    auto const dim = 20;

    saga::iid_distribution<std::bernoulli_distribution> distr(dim);

    auto const value = distr(saga_test::random_engine());

    using Value = std::remove_cv_t<decltype(value)>;

    static_assert(std::is_same<Value, std::vector<bool>>::value, "Must be same!");

    REQUIRE(value.size() == dim);
}

#include <valarray>

TEST_CASE("iid_distribution<bernoully_distribution, std::valarray>")
{
    auto const dim = 20;

    saga::iid_distribution<std::bernoulli_distribution, std::valarray> distr(dim);

    auto const value = distr(saga_test::random_engine());

    using Value = std::remove_cv_t<decltype(value)>;

    static_assert(std::is_same<Value, std::valarray<bool>>::value, "Must be same!");

    REQUIRE(value.size() == dim);
}
