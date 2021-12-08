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
#include <saga/algorithm.hpp>
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

// adjacent_difference
TEST_CASE("adjacent_difference - minimalistic, default operation")
{
    using Value = unsigned;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        // saga
        auto src_in = saga_test::make_istringstream_from_range(src);

        std::vector<Value> dest_saga;
        saga::adjacent_difference(saga::make_istream_cursor<Value>(src_in)
                                  , saga::back_inserter(dest_saga));

        // std
        std::vector<Value> dest_std;
        std::adjacent_difference(src.begin(), src.end(), std::back_inserter(dest_std));

        // Сравнение
        REQUIRE(dest_saga == dest_std);
    };
}

TEST_CASE("adjacent_difference - subcursor, default operation")
{
    using Value = unsigned;

    saga_test::property_checker
    << [](std::vector<Value> const & src, std::vector<Value> const & dest_old)
    {
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto dest_saga = dest_old;

        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto const result_saga = saga::adjacent_difference(input, out_saga);

        // std
        auto dest_std = dest_old;

        auto const result_std
            = std::adjacent_difference(input.begin(), result_saga.in.begin()
                                       , dest_std.begin() + (out_saga.begin() - dest_saga.begin()));

        // Сравнение
        REQUIRE(dest_saga == dest_std);
        REQUIRE((result_saga.out.begin() - dest_saga.begin()) == (result_std - dest_std.begin()));
        REQUIRE(result_saga.out.end() == out_saga.end());
    };
}

TEST_CASE("adjacent_difference - minimalistic, custom operation")
{
    using Value = unsigned;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto const op = std::plus<>{};

        // std
        std::vector<Value> dest_std;
        std::adjacent_difference(src.begin(), src.end(), std::back_inserter(dest_std), op);

        // saga
        auto src_in = saga_test::make_istringstream_from_range(src);

        std::vector<Value> dest_saga;
        saga::adjacent_difference(saga::make_istream_cursor<Value>(src_in)
                                  , saga::back_inserter(dest_saga), op);

        // Сравнение
        REQUIRE(dest_saga == dest_std);
    };
}

TEST_CASE("adjacent_difference - subcursor, custom operation")
{
    using Value = unsigned;

    saga_test::property_checker
    << [](std::vector<Value> const & src, std::vector<Value> const & dest_old)
    {
        auto const op = std::plus<>{};

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto dest_saga = dest_old;

        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto const result_saga = saga::adjacent_difference(input, out_saga, op);

        // std
        auto dest_std = dest_old;

        auto const result_std
            = std::adjacent_difference(input.begin(), result_saga.in.begin()
                                       , dest_std.begin() + (out_saga.begin() - dest_saga.begin()), op);

        // Сравнение
        REQUIRE(dest_saga == dest_std);
        REQUIRE((result_saga.out.begin() - dest_saga.begin()) == (result_std - dest_std.begin()));
        REQUIRE(result_saga.out.end() == out_saga.end());
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

// inclusive_scan
TEST_CASE("inclusive_scan - minimalistic, default operation")
{
    using Value = unsigned;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        // saga
        auto src_in = saga_test::make_istringstream_from_range(src);

        std::vector<Value> dest_saga;
        saga::inclusive_scan(saga::make_istream_cursor<Value>(src_in)
                             , saga::back_inserter(dest_saga));

        // std
        std::vector<Value> dest_std;
        std::partial_sum(src.begin(), src.end(), std::back_inserter(dest_std));

        // Сравнение
        REQUIRE(dest_saga == dest_std);
    };
}

TEST_CASE("inclusive_scan - subcursor, default operation")
{
    using Value = unsigned;

    saga_test::property_checker
    << [](std::vector<Value> const & src, std::vector<Value> const & dest_old)
    {
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto dest_saga = dest_old;

        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto const result_saga = saga::inclusive_scan(input, out_saga);

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

TEST_CASE("inclusive_scan - minimalistic, custom operation")
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
        saga::inclusive_scan(saga::make_istream_cursor<Value>(src_in)
                             , saga::back_inserter(dest_saga), op);

        // Сравнение
        REQUIRE(dest_saga == dest_std);
    };
}

TEST_CASE("inclusive_scan - subcursor, custom operation")
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

        auto const result_saga = saga::inclusive_scan(input, out_saga, op);

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

TEST_CASE("inclusive_scan - minimalistic, custom operation, init value")
{
    using Value = unsigned;

    saga_test::property_checker << [](std::list<Value> const & src, Value const & init_value)
    {
        auto const op = std::multiplies<>{};

        // Ожидаемое
        auto const src_2 = [&]
        {
            auto tmp = src;

            if(!tmp.empty())
            {
                tmp.front() = op(tmp.front(), init_value);
            }

            return tmp;
        }();

        std::vector<Value> dest_expected;
        saga::inclusive_scan(saga::cursor::all(src_2), saga::back_inserter(dest_expected), op);

        // saga
        auto src_in = saga_test::make_istringstream_from_range(src);

        std::vector<Value> dest_saga;
        saga::inclusive_scan(saga::make_istream_cursor<Value>(src_in)
                             , saga::back_inserter(dest_saga), op, init_value);

        // Сравнение
        CAPTURE(src, init_value, src_2);

        REQUIRE(dest_saga == dest_expected);
    };
}

TEST_CASE("inclusive_scan - subcursor, custom operation, init_value")
{
    using Value = unsigned;

    saga_test::property_checker
        << [](std::vector<Value> const & src
              , std::vector<Value> const & dest_old
              , Value const & init_value)
    {
        auto const op = std::multiplies<>{};

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // С начальным значением
        auto dest_saga = dest_old;

        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto const result_saga = saga::inclusive_scan(input, out_saga, op, init_value);

        // Без начального значения
        auto const src_2 = [&]
        {
            std::list<Value> tmp(input.begin(), result_saga.in.begin());

            if(!tmp.empty())
            {
                tmp.front() = op(tmp.front(), init_value);
            }

            return tmp;
        }();

        auto dest_expected = dest_old;
        auto const out_2 = saga::rebase_cursor(out_saga, dest_expected);

        auto const result_expected = saga::inclusive_scan(saga::cursor::all(src_2), out_2, op);

        // Сравнение
        CAPTURE(input, init_value, src_2);

        REQUIRE(dest_saga == dest_expected);

        REQUIRE((!result_saga.in || !result_saga.out));

        REQUIRE((result_saga.in.begin() - input.begin())
                == (result_saga.out.begin() - out_saga.begin()));
        REQUIRE(result_saga.in.end() == input.end());

        REQUIRE(result_saga.out.begin() - dest_saga.begin()
                == result_expected.out.begin() - dest_expected.begin());
        REQUIRE(result_saga.out.end() == out_saga.end());
    };
}

// exclusive_scan
TEST_CASE("exclusive_scan - minimalistic, default operation")
{
    using Value = unsigned;

    saga_test::property_checker << [](std::vector<Value> const & src, Value const & init_value)
    {
        // saga
        auto src_in = saga_test::make_istringstream_from_range(src);

        std::vector<Value> dest_saga;
        saga::exclusive_scan(saga::make_istream_cursor<Value>(src_in)
                             , saga::back_inserter(dest_saga), init_value);

        // Ожидаемое
        auto const src_2 = [&]
        {
            std::list<Value> tmp;

            if(!src.empty())
            {
                tmp.push_back(init_value);
                tmp.insert(tmp.end(), src.begin(), src.end() - 1);
            }

            return tmp;
        }();

        std::vector<Value> dest_expected;
        std::partial_sum(src_2.begin(), src_2.end(), std::back_inserter(dest_expected));

        // Сравнение
        REQUIRE(dest_saga == dest_expected);
    };
}

TEST_CASE("exclusive_scan - minimalistic, custom operation")
{
    using Value = unsigned;

    saga_test::property_checker << [](std::vector<Value> const & src, Value const & init_value)
    {
        auto const bin_op = std::multiplies<>{};

        // saga
        auto src_in = saga_test::make_istringstream_from_range(src);

        std::vector<Value> dest_saga;
        saga::exclusive_scan(saga::make_istream_cursor<Value>(src_in)
                             , saga::back_inserter(dest_saga), init_value, bin_op);

        // Ожидаемое
        auto const src_2 = [&]
        {
            std::list<Value> tmp;

            if(!src.empty())
            {
                tmp.push_back(init_value);
                tmp.insert(tmp.end(), src.begin(), src.end() - 1);
            }

            return tmp;
        }();

        std::vector<Value> dest_expected;
        std::partial_sum(src_2.begin(), src_2.end(), std::back_inserter(dest_expected), bin_op);

        // Сравнение
        REQUIRE(dest_saga == dest_expected);
    };
}

TEST_CASE("exclusive_scan: inplace, default operation")
{
    using Value = unsigned;
    saga_test::property_checker << [](std::vector<Value> const & src, Value const & init_value)
    {
        // Копирование
        std::vector<Value> expected;
        saga::exclusive_scan(saga::cursor::all(src), saga::back_inserter(expected), init_value);

        // На месте
        std::vector<Value> actual = src;
        saga::exclusive_scan(saga::cursor::all(actual), saga::cursor::all(actual), init_value);

        // Проверка
        REQUIRE(actual == expected);
    };
}

TEST_CASE("exclusive_scan: inplace, custom operation")
{
    using Value = unsigned;
    saga_test::property_checker << [](std::vector<Value> const & src, Value const & init_value)
    {
        auto const bin_op = std::multiplies<>{};

        // Копирование
        std::vector<Value> expected;
        saga::exclusive_scan(saga::cursor::all(src), saga::back_inserter(expected)
                             , init_value, bin_op);

        // На месте
        std::vector<Value> actual = src;
        saga::exclusive_scan(saga::cursor::all(actual), saga::cursor::all(actual)
                             , init_value, bin_op);

        // Проверка
        REQUIRE(actual == expected);
    };
}

TEST_CASE("exclusive_scan : subrange, default operation")
{
    using Value = unsigned;
    saga_test::property_checker << [](std::vector<Value> const & src
                                      , std::vector<Value> const & dest_old
                                      , Value const & init_value)
    {
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // exclusive_scan
        auto dest_actual = dest_old;
        auto const out_actual = saga_test::random_subcursor_of(saga::cursor::all(dest_actual));

        auto const r_actual = saga::exclusive_scan(input, out_actual, init_value);

        // inclusive_scan (с другим входом)
        auto src_inclusive = src;

        auto const input_inclusive = saga::rebase_cursor(input, src_inclusive);

        if(!!input_inclusive)
        {
            std::rotate(input_inclusive.begin(), input_inclusive.end() - 1, input_inclusive.end());
            input_inclusive.front() = init_value;
        }

        auto dest_expected = dest_old;

        auto const out_expected = saga::rebase_cursor(out_actual, dest_expected);

        auto const r_expected = saga::partial_sum(input_inclusive, out_expected);

        // Сравнение
        CAPTURE(src, dest_old, init_value, input, input_inclusive
                , saga::rebase_cursor(out_actual, dest_old));

        REQUIRE(dest_actual == dest_expected);

        auto const r_expected_in = saga::rebase_cursor(r_expected.in, src);

        REQUIRE(r_actual.in.begin() == r_expected_in.begin());
        REQUIRE(r_actual.in.end() == r_expected_in.end());
        REQUIRE(r_actual.in.dropped_front().begin() == r_expected_in.dropped_front().begin());
        REQUIRE(r_actual.in.dropped_back().end() == r_expected_in.dropped_back().end());

        auto const r_expected_out = saga::rebase_cursor(r_expected.out, dest_actual);

        REQUIRE(r_actual.out.begin() == r_expected_out.begin());
        REQUIRE(r_actual.out.end() == r_expected_out.end());
        REQUIRE(r_actual.out.dropped_front().begin() == r_expected_out.dropped_front().begin());
        REQUIRE(r_actual.out.dropped_back().end() == r_expected_out.dropped_back().end());
    };
}

TEST_CASE("exclusive_scan : subrange, custom operation")
{
    using Value = unsigned;
    saga_test::property_checker << [](std::vector<Value> const & src
                                      , std::vector<Value> const & dest_old
                                      , Value const & init_value)
    {
        auto const bin_op = std::multiplies<>{};

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // exclusive_scan
        auto dest_actual = dest_old;
        auto const out_actual = saga_test::random_subcursor_of(saga::cursor::all(dest_actual));

        auto const r_actual = saga::exclusive_scan(input, out_actual, init_value, bin_op);

        // inclusive_scan (с другим входом)
        auto src_inclusive = src;

        auto const input_inclusive = saga::rebase_cursor(input, src_inclusive);

        if(!!input_inclusive)
        {
            std::rotate(input_inclusive.begin(), input_inclusive.end() - 1, input_inclusive.end());
            input_inclusive.front() = init_value;
        }

        auto dest_expected = dest_old;

        auto const out_expected = saga::rebase_cursor(out_actual, dest_expected);

        auto const r_expected = saga::inclusive_scan(input_inclusive, out_expected, bin_op);

        // Сравнение
        CAPTURE(src, dest_old, init_value, input, input_inclusive
                , saga::rebase_cursor(out_actual, dest_old));

        REQUIRE(dest_actual == dest_expected);

        auto const r_expected_in = saga::rebase_cursor(r_expected.in, src);

        REQUIRE(r_actual.in.begin() == r_expected_in.begin());
        REQUIRE(r_actual.in.end() == r_expected_in.end());
        REQUIRE(r_actual.in.dropped_front().begin() == r_expected_in.dropped_front().begin());
        REQUIRE(r_actual.in.dropped_back().end() == r_expected_in.dropped_back().end());

        auto const r_expected_out = saga::rebase_cursor(r_expected.out, dest_actual);

        REQUIRE(r_actual.out.begin() == r_expected_out.begin());
        REQUIRE(r_actual.out.end() == r_expected_out.end());
        REQUIRE(r_actual.out.dropped_front().begin() == r_expected_out.dropped_front().begin());
        REQUIRE(r_actual.out.dropped_back().end() == r_expected_out.dropped_back().end());
    };
}

// transform_exclusive_scan
TEST_CASE("transform_exclusive_scan: minimalistic")
{
    using Value = unsigned;
    saga_test::property_checker << [](std::vector<Value> const & src, Value const & init_value)
    {
        auto const unary_op = [](Value const & arg) { return arg % 10; };
        auto const bin_op = std::multiplies<>{};

        // transform + exclusive_scan
        std::vector<Value> tmp;
        saga::transform(saga::cursor::all(src), saga::back_inserter(tmp), unary_op);

        std::vector<Value> expected;
        saga::exclusive_scan(saga::cursor::all(tmp), saga::back_inserter(expected)
                             , init_value, bin_op);

        // transform_exclusive_scan
        auto src_in = saga_test::make_istringstream_from_range(src);

        std::vector<Value> actual;
        saga::transform_exclusive_scan(saga::make_istream_cursor<Value>(src_in)
                                       , saga::back_inserter(actual)
                                       , init_value, bin_op, unary_op);

        // Проверка
        REQUIRE(actual == expected);
    };
}

TEST_CASE("transform_exclusive_scan: inplace")
{
    using Value = unsigned;
    saga_test::property_checker << [](std::vector<Value> const & src, Value const & init_value)
    {
        auto const unary_op = [](Value const & arg) { return arg % 10; };
        auto const bin_op = std::multiplies<>{};

        // Копирование
        std::vector<Value> expected;
        saga::transform_exclusive_scan(saga::cursor::all(src), saga::back_inserter(expected)
                                       , init_value, bin_op, unary_op);

        // На месте
        std::vector<Value> actual = src;
        saga::transform_exclusive_scan(saga::cursor::all(actual), saga::cursor::all(actual)
                                       , init_value, bin_op, unary_op);

        // Проверка
        REQUIRE(actual == expected);
    };
}

TEST_CASE("transform_exclusive_scan: subranges")
{
    using Value = unsigned;
    saga_test::property_checker << [](std::vector<Value> const & src
                                      , std::vector<Value> const & dest_old
                                      , Value const & init_value)
    {
        auto const unary_op = [](Value const & arg) { return arg % 10; };
        auto const bin_op = std::multiplies<>{};

        // Подготовка
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // transform_exclusive_scan
        std::vector<Value> dest_actual = dest_old;

        auto const out_actual = saga_test::random_subcursor_of(saga::cursor::all(dest_actual));
        auto const r_actual
            = saga::transform_exclusive_scan(input, out_actual, init_value, bin_op, unary_op);

        // transform + exclusive_scan
        std::vector<Value> dest_expected = dest_old;
        auto out_expected = saga::rebase_cursor(out_actual, dest_expected);
        out_expected.forget_front();
        auto const r_expected = saga::transform(input, out_expected, unary_op);

        saga::exclusive_scan(r_expected.out.dropped_front(), r_expected.out.dropped_front()
                             , init_value, bin_op);

        // Проверка
        CAPTURE(src, dest_old, input, init_value, saga::rebase_cursor(out_actual, dest_old));

        REQUIRE(dest_actual == dest_expected);

        REQUIRE(r_actual.in.begin() == r_expected.in.begin());
        REQUIRE(r_actual.in.end() == r_expected.in.end());
        REQUIRE(r_actual.in.dropped_front().begin() == r_expected.in.dropped_front().begin());
        REQUIRE(r_actual.in.dropped_back().end() == r_expected.in.dropped_back().end());

        REQUIRE(r_actual.out.begin() - out_actual.begin()
                == r_expected.out.begin() - out_expected.begin());
        REQUIRE(r_actual.out.end() == out_actual.end());
        REQUIRE(r_actual.out.dropped_front().begin() == dest_actual.begin());
        REQUIRE(r_actual.out.dropped_back().end() == dest_actual.end());
    };
}
