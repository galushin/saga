/* (c) 2021 Галушин Павел Викторович, galushin@gmail.com

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
#include <saga/numeric.hpp>

// Тестовая инфраструктура
#include "saga_test.hpp"
#include <catch/catch.hpp>

// Вспомогательные возможности, используемые в тестах
#include <saga/cursor/subrange.hpp>
#include <saga/cursor/istream_cursor.hpp>

#include <list>
#include <vector>

// Тесты
TEST_CASE("iota")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src, Value const & init_value)
    {
        // saga
        auto src_saga = src;
        auto const cur = saga_test::random_subcursor_of(saga::cursor::all(src_saga));

        saga::iota(cur, init_value);

        // std
        auto src_std = src;

        std::iota(src_std.begin() + (cur.begin() - src_saga.begin())
                  , src_std.begin() + (cur.end() - src_saga.begin())
                  , init_value);

        // Сравнение
        REQUIRE(src_saga == src_std);
    };
}

// accumulate
TEST_CASE("accumulate - default operation")
{
    using Element = unsigned int;
    using InitValue = unsigned long;

    static_assert(!std::is_same<Element, InitValue>{}, "");

    saga_test::property_checker << [](std::vector<Element> container,
                                      InitValue const & init_value)
    {
        auto const values = saga_test::random_subcursor_of(saga::cursor::all(container));

        auto const expected = std::accumulate(values.begin(), values.end(), init_value);

        auto const actual  = saga::accumulate(saga::cursor::all(values), init_value);

        REQUIRE(actual == expected);

        static_assert(std::is_same<InitValue const, decltype(actual)>{}, "");
    };
}

TEST_CASE("accumulate - generic operation")
{
    using Element = unsigned int;
    using InitValue = unsigned long;

    static_assert(!std::is_same<Element, InitValue>{}, "");

    saga_test::property_checker << [](std::vector<Element> const & container,
                                      InitValue const & init_value)
    {
        auto const values = saga_test::random_subcursor_of(saga::cursor::all(container));

        auto const min_op = [](InitValue const & lhs, InitValue const & rhs)
        {
            return std::min(lhs, rhs);
        };

        auto const expected = std::accumulate(values.begin(), values.end(), init_value, min_op);

        auto input = saga_test::make_istringstream_from_range(values);

        auto const actual
            = saga::accumulate(saga::make_istream_cursor<Element>(input), init_value, min_op);

        REQUIRE(actual == expected);

        static_assert(std::is_same<InitValue const, decltype(actual)>{}, "");
    };
}

TEST_CASE("accumulate - constexpr")
{
    constexpr int values[] = {1, 2, 3, 4, 5};

    constexpr auto const sum = saga::accumulate(saga::cursor::all(values), 1, std::multiplies<>{});

    static_assert(sum == 120, "");
}

TEST_CASE("accumulate - move only init value")
{
    saga_test::property_checker << [](std::vector<int> values, int const & init_value)
    {
        auto const fun = std::bit_xor<>{};

        auto const result_std = std::accumulate(values.begin(), values.end(), init_value, fun);

        auto const my_op = [=](saga_test::move_only<int> lhs, int rhs)
        {
            lhs.value = fun(lhs.value, rhs);
            return lhs;
        };

        auto const result_saga = saga::accumulate(saga::cursor::all(values),
                                                  saga_test::move_only<int>(init_value), my_op);

        REQUIRE(result_saga.value == result_std);
    };
}

// partial sum
TEST_CASE("partial_sum - minimalistic, default operation")
{
    using Value = unsigned;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        // saga
        auto src_in = saga_test::make_istringstream_from_range(src);

        std::vector<Value> dest_saga;
        saga::partial_sum(saga::make_istream_cursor<Value>(src_in), saga::back_inserter(dest_saga));

        // std
        std::vector<Value> dest_std;
        std::partial_sum(src.begin(), src.end(), std::back_inserter(dest_std));

        // Сравнение
        REQUIRE(dest_saga == dest_std);
    };
}

TEST_CASE("partial_sum - subcursor, default operation")
{
    using Value = unsigned;

    saga_test::property_checker
    << [](std::vector<Value> const & src, std::vector<Value> const & dest_old)
    {
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto dest_saga = dest_old;

        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto const result_saga = saga::partial_sum(input, out_saga);

        // std
        auto dest_std = dest_old;

        auto const result_std
            = std::partial_sum(input.begin(), result_saga.in.begin()
                               , dest_std.begin() + (out_saga.begin() - dest_saga.begin()));

        // Сравнение
        REQUIRE(dest_saga == dest_std);
        REQUIRE((result_saga.out.begin() - dest_saga.begin()) == (result_std - dest_std.begin()));
        REQUIRE(result_saga.out.end() == out_saga.end());
    };
}

TEST_CASE("partial_sum - minimalistic, custom operation")
{
    using Value = unsigned;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto const op = std::multiplies<>{};

        // std
        std::vector<Value> dest_std;
        std::partial_sum(src.begin(), src.end(), std::back_inserter(dest_std), op);

        // saga
        auto src_in = saga_test::make_istringstream_from_range(src);

        std::vector<Value> dest_saga;
        saga::partial_sum(saga::make_istream_cursor<Value>(src_in)
                          , saga::back_inserter(dest_saga), op);

        // Сравнение
        REQUIRE(dest_saga == dest_std);
    };
}

TEST_CASE("partial_sum - subcursor, custom operation")
{
    using Value = unsigned;

    saga_test::property_checker
    << [](std::vector<Value> const & src, std::vector<Value> const & dest_old)
    {
        auto const op = std::multiplies<>{};

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto dest_saga = dest_old;

        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto const result_saga = saga::partial_sum(input, out_saga, op);

        // std
        auto dest_std = dest_old;

        auto const result_std
            = std::partial_sum(input.begin(), result_saga.in.begin()
                               , dest_std.begin() + (out_saga.begin() - dest_saga.begin()), op);

        // Сравнение
        REQUIRE(dest_saga == dest_std);
        REQUIRE((result_saga.out.begin() - dest_saga.begin()) == (result_std - dest_std.begin()));
        REQUIRE(result_saga.out.end() == out_saga.end());
    };
}

// reduce
TEST_CASE("reduce - general, minimalistic")
{
    using Value = unsigned;

    saga_test::property_checker << [](std::vector<Value> const & src, Value const & init_value)
    {
        auto src_in = saga_test::make_istringstream_from_range(src);

        auto const bin_op = std::bit_xor<>{};

        REQUIRE(saga::reduce(saga::make_istream_cursor<Value>(src_in), init_value, bin_op)
                == saga::accumulate(saga::cursor::all(src), init_value, bin_op));

    };
}

TEST_CASE("reduce - general, subrange")
{
    using Value = unsigned;

    saga_test::property_checker << [](std::vector<Value> const & src, Value const & init_value)
    {
        auto const cur = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const bin_op = std::bit_xor<>{};

        REQUIRE(saga::reduce(cur, init_value, bin_op)
                == saga::accumulate(cur, init_value, bin_op));

    };
}

TEST_CASE("reduce - default operation, minimalistic")
{
    using Value = unsigned;

    saga_test::property_checker << [](std::vector<Value> const & src, Value const & init_value)
    {
        auto src_in = saga_test::make_istringstream_from_range(src);

        REQUIRE(saga::reduce(saga::make_istream_cursor<Value>(src_in), init_value)
                == saga::reduce(saga::cursor::all(src), init_value, std::plus<>{}));

    };
}

TEST_CASE("reduce - default operation, subrange")
{
    using Value = unsigned;

    saga_test::property_checker << [](std::vector<Value> const & src, Value const & init_value)
    {
        auto const cur = saga_test::random_subcursor_of(saga::cursor::all(src));

        REQUIRE(saga::reduce(cur, init_value)
                == saga::reduce(cur, init_value, std::plus<>{}));

    };
}

TEST_CASE("reduce - default init value and operation, minimalistic")
{
    using Value = unsigned;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto src_in = saga_test::make_istringstream_from_range(src);

        REQUIRE(saga::reduce(saga::make_istream_cursor<Value>(src_in))
                == saga::reduce(saga::cursor::all(src), Value{}, std::plus<>{}));

    };
}

TEST_CASE("reduce - default init value and operation, subrange")
{
    using Value = unsigned;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto const cur = saga_test::random_subcursor_of(saga::cursor::all(src));

        REQUIRE(saga::reduce(cur) == saga::reduce(cur, Value{}, std::plus<>{}));

    };
}

// inner_product
TEST_CASE("inner_product: default operations")
{
    using Value = unsigned int;

    saga_test::property_checker << [](std::vector<Value> lhs,
                                      std::list<bool> const & rhs,
                                      Value const & init_value)
    {
        auto const n_min = std::min(lhs.size(), rhs.size());

        auto const result_std
            = std::inner_product(lhs.begin(), lhs.begin() + n_min, rhs.begin(), init_value);

        auto const result_saga
            = saga::inner_product(saga::cursor::all(lhs), saga::cursor::all(rhs), init_value);

        static_assert(std::is_same<decltype(result_saga), Value const>{}, "");

        REQUIRE(result_saga == result_std);
    };
}

TEST_CASE("inner_product: generic operations")
{
    using Value = unsigned int;

    saga_test::property_checker << [](std::vector<Value> const & lhs,
                                      std::list<Value> const & rhs,
                                      Value const & init_value)
    {
        auto op1 = [](Value const & x, Value const & y)
        {
            return std::min(x, y);
        };

        auto op2 = [](Value const & x, Value const & y)
        {
            return std::max(x, y);
        };

        auto const n_min = std::min(lhs.size(), rhs.size());

        auto const result_std = std::inner_product(lhs.begin(), lhs.begin() + n_min, rhs.begin(),
                                                   init_value, op1, op2);

        auto const result_saga = saga::inner_product(saga::cursor::all(lhs), saga::cursor::all(rhs),
                                                     init_value, op1, op2);

        static_assert(std::is_same<decltype(result_saga), Value const>{}, "");

        REQUIRE(result_saga == result_std);
    };
}

TEST_CASE("inner_product - move only init value")
{
    saga_test::property_checker << [](std::vector<int> values,
                                      std::list<bool> const & mask,
                                      int const & init_value)
    {
        auto const fun = std::bit_xor<>{};

        auto const n_min = std::min(values.size(), mask.size());
        auto const result_std = std::inner_product(values.begin(), values.begin() + n_min
                                                  , mask.begin(), init_value
                                                  , fun, std::multiplies<>{});

        auto const my_plus = [=](saga_test::move_only<int> lhs, int rhs)
        {
            lhs.value = fun(lhs.value, rhs);
            return lhs;
        };

        auto const result_saga = saga::inner_product(saga::cursor::all(values),
                                                     saga::cursor::all(mask),
                                                     saga_test::move_only<int>(init_value),
                                                     my_plus, std::multiplies<>{});

        REQUIRE(result_saga.value == result_std);
    };
}

TEST_CASE("inner_product - constexpr, generic")
{
    constexpr int values[] = {1, 2, 3, 4, 5};

    constexpr auto const sum = saga::inner_product(saga::cursor::all(values),
                                                   saga::cursor::all(values), 0);

    static_assert(sum == 55, "");
}
