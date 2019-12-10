#include <saga/optimization/local_search.hpp>

#include <catch/catch.hpp>

#include <saga/random/iid_distribution.hpp>
#include "../random_engine.hpp"

template <class Argument, class Objective, class RealType>
void test_local_search_boolean(Objective const & objective, typename Argument::size_type dim,
                               RealType desired_value, RealType eps = 0)
{
    // @todo Через тестирование, основанное на свойствах
    saga::iid_distribution<std::bernoulli_distribution> init_distr(dim);

    for(auto N = 100; N > 0; -- N)
    {
        auto const x_init = init_distr(saga_test::random_engine());
        auto const x_result = saga::local_search_boolean(objective, x_init);

        CAPTURE(x_result);
        REQUIRE(objective(x_result) == Approx(desired_value).epsilon(eps));
    }
}

TEST_CASE("local search (pseudoboolen, first improvement) : L1 norm")
{

    using Argument = std::vector<bool>;
    auto const objective = [](Argument const & arg)
    {
        return std::count(arg.begin(), arg.end(), true);
    };

    // @todo Тестировать для разных размерностей?
    auto const dim = 20;
    test_local_search_boolean<Argument>(objective, dim, dim);
}

TEST_CASE("local search (pseudoboolen, first improvement) : L1 distance to some vector")
{
    using Argument = std::vector<bool>;

    auto const dim = 20;
    saga::iid_distribution<std::bernoulli_distribution> init_distr(dim);

    auto const x_opt = init_distr(saga_test::random_engine());
    auto const objective = [&x_opt](Argument const & arg)
    {
        assert(x_opt.size() == arg.size());

        return std::inner_product(arg.begin(), arg.end(), x_opt.begin(), 0*arg.size(),
                                  std::plus<>{}, std::not_equal_to<>{});
    };

    // @todo Тестировать для разных размерностей?
    test_local_search_boolean<Argument>(objective, dim, dim);
}
