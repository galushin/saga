#include <saga/optimization/local_search.hpp>

#include <catch/catch.hpp>

#include <saga/random/iid_distribution.hpp>
#include "../random_engine.hpp"

TEST_CASE("local search (pseudoboolen, first improvement)")
{
    auto const dim = 20;

    using Argument = std::vector<bool>;
    auto const objective = [](Argument const & arg)
    {
        return std::count(arg.begin(), arg.end(), true);
    };

    saga::iid_distribution<std::bernoulli_distribution> init_distr(dim);

    // @todo Через тестирование, основанное на свойствах
    for(auto N = 100; N > 0; -- N)
    {
        auto const x_init = init_distr(saga_test::random_engine());
        auto const x_result = saga::local_search_boolean(objective, x_init);

        CAPTURE(x_result);
        REQUIRE(objective(x_result) == dim);
    }
}
