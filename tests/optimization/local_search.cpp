/* (c) 2019-2021 Галушин Павел Викторович, galushin@gmail.com

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
#include <saga/optimization/local_search.hpp>

// Инфраструктура тестирования
#include "../saga_test.hpp"
#include <catch/catch.hpp>

// Вспомогательные возможноти
#include <saga/optimization/test_objectives.hpp>
#include <saga/random/iid_distribution.hpp>

namespace
{
    template <class Objective, class Argument>
    bool is_local_maximum_binary(Objective const & objective, Argument const & arg)
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
}

TEST_CASE("local search (pseudoboolen, first improvement) : L1 norm")
{
    saga_test::property_checker << [](std::vector<bool> const & x_init)
    {
        auto const objective = saga::boolean_manhattan_norm;

        auto const result = saga::local_search_boolean(objective, x_init);

        CAPTURE(result.solution);
        CAPTURE(result.objective_value);

        REQUIRE(result.objective_value == objective(result.solution));
        REQUIRE(result.objective_value == 0);
    };
}

TEST_CASE("local search (pseudoboolen, first improvement) : L1 distance to some vector")
{
    using Argument = std::vector<bool>;

    saga_test::property_checker << [](Argument x_opt, Argument x_init)
    {
        auto const dim = std::min(x_opt.size(), x_init.size());
        x_opt.resize(dim);
        x_init.resize(dim);

        auto const objective = [&x_opt](Argument const & arg)
        {
            return saga::boolean_manhattan_distance(arg, x_opt);
        };

        auto const result = saga::local_search_boolean(objective, x_init);

        CAPTURE(result.solution);
        CAPTURE(result.objective_value);

        REQUIRE(result.objective_value == objective(result.solution));
        REQUIRE(result.objective_value == 0);
    };
}

TEST_CASE("local search (pseudoboolean, first improvement): number of unequal adjacent bits")
{
    saga_test::property_checker << [](std::vector<bool> const & x_init)
    {
        auto const objective = saga::count_adjacent_unequal;

        auto const result = saga::local_search_boolean(objective, x_init, std::greater<>{});

        CAPTURE(result.solution);
        CAPTURE(result.objective_value);

        REQUIRE(is_local_maximum_binary(objective, result.solution));
    };
}

#include <list>

TEST_CASE("local search (pseudoboolean, first improvement): number of unequal adjacent bits, on list")
{
    saga_test::property_checker << [](std::list<bool> const & x_init)
    {
        auto const objective = saga::count_adjacent_unequal;

        auto const result = saga::local_search_boolean(objective, x_init, std::greater<>{});

        CAPTURE(result.solution);
        CAPTURE(result.objective_value);

        REQUIRE(is_local_maximum_binary(objective, result.solution));
    };
}

TEST_CASE("local search (pseudoboolean, first improvement): number of unequal adjacent bits, mask")
{
    using Argument = std::vector<bool>;

    saga_test::property_checker << [](Argument x_init, Argument mask)
    {
        auto const dim = std::min(x_init.size(), mask.size());
        x_init.resize(dim);
        mask.resize(dim);

        auto const objective = [&](Argument const & arg)
        {
            assert(arg.size() == mask.size());
            auto x = arg;

            for(auto i = 0*arg.size(); i+1 < arg.size(); ++ i)
            {
                x[i] = x[i] ^ mask[i];
            }

            return saga::count_adjacent_unequal(x);
        };

        auto const result = saga::local_search_boolean(objective, x_init, std::greater<>{});

        CAPTURE(result.solution);
        CAPTURE(result.objective_value);

        REQUIRE(is_local_maximum_binary(objective, result.solution));
    };
}

namespace
{
    template <class Argument, class SearchSpace>
    Argument random_point_of(SearchSpace const & space)
    {
        Argument result;
        std::transform(space.begin(), space.end(), std::back_inserter(result),
                       [](typename SearchSpace::value_type const & var)
                       {
                            return saga_test::random_uniform(var.min, var.max);
                       });

        return result;
    }
}

// @todo Тест должен влезать на один экран
TEST_CASE("local search (integer) : L1 norm minimization")
{
    using Integer = int;
    using Argument = std::vector<Integer>;

    saga_test::property_checker << [](saga_test::container_size<Argument::size_type> const dim)
    {
        auto const objective = saga::manhattan_norm;

        // Cлучайным образом выбираем область поиска
        saga::search_space_integer<Integer> space;

        for(auto num = dim.value; num > 0; --num)
        {
            auto const limit1 = saga_test::random_uniform(-20, 20);
            auto const limit2 = saga_test::random_uniform(-20, 20);

            auto const limits = std::minmax(limit1, limit2);

            space.add(limits.first, limits.second);
        }

        // Определяем точку оптимума
        Argument const x_opt = [&]()
        {
            // @todo Заменить на алгоритм
            Argument result;
            for(auto const & var : space)
            {
                // @todo Рефакторинг
                if(var.min > 0)
                {
                    result.push_back(var.min);
                }
                else if(var.max < 0)
                {
                    result.push_back(var.max);
                }
                else
                {
                    result.emplace_back(0);
                }
            }
            return result;
        }();

        auto const y_opt = objective(x_opt);

        // Случайным образом выбираем стартовый вектор
        auto const x_init = ::random_point_of<Argument>(space);

        // Выполняем локальный спуск
        auto const result = saga::local_search_integer(space, objective, x_init);

        // Проверяем результаты
        CAPTURE(x_init);

        REQUIRE(result.solution == x_opt);
        REQUIRE(result.objective_value == y_opt);
        REQUIRE(result.objective_value == objective(result.solution));
    };
}

// @todo Тест должен влезать на один экран
TEST_CASE("local search (integer) : L1 norm maximization")
{
    using Integer = int;
    using Argument = std::vector<Integer>;

    saga_test::property_checker << [](saga_test::container_size<Argument::size_type> const dim)
    {
        auto const objective = saga::manhattan_norm;

        // Cлучайным образом выбираем область поиска
        using SearchSpace = saga::search_space_integer<Integer>;
        SearchSpace space;

        for(auto num = dim.value; num > 0; --num)
        {
            // Чтобы задача была одноэкстремальной, границы не должны быть одного знака!
            auto lim1 = saga_test::random_uniform(0, 10);
            auto lim2 = lim1 + saga_test::random_uniform(0, 10);

            if(std::bernoulli_distribution(0.5)(saga_test::random_engine()))
            {
                space.add(lim1, lim2);
            }
            else
            {
                space.add(-lim2, -lim1);
            }
        }

        // Определяем точку оптимума
        Argument const x_opt = [&]()
        {
            // @todo Заменить на алгоритм
            Argument result;
            for(auto const & var : space)
            {
                using std::abs;
                result.emplace_back(abs(var.max) < abs(var.min) ? var.min : var.max);
            }
            return result;
        }();

        auto const y_opt = objective(x_opt);

        // Случайным образом выбираем стартовый вектор
        auto const x_init = ::random_point_of<Argument>(space);

        // Выполняем локальный спуск
        auto const result = saga::local_search_integer(space, objective, x_init, std::greater<>{});

        // Проверяем результаты
        CAPTURE(x_init);

        REQUIRE(result.solution == x_opt);
        REQUIRE(result.objective_value == y_opt);
        REQUIRE(result.objective_value == objective(result.solution));
    };
}
