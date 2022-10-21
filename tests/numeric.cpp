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
#include <catch2/catch_amalgamated.hpp>

// Вспомогательные возможности, используемые в тестах
#include <saga/algorithm.hpp>
#include <saga/cursor/indices.hpp>
#include <saga/cursor/istream_cursor.hpp>
#include <saga/cursor/subrange.hpp>
#include <saga/utility/functional_macro.hpp>

#include <list>
#include <vector>

// Тесты
namespace
{
    constexpr bool check_iota_array()
    {
        std::array<int, 5> expected = {1, 2, 3, 4, 5};

        std::array<int, 5> actual{};
        auto const result = saga::iota(saga::cursor::all(actual), 1);

        return saga::equal(saga::cursor::all(actual), saga::cursor::all(expected))
               && result.value == 6
               && result.out == saga::cursor::drop_front_n(saga::cursor::all(actual), 5);
    }

    static_assert(check_iota_array(), "");
}

TEST_CASE("iota")
{
    using Value = unsigned int;

    saga_test::property_checker << [](std::vector<Value> const & src, Value const & init_value)
    {
        // saga
        auto src_saga = src;
        auto const cur = saga_test::random_subcursor_of(saga::cursor::all(src_saga));

        auto const result = saga::iota(cur, init_value);

        // std
        auto src_std = src;

        auto const cur_std = saga::rebase_cursor(cur, src_std);

        std::iota(cur_std.begin(), cur_std.end(), init_value);

        // Проверки
        REQUIRE(src_saga == src_std);

        auto const num = Value(saga::cursor::size(cur));

        REQUIRE(result.out == saga::cursor::drop_front_n(cur, num));
        REQUIRE(result.value == init_value + num);
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
namespace
{
    template <class Value, class... BinOp>
    void test_adjacent_difference_minimal(std::vector<Value> const & src, BinOp... bin_op)
    {
        static_assert(sizeof...(BinOp) <= 1);

        // std
        std::vector<Value> dest_std;
        std::adjacent_difference(src.begin(), src.end(), std::back_inserter(dest_std), bin_op...);

        // saga
        auto src_in = saga_test::make_istringstream_from_range(src);

        std::vector<Value> dest_saga;
        saga::adjacent_difference(saga::make_istream_cursor<Value>(src_in)
                                  , saga::back_inserter(dest_saga), bin_op...);

        // Сравнение
        REQUIRE(dest_saga == dest_std);
    }

    template <class Value, class... BinOp>
    void test_adjacent_difference_subcursor(std::vector<Value> const & src
                                            , std::vector<Value> const & dest_old, BinOp... bin_op)
    {
        static_assert(sizeof...(BinOp) <= 1);

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto dest_saga = dest_old;

        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto const result_saga = saga::adjacent_difference(input, out_saga, bin_op...);

        // std
        auto dest_std = dest_old;

        auto out_std_pos = dest_std.begin() + (out_saga.begin() - dest_saga.begin());
        auto const result_std = std::adjacent_difference(input.begin(), result_saga.in.begin()
                                                         , out_std_pos, bin_op...);

        // Сравнение
        REQUIRE(dest_saga == dest_std);
        REQUIRE((result_saga.out.begin() - dest_saga.begin()) == (result_std - dest_std.begin()));
        REQUIRE(result_saga.out.end() == out_saga.end());
    }
}

TEST_CASE("adjacent_difference")
{
    using Value = unsigned;

    saga_test::property_checker
    << ::test_adjacent_difference_minimal<Value>
    << ::test_adjacent_difference_subcursor<Value>
    << [](std::vector<Value> const & src)
    {
        ::test_adjacent_difference_minimal(src, std::plus<>{});
    }
    << [](std::vector<Value> const & src, std::vector<Value> const & dest_old)
    {
        ::test_adjacent_difference_subcursor(src, dest_old, std::plus<>{});
    };
}

// partial sum
namespace
{
    template <class Value, class... BinOp>
    void test_partial_sum_minimal(std::vector<Value> const & src, BinOp... bin_op)
    {
        static_assert(sizeof...(BinOp) <= 1);

        // std
        std::vector<Value> dest_std;
        std::partial_sum(src.begin(), src.end(), std::back_inserter(dest_std), bin_op...);

        // saga
        auto src_in = saga_test::make_istringstream_from_range(src);

        std::vector<Value> dest_saga;
        saga::partial_sum(saga::make_istream_cursor<Value>(src_in)
                          , saga::back_inserter(dest_saga), bin_op...);

        // Сравнение
        REQUIRE(dest_saga == dest_std);
    }

    template <class Value, class... BinOp>
    void test_partial_sum_subcursor(std::vector<Value> const & src
                                    , std::vector<Value> const & dest_old, BinOp... bin_op)
    {
        static_assert(sizeof...(BinOp) <= 1);

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto dest_saga = dest_old;

        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto const result_saga = saga::partial_sum(input, out_saga, bin_op...);

        // std
        auto dest_std = dest_old;

        auto const dest_std_pos = dest_std.begin() + (out_saga.begin() - dest_saga.begin());
        auto const result_std = std::partial_sum(input.begin(), result_saga.in.begin()
                                                 , dest_std_pos, bin_op...);

        // Сравнение
        REQUIRE(dest_saga == dest_std);
        REQUIRE((result_saga.out.begin() - dest_saga.begin()) == (result_std - dest_std.begin()));
        REQUIRE(result_saga.out.end() == out_saga.end());
    }
}

TEST_CASE("partial_sum")
{
    using Value = unsigned;

    saga_test::property_checker
    << ::test_partial_sum_minimal<Value>
    << ::test_partial_sum_subcursor<Value>
    << [](std::vector<Value> const & src)
    {
        ::test_partial_sum_minimal(src, std::multiplies<>{});
    }
    << [](std::vector<Value> const & src, std::vector<Value> const & dest_old)
    {
        ::test_partial_sum_subcursor(src, dest_old, std::multiplies<>{});
    };
}

// reduce
namespace
{
    template <class Value, class... BinOp>
    void test_reduce_minimal(std::vector<Value> const & src
                             , Value const & init_value, BinOp... bin_op)
    {
        static_assert(sizeof...(BinOp) <= 1);

        auto src_in = saga_test::make_istringstream_from_range(src);

        REQUIRE(saga::reduce(saga::make_istream_cursor<Value>(src_in), init_value, bin_op...)
                == saga::accumulate(saga::cursor::all(src), init_value, bin_op...));

    }
}
TEST_CASE("reduce - minimalistic")
{
    using Value = unsigned;

    saga_test::property_checker
    << ::test_reduce_minimal<Value>
    << [](std::vector<Value> const & src, Value const & init_value)
    {
        ::test_reduce_minimal(src, init_value, std::bit_xor<>{});
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
namespace
{
    template <class Container1, class Container2, class Value, class... BinOps>
    void test_inner_product(Container1 const & lhs, Container2 const & rhs,
                            Value const & init_value, BinOps... bin_ops)
    {
        static_assert(sizeof...(BinOps) == 0 || sizeof...(BinOps) == 2);

        auto const n_min = std::min(lhs.size(), rhs.size());

        auto const result_std = std::inner_product(lhs.begin(), lhs.begin() + n_min, rhs.begin(),
                                                   init_value, bin_ops...);

        auto const result_saga = saga::inner_product(saga::cursor::all(lhs), saga::cursor::all(rhs),
                                                     init_value, bin_ops...);

        static_assert(std::is_same<decltype(result_saga), Value const>{}, "");

        REQUIRE(result_saga == result_std);
    }
}

TEST_CASE("inner_product")
{
    using Value = unsigned int;

    saga_test::property_checker
    << ::test_inner_product<std::vector<Value>, std::list<bool>, Value>
    << [](std::vector<Value> const & lhs, std::list<Value> const & rhs, Value const & init_value)
    {
        auto const op1 = SAGA_OVERLOAD_SET(std::min);
        auto const op2 = SAGA_OVERLOAD_SET(std::max);

        ::test_inner_product(lhs, rhs, init_value, op1, op2);
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
namespace
{
    template <class Value, class... Args>
    void test_inclusive_scan_minimal(std::vector<Value> const & src, Args... bin_op)
    {
        static_assert(sizeof...(Args) <= 1);

        // std
        std::vector<Value> dest_std;
        std::partial_sum(src.begin(), src.end(), std::back_inserter(dest_std), bin_op...);

        // saga
        auto src_in = saga_test::make_istringstream_from_range(src);

        std::vector<Value> dest_saga;
        saga::inclusive_scan(saga::make_istream_cursor<Value>(src_in)
                             , saga::back_inserter(dest_saga), bin_op...);

        // Сравнение
        REQUIRE(dest_saga == dest_std);
    }

    template <class Value, class... Args>
    void test_inclusive_scan_inplace(std::vector<Value> const & src, Args... args)
    {
        static_assert(sizeof...(Args) <= 2);

        // Копируем
        std::vector<Value> dest_expected;
        saga::inclusive_scan(saga::cursor::all(src), saga::back_inserter(dest_expected), args...);

        // Выполняем алгоритм на месте
        std::vector<Value> dest = src;
        saga::inclusive_scan(saga::cursor::all(dest), saga::cursor::all(dest), args...);

        // Сравнение
        REQUIRE(dest == dest_expected);
    }

    template <class Value, class... Args>
    void test_inclusive_scan_subcursor(std::vector<Value> const & src
                                       , std::vector<Value> const & dest_old, Args... bin_op)
    {
        static_assert(sizeof...(Args) <= 1);

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto dest_saga = dest_old;

        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto const result_saga = saga::inclusive_scan(input, out_saga, bin_op...);

        // std
        auto dest_std = dest_old;
        auto const dest_std_pos = dest_std.begin() + (out_saga.begin() - dest_saga.begin());

        auto const result_std = std::partial_sum(input.begin(), result_saga.in.begin()
                                                 , dest_std_pos, bin_op...);

        // Сравнение
        REQUIRE(dest_saga == dest_std);
        REQUIRE((result_saga.out.begin() - dest_saga.begin()) == (result_std - dest_std.begin()));
        REQUIRE(result_saga.out.end() == out_saga.end());
    }
}

TEST_CASE("inclusive_scan - general")
{
    using Value = unsigned;

    saga_test::property_checker
    << ::test_inclusive_scan_minimal<Value>
    << [](std::vector<Value> const & src)
    {
        ::test_inclusive_scan_minimal(src, std::multiplies<>{});
    }
    << ::test_inclusive_scan_inplace<Value>
    << [](std::vector<Value> const & src)
    {
        ::test_inclusive_scan_inplace(src, std::multiplies<>{});
    }
    << [](std::vector<Value> const & src, Value const & init_value)
    {
        ::test_inclusive_scan_inplace(src, std::multiplies<>{}, init_value);
    }
    << ::test_inclusive_scan_subcursor<Value>
    << [](std::vector<Value> const & src, std::vector<Value> const & dest_old)
    {
        ::test_inclusive_scan_subcursor(src, dest_old, std::multiplies<>{});
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

        REQUIRE(result_saga.out == saga::rebase_cursor(result_expected.out, dest_saga));
    };
}

// exclusive_scan
namespace
{
    template <class Value, class... Args>
    void test_exclusive_scan_minimal(std::vector<Value> const & src
                                     , Value const & init_value, Args... bin_op)
    {
        static_assert(sizeof...(Args) <= 1);

        // saga
        auto src_in = saga_test::make_istringstream_from_range(src);

        std::vector<Value> dest_saga;
        saga::exclusive_scan(saga::make_istream_cursor<Value>(src_in)
                             , saga::back_inserter(dest_saga), init_value, bin_op...);

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
        std::partial_sum(src_2.begin(), src_2.end(), std::back_inserter(dest_expected), bin_op...);

        // Сравнение
        REQUIRE(dest_saga == dest_expected);
    }

    template <class Value, class... Args>
    void test_exclusive_scan_inplace(std::vector<Value> const & src
                                     , Value const & init_value, Args... bin_op)
    {
        // Копирование
        std::vector<Value> expected;
        saga::exclusive_scan(saga::cursor::all(src), saga::back_inserter(expected)
                             , init_value, bin_op...);

        // На месте
        std::vector<Value> actual = src;
        saga::exclusive_scan(saga::cursor::all(actual), saga::cursor::all(actual)
                             , init_value, bin_op...);

        // Проверка
        REQUIRE(actual == expected);
    }

    template <class Value, class... Args>
    void test_exclusive_scan_subcursor(std::vector<Value> const & src
                                      , std::vector<Value> const & dest_old
                                      , Value const & init_value, Args... bin_op)
    {
        static_assert(sizeof...(Args) <= 1);

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // exclusive_scan
        auto dest_actual = dest_old;
        auto const out_actual = saga_test::random_subcursor_of(saga::cursor::all(dest_actual));

        auto const r_actual = saga::exclusive_scan(input, out_actual, init_value, bin_op...);

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

        auto const r_expected = saga::inclusive_scan(input_inclusive, out_expected, bin_op...);

        // Сравнение
        CAPTURE(src, dest_old, init_value, input, input_inclusive
                , saga::rebase_cursor(out_actual, dest_old));

        REQUIRE(dest_actual == dest_expected);

        REQUIRE(r_actual.in == saga::rebase_cursor(r_expected.in, src));
        REQUIRE(r_actual.out == saga::rebase_cursor(r_expected.out, dest_actual));
    }
}

TEST_CASE("exclusive_scan - general")
{
    using Value = unsigned;

    saga_test::property_checker
    << ::test_exclusive_scan_minimal<Value>
    << [](std::vector<Value> const & src, Value const & init_value)
    {
        ::test_exclusive_scan_minimal(src, init_value, std::multiplies<>{});
    }
    << ::test_exclusive_scan_inplace<Value>
    << [](std::vector<Value> const & src, Value const & init_value)
    {
        ::test_exclusive_scan_inplace(src, init_value, std::multiplies<>{});
    }
    << ::test_exclusive_scan_subcursor<Value>
    << [](std::vector<Value> const & src, std::vector<Value> const & dest_old
          , Value const & init_value)
    {
        ::test_exclusive_scan_subcursor(src, dest_old, init_value, std::multiplies<>{});
    };
}

// transform_reduce
namespace
{
    template <class Value1, class Value2, class Total, class... Args>
    void test_transform_reduce_two_ranges_minimal(std::vector<Value1> const & lhs
                                      , std::vector<Value2> const & rhs
                                      , Total init_value
                                      , Args... bin_ops)
    {
        static_assert(sizeof...(Args) <= 2);

        // inner_product
        auto const r_expected = saga::inner_product(saga::cursor::all(lhs)
                                                    , saga::cursor::all(rhs), init_value
                                                    , bin_ops...);

        // transform_reduce
        auto lhs_is = saga_test::make_istringstream_from_range(lhs);
        auto rhs_is = saga_test::make_istringstream_from_range(rhs);

        auto const r_actual = saga::transform_reduce(saga::make_istream_cursor<Value1>(lhs_is)
                                                     , saga::make_istream_cursor<Value2>(rhs_is)
                                                     , init_value, bin_ops...);

        // Сравнение
        REQUIRE(r_actual == r_expected);
    }

    template <class Value1, class Value2, class Total, class... Args>
    void test_transform_reduce_two_ranges_subcursor(std::vector<Value1> const & lhs
                                                   , std::vector<Value2> const & rhs
                                                   , Total init_value, Args... bin_ops)
    {
        static_assert(sizeof...(Args) <= 2);

        auto const in1 = saga_test::random_subcursor_of(saga::cursor::all(lhs));
        auto const in2 = saga_test::random_subcursor_of(saga::cursor::all(rhs));

        // inner_product
        auto const r_expected = saga::inner_product(in1, in2, init_value, bin_ops...);

        // transform_reduce
        auto const r_actual = saga::transform_reduce(in1, in2, init_value, bin_ops...);

        // Сравнение
        REQUIRE(r_actual == r_expected);
    }
}

TEST_CASE("transform_reduce: two ranges")
{
    using Value1 = bool;
    using Value2 = unsigned int;
    using Total = unsigned long;

    saga_test::property_checker
    << ::test_transform_reduce_two_ranges_minimal<Value1, Value2, Total>
    << [](std::vector<Value1> const & lhs, std::vector<Value2> const & rhs, Total init_value)
    {
        auto reducer = [](Total const & x, Total const & y)
        {
            return std::min(x, y);
        };

        auto combiner = std::bit_xor<>{};

        ::test_transform_reduce_two_ranges_minimal(lhs, rhs, init_value, reducer, combiner);
    }
    << ::test_transform_reduce_two_ranges_subcursor<Value1, Value2, Total>
    << [](std::vector<Value1> const & lhs, std::vector<Value2> const & rhs, Total init_value)
    {
        auto reducer = [](Total const & x, Total const & y)
        {
            return std::min(x, y);
        };

        auto combiner = std::bit_xor<>{};

        ::test_transform_reduce_two_ranges_subcursor(lhs, rhs, init_value, reducer, combiner);
    };
}

TEST_CASE("transform_reduce: one range, minimalistic")
{
    using Value1 = int;
    using Value2 = long;

    saga_test::property_checker << [](std::vector<Value1> const & src, Value2 const & init_value)
    {
        auto const transformer = [](Value1 const & arg)
        {
            return arg % 101;
        };

        auto const reducer = [](Value2 const & lhs, Value2 const & rhs)
        {
            return lhs ^ rhs;
        };

        // transform + reduce
        std::vector<Value2> tmp;
        saga::transform(saga::cursor::all(src), saga::back_inserter(tmp), transformer);

        auto const r_expected = saga::reduce(saga::cursor::all(tmp), init_value, reducer);

        // transform_reduce
        auto src_in = saga_test::make_istringstream_from_range(src);

        auto const r_actual = saga::transform_reduce(saga::make_istream_cursor<Value1>(src_in)
                                                     , init_value, reducer, transformer);

        // Сравнение
        REQUIRE(r_actual == r_expected);
    };
}

TEST_CASE("transform_reduce: one range, subcursors")
{
    using Value1 = int;
    using Value2 = long;

    saga_test::property_checker << [](std::vector<Value1> const & src, Value2 const & init_value)
    {
        auto const transformer = [](Value1 const & arg)
        {
            return arg % 101;
        };

        auto const reducer = [](Value2 const & lhs, Value2 const & rhs)
        {
            return lhs ^ rhs;
        };

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // transform + reduce
        std::vector<Value2> tmp;
        saga::transform(input, saga::back_inserter(tmp), transformer);

        auto const r_expected = saga::reduce(saga::cursor::all(tmp), init_value, reducer);

        // transform_reduce
        auto const r_actual = saga::transform_reduce(input, init_value, reducer, transformer);

        // Сравнение
        REQUIRE(r_actual == r_expected);
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

        REQUIRE(r_actual.in == r_expected.in);

        REQUIRE(r_actual.out.begin() - out_actual.begin()
                == r_expected.out.begin() - out_expected.begin());
        REQUIRE(r_actual.out.end() == out_actual.end());
        REQUIRE(r_actual.out.dropped_front().begin() == dest_actual.begin());
        REQUIRE(r_actual.out.dropped_back().end() == dest_actual.end());
    };
}

// transform_inclusive_scan
namespace
{
    template <class Container, class... Args>
    void test_transform_inclusive_scan_minimal(Container const & src, Args const & ... init_value)
    {
        static_assert(sizeof...(Args) <= 1);

        using Value = typename Container::value_type;

        auto const unary_op = [](Value const & arg) { return arg % 10; };
        auto const bin_op = std::multiplies<>{};

        // transform + inclusive_scan
        Container tmp;
        saga::transform(saga::cursor::all(src), saga::back_inserter(tmp), unary_op);

        Container expected;
        saga::inclusive_scan(saga::cursor::all(tmp), saga::back_inserter(expected)
                             , bin_op, init_value...);

        // transform_exclusive_scan
        auto src_in = saga_test::make_istringstream_from_range(src);

        Container actual;
        saga::transform_inclusive_scan(saga::make_istream_cursor<Value>(src_in)
                                       , saga::back_inserter(actual)
                                       , bin_op, unary_op, init_value...);

        // Проверка
        REQUIRE(actual == expected);
    }
}

TEST_CASE("transform_inclusive_scan")
{
    using Value = unsigned;

    saga_test::property_checker
    << ::test_transform_inclusive_scan_minimal<std::vector<Value>>
    << ::test_transform_inclusive_scan_minimal<std::vector<Value>, Value>;
}

namespace
{
    template <class Container, class... Args>
    void test_transform_inclusive_scan_subcursor(Container const & src, Container const & dest_old
                                                 , Args const & ... init_value)
    {
        static_assert(sizeof...(Args) <= 1);

        using Value = typename Container::value_type;

        auto const unary_op = [](Value const & arg) { return arg % 10; };
        auto const bin_op = std::multiplies<>{};

        // Подготовка
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // transform_exclusive_scan
        auto dest_actual = dest_old;

        auto const out_actual = saga_test::random_subcursor_of(saga::cursor::all(dest_actual));
        auto const r_actual
            = saga::transform_inclusive_scan(input, out_actual, bin_op, unary_op, init_value...);

        // transform + exclusive_scan
        auto dest_expected = dest_old;

        auto out_expected = saga::rebase_cursor(out_actual, dest_expected);
        out_expected.forget_front();
        auto const r_expected = saga::transform(input, out_expected, unary_op);

        saga::inclusive_scan(r_expected.out.dropped_front(), r_expected.out.dropped_front()
                             , bin_op, init_value...);

        // Проверка
        REQUIRE(dest_actual == dest_expected);

        REQUIRE(r_actual.in == r_expected.in);

        REQUIRE(r_actual.out.begin() - out_actual.begin()
                == r_expected.out.begin() - out_expected.begin());
        REQUIRE(r_actual.out.end() == out_actual.end());
        REQUIRE(r_actual.out.dropped_front().begin() == dest_actual.begin());
        REQUIRE(r_actual.out.dropped_back().end() == dest_actual.end());
    }
}

TEST_CASE("transform_inclusive_scan: subranges")
{
    using Value = unsigned;
    saga_test::property_checker
    << ::test_transform_inclusive_scan_subcursor<std::vector<Value>>
    << ::test_transform_inclusive_scan_subcursor<std::vector<Value>, Value>;
}

namespace
{
    template <class Container, class... Args>
    void test_transform_inclusive_scan_inplace(Container const & src, Args const & ... init_value)
    {
        static_assert(sizeof...(Args) <= 1);

        using Value = typename Container::value_type;

        auto const unary_op = [](Value const & arg) { return arg % 10; };
        auto const bin_op = std::multiplies<>{};

        // Копирование
        Container expected;
        saga::transform_inclusive_scan(saga::cursor::all(src), saga::back_inserter(expected)
                                       , bin_op, unary_op, init_value...);

        // На месте
        Container actual = src;
        saga::transform_inclusive_scan(saga::cursor::all(actual), saga::cursor::all(actual)
                                       , bin_op, unary_op, init_value...);

        // Проверка
        REQUIRE(actual == expected);
    }
}

TEST_CASE("transform_inclusive_scan: inplace")
{
    using Value = unsigned;
    saga_test::property_checker
    << ::test_transform_inclusive_scan_inplace<std::vector<Value>>
    << ::test_transform_inclusive_scan_inplace<std::vector<Value>, Value>;
}

namespace
{
    static_assert(2 * 3 == saga::gcd(2 * 2 * 3, 2 * 3 * 3), "");

    static_assert(2 * 2 * 3 * 3 == saga::lcm(2 * 2 * 3, 2 * 3 * 3));
    static_assert(225 == saga::lcm(45, 75));
}

TEST_CASE("gcd : functional object")
{
    using Value1 = int;
    using Value2 = long long;

    static_assert(sizeof(Value2) > sizeof(Value1), "");

    saga_test::property_checker <<[](Value1 const & lhs, Value1 const & rhs)
    {
        REQUIRE(saga::gcd(lhs, Value2(rhs)) == std::gcd(lhs, Value2(rhs)));
    };
}

TEST_CASE("lcm : functional object")
{
    using Value1 = int;
    using Value2 = long long;

    static_assert(sizeof(Value2) > sizeof(Value1), "");

    saga_test::property_checker <<[](Value1 const & lhs, Value1 const & rhs)
    {
        REQUIRE(saga::lcm(lhs, Value2(rhs)) == std::lcm(Value2(lhs), Value2(rhs)));
    };
}

TEST_CASE("nth_permutation: default compare")
{
    using Container = std::vector<int>;
    Container const elements{0, 1, 2, 3};

    auto const num = elements.size();

    auto const n_perm = saga::accumulate(saga::cursor::indices(1u, num + 1), 1u
                                         , std::multiplies<>{});

    std::vector<Container> perms;

    for(auto const & index : saga::cursor::indices(n_perm))
    {
        auto perm = elements;
        saga::nth_permutation(saga::cursor::all(perm), index);

        REQUIRE(saga::is_permutation(saga::cursor::all(perm), saga::cursor::all(elements)));

        perms.push_back(std::move(perm));
    }

    CAPTURE(perms);
    CAPTURE(saga::is_sorted_until(saga::cursor::all(perms)).dropped_front());

    REQUIRE(perms.size() == n_perm);
    REQUIRE(saga::is_sorted(saga::cursor::all(perms)));

    perms.erase(saga::unique(saga::cursor::all(perms)).begin(), perms.end());

    REQUIRE(perms.size() == n_perm);
}
