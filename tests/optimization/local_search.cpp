#include <saga/optimization/local_search.hpp>

#include <catch/catch.hpp>

#include <saga/random/iid_distribution.hpp>
#include "../random_engine.hpp"

template <class Argument, class Objective, class RealType>
void test_local_search_boolean(Objective const & objective, typename Argument::size_type dim,
                               RealType desired_value, RealType eps = 0)
{
    saga::iid_distribution<std::bernoulli_distribution> init_distr(dim);

    for(auto N = 100; N > 0; -- N)
    {
        auto const x_init = init_distr(saga_test::random_engine());
        auto const x_result = saga::local_search_boolean(objective, x_init);

        CAPTURE(x_result);
        REQUIRE(objective(x_result) == Approx(desired_value).epsilon(eps));
    }
}

template <class Objective, class Argument>
bool is_local_maximum(Objective const & objective, Argument const & arg)
{
    auto const y_arg = objective(arg);
    auto x = arg;

    for(auto i = x.begin(); i != x.end(); ++ i)
    {
        *i = !*i;

        auto const y = objective(x);

        if(y > y_arg)
        {
            return false;
        }

        *i = !*i;
    }

    return true;
}

TEST_CASE("local search (pseudoboolen, first improvement) : L1 norm")
{

    using Argument = std::vector<bool>;
    auto const objective = [](Argument const & arg)
    {
        return std::count(arg.begin(), arg.end(), true);
    };

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

    test_local_search_boolean<Argument>(objective, dim, dim);
}

TEST_CASE("local search (pseudoboolean, first improvement): number of unequal adjacent bits")
{
    using Argument = std::vector<bool>;
    auto const objective = [](Argument const & arg)
    {
        auto result = arg.size()*0;

        for(auto i = 0*arg.size(); i+1 != arg.size(); ++ i)
        {
            result += (arg[i] != arg[i+1]);
        }

        return result;
    };

    auto const dim = 20;

    saga::iid_distribution<std::bernoulli_distribution> init_distr(dim);

    for(auto N = 100; N > 0; -- N)
    {
        auto const x_init = init_distr(saga_test::random_engine());
        auto const x_result = saga::local_search_boolean(objective, x_init);

        CAPTURE(x_result);

        REQUIRE(is_local_maximum(objective, x_result));
    }
}

TEST_CASE("local search (pseudoboolean, first improvement): number of unequal adjacent bits, mask")
{
    using Argument = std::vector<bool>;

    auto const dim = 20;

    saga::iid_distribution<std::bernoulli_distribution> mask_distr(dim);
    auto const mask = mask_distr(saga_test::random_engine());

    auto const objective = [&](Argument const & arg)
    {
        assert(arg.size() == mask.size());

        auto result = arg.size()*0;

        for(auto i = 0*arg.size(); i+1 != arg.size(); ++ i)
        {
            result += ((arg[i] ^ mask[i]) != (arg[i+1] ^ mask[i+1]));
        }

        return result;
    };

    saga::iid_distribution<std::bernoulli_distribution> init_distr(dim);

    for(auto N = 100; N > 0; -- N)
    {
        auto const x_init = init_distr(saga_test::random_engine());
        auto const x_result = saga::local_search_boolean(objective, x_init);

        CAPTURE(x_result);

        REQUIRE(is_local_maximum(objective, x_result));
    }
}
