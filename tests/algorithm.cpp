/* (c) 2020-2022 Галушин Павел Викторович, galushin@gmail.com

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

// Тестируемый заголовочный файл
#include <saga/algorithm.hpp>

// Инфраструктура тестирования
#include "saga_test.hpp"
#include <catch2/catch_amalgamated.hpp>

// Вспомогательные файлы
#include <saga/accumulator/sum.hpp>
#include <saga/cursor/indices.hpp>
#include <saga/cursor/istream_cursor.hpp>
#include <saga/cursor/subrange.hpp>
#include <saga/cursor/take.hpp>
#include <saga/iterator/reverse.hpp>
#include <saga/math.hpp>
#include <saga/utility/functional_macro.hpp>

#include <forward_list>
#include <list>
#include <string>
#include <vector>

// Тесты
namespace
{
    using Containers = std::tuple<std::forward_list<int>, std::list<int>, std::vector<int>>;
}

TEMPLATE_LIST_TEST_CASE("cursor::size", "saga_test", Containers)
{
    saga_test::property_checker << [](TestType const & src)
    {
        REQUIRE(saga::cursor::size(saga::cursor::all(src))
                == std::distance(src.begin(), src.end()));
    };
}

TEMPLATE_LIST_TEST_CASE("random_position_of", "saga_test", Containers)
{
    saga_test::property_checker << [](TestType const & src)
    {
        auto const pos = saga_test::random_position_of(src);

        REQUIRE(0 <= pos);
        REQUIRE(pos <= std::distance(src.begin(), src.end()));
    };
}

TEMPLATE_LIST_TEST_CASE("random_subcursor_of", "saga_test", Containers)
{
    saga_test::property_checker << [](TestType const & src)
    {
        auto const result = saga_test::random_subcursor_of(saga::cursor::all(src));
        auto const num = std::distance(src.begin(), src.end());

        auto const pos1 = std::distance(src.begin(), result.begin());
        auto const pos2 = std::distance(src.begin(), result.end());

        REQUIRE(0 <= pos1);
        REQUIRE(pos1 <= num);

        REQUIRE(0 <= pos2);
        REQUIRE(pos2 <= num);

        REQUIRE(pos1 <= pos2);
    };
}

namespace
{
    template <class Value1, class Value2, class... Args>
    void check_equal_mimimal(std::vector<Value1> const & src1
                             , std::vector<Value2> const & src2
                             , Args... bin_pred)
    {
        static_assert(sizeof...(Args) <= 1);

        auto src1_in = saga_test::make_istringstream_from_range(src1);
        auto src2_in = saga_test::make_istringstream_from_range(src2);

        REQUIRE(saga::equal(saga::make_istream_cursor<Value1>(src1_in),
                            saga::make_istream_cursor<Value2>(src2_in), bin_pred...)
                == std::equal(src1.begin(), src1.end(), src2.begin(), src2.end(), bin_pred...));

        REQUIRE(saga::equal(saga::cursor::all(src1), saga::cursor::all(src1), bin_pred...));
        REQUIRE(saga::equal(saga::cursor::all(src2), saga::cursor::all(src2), bin_pred...));
    }

    template <class Value1, class Value2, class... Args>
    void check_equal_subcursor(std::vector<Value1> const & src1
                               , std::vector<Value2> const & src2
                               , Args... bin_pred)
    {
        static_assert(sizeof...(Args) <= 1);

        auto const in1 = saga_test::random_subcursor_of(saga::cursor::all(src1));
        auto const in2 = saga_test::random_subcursor_of(saga::cursor::all(src2));

        REQUIRE(saga::equal(in1, in2, bin_pred...)
                == std::equal(in1.begin(), in1.end(), in2.begin(), in2.end(), bin_pred...));

        REQUIRE(saga::equal(in1, in1, bin_pred...));
        REQUIRE(saga::equal(in2, in2, bin_pred...));
    }
}

TEST_CASE("equal - general")
{
    using Value1 = int;
    using Value2 = long;

    saga_test::property_checker
    << ::check_equal_mimimal<Value1, Value2>
    << ::check_equal_subcursor<Value1, Value2>
    << [](std::vector<Value1> const & src1, std::vector<Value2> const & src2)
    {
        auto const bin_pred = saga::equivalent_up_to([](auto const & arg) { return arg % 2; });

        ::check_equal_mimimal(src1, src2, bin_pred);
    }
    << [](std::vector<Value1> const & src1, std::vector<Value2> const & src2)
    {
        auto const bin_pred = saga::equivalent_up_to([](auto const & arg) { return arg % 2; });

        ::check_equal_subcursor(src1, src2, bin_pred);
    };
}

TEST_CASE("equal - custom predicate, invented true")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src1)
    {
        auto const fun = [](Value const & arg) { return arg % 2; };

        std::vector<Value> src2;
        saga::transform(saga::cursor::all(src1), saga::back_inserter(src2), fun);

        CAPTURE(src1, src2);

        auto const bin_pred = saga::equivalent_up_to(fun);

        REQUIRE(saga::equal(saga::cursor::all(src1), saga::cursor::all(src2), bin_pred));
    };
}

TEST_CASE("equal - subcursor, custom predicate, invented")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src1)
    {
        auto const fun = [](Value const & arg) { return arg % 2; };

        std::vector<Value> src2;
        saga::transform(saga::cursor::all(src1), saga::back_inserter(src2), fun);

        CAPTURE(src1, src2);

        auto const bin_pred = saga::equivalent_up_to(fun);

        auto const in1 = saga_test::random_subcursor_of(saga::cursor::all(src1));
        auto const in2 = saga_test::random_subcursor_of(saga::cursor::all(src2));

        REQUIRE(saga::equal(in1, in2, bin_pred)
                == std::equal(in1.begin(), in1.end(), in2.begin(), in2.end(), bin_pred));
    };
}

// @todo Аналогичный тест для произвольного предиката, нужен трассировщик для функциональных объектов
TEST_CASE("equal: random access cursor optimization, default predicate")
{
    using Value = saga::regular_tracer<int>;

    saga_test::property_checker << [](std::vector<Value> const & src1, Value const & value)
    {
        auto const src2 = [&](){ auto tmp = src1; tmp.push_back(value); return tmp; }();

        REQUIRE(src2.size() != src1.size());

        auto const equality_comparisons_before = Value::equality_comparisons();

        REQUIRE(saga::equal(saga::cursor::all(src1), saga::cursor::all(src2)) == false);

        REQUIRE(Value::equality_comparisons() == equality_comparisons_before);
    };
}

TEST_CASE("all_of, any_of, some_of - minimal")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto const pred = [](Value const & x) { return x % 2 == 0; };

        auto src_istream_all = saga_test::make_istringstream_from_range(src);
        auto src_istream_any = saga_test::make_istringstream_from_range(src);
        auto src_istream_none = saga_test::make_istringstream_from_range(src);

        REQUIRE(saga::all_of(saga::make_istream_cursor<Value>(src_istream_all), pred)
                == std::all_of(src.begin(), src.end(), pred));

        REQUIRE(saga::any_of(saga::make_istream_cursor<Value>(src_istream_any), pred)
                == std::any_of(src.begin(), src.end(), pred));

        REQUIRE(saga::none_of(saga::make_istream_cursor<Value>(src_istream_none), pred)
                == std::none_of(src.begin(), src.end(), pred));
    };
}

TEST_CASE("all_of, any_of, some_of - subcursor")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & values)
    {
        auto const pred = [](Value const & x) { return x % 2 == 0; };

        auto const src = saga_test::random_subcursor_of(saga::cursor::all(values));

        REQUIRE(saga::all_of(src, pred)  == std::all_of(src.begin(), src.end(), pred));
        REQUIRE(saga::any_of(src, pred)  == std::any_of(src.begin(), src.end(), pred));
        REQUIRE(saga::none_of(src, pred) == std::none_of(src.begin(), src.end(), pred));
    };
}

TEST_CASE("for_each - minimal")
{
    using Value = unsigned;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        // saga
        auto src_in = saga_test::make_istringstream_from_range(src);
        auto const result_saga = saga::for_each(saga::make_istream_cursor<Value>(src_in)
                                                , saga::sum_accumulator<Value>{});

        // std
        auto const result_std = std::for_each(src.begin(), src.end()
                                              , saga::sum_accumulator<Value>{});

        // Сравение
        REQUIRE(result_saga.fun.sum() == result_std.sum());
    };
}

TEST_CASE("for_each - subcursor, const")
{
    using Value = unsigned;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto const sub = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto const result_saga = saga::for_each(sub, saga::sum_accumulator<Value>{});

        // std
        auto const result_std = std::for_each(sub.begin(), sub.end()
                                              , saga::sum_accumulator<Value>{});

        // Сравение
        REQUIRE(result_saga.in.begin() == sub.end());
        REQUIRE(result_saga.in.end() == sub.end());

        REQUIRE(result_saga.fun.sum() == result_std.sum());
    };
}

TEST_CASE("for_each - subcursor, mutable")
{
    using Value = unsigned;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto const fun = [](Value & x) { x %= 2; };

        // saga
        auto src_saga = src;
        auto const cur_saga = saga_test::random_subcursor_of(saga::cursor::all(src_saga));

        auto const result_saga = saga::for_each(cur_saga, fun);

        // std
        auto src_std = src;
        auto const range_std = saga::rebase_cursor(cur_saga, src_std);

        std::for_each(range_std.begin(), range_std.end(), fun);

        // Сравение
        REQUIRE(result_saga.in.begin() == cur_saga.end());
        REQUIRE(result_saga.in.end() == cur_saga.end());

        REQUIRE(src_saga == src_std);
    };
}

TEST_CASE("for_each_n - minimal")
{
    using Value = unsigned;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        // saga
        auto const num = saga_test::random_uniform(0, src.size());

        auto src_in = saga_test::make_istringstream_from_range(src);

        auto const result_saga = saga::for_each_n(saga::make_istream_cursor<Value>(src_in)
                                                  , num, saga::sum_accumulator<Value>{});

        // std
        auto const result_std
            = std::for_each(src.begin(), src.begin() + num, saga::sum_accumulator<Value>{});

        // Сравение
        REQUIRE(result_saga.fun.sum() == result_std.sum());
        REQUIRE(result_saga.fun.sum() == result_std.sum());
    };
}

TEST_CASE("for_each_n - subcursor, const")
{
    using Value = unsigned;

    saga_test::property_checker << [](std::vector<Value> const & src
                                      , saga_test::container_size<std::ptrdiff_t> num)
    {
        auto const sub = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto const result_saga = saga::for_each_n(sub, num.value, saga::sum_accumulator<Value>{});

        // std
        auto const n_common = std::min(sub.size(), num.value);

        auto const result_std
            = std::for_each(sub.begin(), sub.begin() + n_common, saga::sum_accumulator<Value>{});

        // Сравение
        auto result_expected = sub;
        result_expected.drop_front(n_common);

        REQUIRE(result_saga.in == result_expected);

        REQUIRE(result_saga.fun.sum() == result_std.sum());
    };
}

TEST_CASE("for_each_n - subcursor, mutable")
{
    using Value = unsigned;

    saga_test::property_checker << [](std::vector<Value> const & src
                                      , saga_test::container_size<std::ptrdiff_t> num)
    {
        auto const fun = [](Value & x) { x %= 2; };

        // saga
        auto src_saga = src;
        auto const cur_saga = saga_test::random_subcursor_of(saga::cursor::all(src_saga));

        auto const result_saga = saga::for_each_n(cur_saga, num.value, fun);

        auto const n_common = std::min(cur_saga.size(), num.value);

        // std
        auto src_std = src;
        auto const range_std = saga::rebase_cursor(cur_saga, src_std);

        std::for_each(range_std.begin(), range_std.begin() + n_common, fun);

        // Сравение
        auto result_in_expected = cur_saga;
        result_in_expected.drop_front(n_common);

        REQUIRE(result_saga.in == result_in_expected);

        REQUIRE(src_saga == src_std);
    };
}

TEST_CASE("count - minimal")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src, Value const & value)
    {
        auto src_istream = saga_test::make_istringstream_from_range(src);

        REQUIRE(saga::count(saga::make_istream_cursor<Value>(src_istream), value)
                == std::count(src.begin(), src.end(), value));
    };
}

TEST_CASE("count >= 0")
{
    using Value = int;

    saga_test::property_checker << [](saga_test::container_size<Value> num, Value const & value)
    {
        std::vector<Value> const src(num.value, value);

        auto src_istream = saga_test::make_istringstream_from_range(src);

        REQUIRE(saga::count(saga::make_istream_cursor<Value>(src_istream), value)
                == std::count(src.begin(), src.end(), value));
    };
}

TEST_CASE("count - subcursor")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & values, Value const & value)
    {
        auto const src = saga_test::random_subcursor_of(saga::cursor::all(values));

        REQUIRE(saga::count(saga::cursor::all(src), value)
                == std::count(src.begin(), src.end(), value));
    };
}

TEST_CASE("count - subcursor, custom binary predicate")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & values, Value const & value)
    {
        auto const bin_pred = [](Value const & lhs, Value const & rhs)
            { return lhs % 2 == rhs % 2; };

        auto const pred = [&](Value const & arg) { return bin_pred(arg, value); };

        auto const src = saga_test::random_subcursor_of(saga::cursor::all(values));

        REQUIRE(saga::count(saga::cursor::all(src), value, bin_pred)
                == std::count_if(src.begin(), src.end(), pred));
    };
}

TEST_CASE("count_if - minimal")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto const pred = [](Value const & x) { return x % 2 == 0; };

        auto src_istream = saga_test::make_istringstream_from_range(src);

        REQUIRE(saga::count_if(saga::make_istream_cursor<Value>(src_istream), pred)
                == std::count_if(src.begin(), src.end(), pred));
    };
}

TEST_CASE("count_if - subcursor")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & values)
    {
        auto const pred = [](Value const & x) { return x % 2 == 0; };

        auto const src = saga_test::random_subcursor_of(saga::cursor::all(values));

        REQUIRE(saga::count_if(saga::cursor::all(src), pred)
                == std::count_if(src.begin(), src.end(), pred));
    };
}

TEST_CASE("mismatch - minimal, default predicate")
{
    using Value1 = int;
    using Value2 = long;

    saga_test::property_checker
    << [](std::vector<Value1> const & src1, std::vector<Value2> const & src2)
    {
        auto src1_in = saga_test::make_istringstream_from_range(src1);
        auto src2_in = saga_test::make_istringstream_from_range(src2);

        auto r_saga = saga::mismatch(saga::make_istream_cursor<Value1>(src1_in)
                                    , saga::make_istream_cursor<Value2>(src2_in));

        auto const r_std = std::mismatch(src1.begin(), src1.end(), src2.begin(), src2.end());

        // Проверки
        REQUIRE(!r_saga.in1 == (r_std.first == src1.end()));
        REQUIRE(!r_saga.in2 == (r_std.second == src2.end()));

        REQUIRE((!r_saga.in1 || !r_saga.in2 || (*r_saga.in1 != *r_saga.in2)));

        REQUIRE(saga::cursor::size(std::move(r_saga.in1)) == (src1.end() - r_std.first));
        REQUIRE(saga::cursor::size(std::move(r_saga.in2)) == (src2.end() - r_std.second));
    };
}

TEST_CASE("mismatch - minimal, custom predicate")
{
    using Value1 = int;
    using Value2 = long;

    saga_test::property_checker
    << [](std::vector<Value1> const & src1, std::vector<Value2> const & src2)
    {
        auto const pred = saga::equivalent_up_to([](auto const & arg) { return arg % 7; });

        auto src1_in = saga_test::make_istringstream_from_range(src1);
        auto src2_in = saga_test::make_istringstream_from_range(src2);

        auto r_saga = saga::mismatch(saga::make_istream_cursor<Value1>(src1_in)
                                    , saga::make_istream_cursor<Value2>(src2_in), pred);

        auto const r_std = std::mismatch(src1.begin(), src1.end(), src2.begin(), src2.end(), pred);

        // Проверки
        REQUIRE(!r_saga.in1 == (r_std.first == src1.end()));
        REQUIRE(!r_saga.in2 == (r_std.second == src2.end()));

        REQUIRE((!r_saga.in1 || !r_saga.in2 || !pred(*r_saga.in1, *r_saga.in2)));

        REQUIRE(saga::cursor::size(std::move(r_saga.in1)) == (src1.end() - r_std.first));
        REQUIRE(saga::cursor::size(std::move(r_saga.in2)) == (src2.end() - r_std.second));
    };
}

TEST_CASE("mismatch - subcursors, default predicate")
{
    saga_test::property_checker <<[](std::vector<int> const & src1, std::list<long> const & src2)
    {
        auto const in1 = saga_test::random_subcursor_of(saga::cursor::all(src1));
        auto const in2 = saga_test::random_subcursor_of(saga::cursor::all(src2));

        auto const r_saga = saga::mismatch(in1, in2);
        auto const r_std = std::mismatch(in1.begin(), in1.end(), in2.begin(), in2.end());

        REQUIRE(r_saga.in1.begin() == r_std.first);
        REQUIRE(r_saga.in1.end() == in1.end());

        REQUIRE(r_saga.in2.begin() == r_std.second);
        REQUIRE(r_saga.in2.end() == in2.end());
    };
}

TEST_CASE("mismatch - subcursors, custom predicate")
{
    saga_test::property_checker <<[](std::vector<int> const & src1, std::list<long> const & src2)
    {
        auto const bin_pred = saga::equivalent_up_to([](auto const & arg) { return arg % 7; });

        auto const in1 = saga_test::random_subcursor_of(saga::cursor::all(src1));
        auto const in2 = saga_test::random_subcursor_of(saga::cursor::all(src2));

        auto const r_saga = saga::mismatch(in1, in2, bin_pred);
        auto const r_std = std::mismatch(in1.begin(), in1.end(), in2.begin(), in2.end(), bin_pred);

        REQUIRE(r_saga.in1.begin() == r_std.first);
        REQUIRE(r_saga.in1.end() == in1.end());

        REQUIRE(r_saga.in2.begin() == r_std.second);
        REQUIRE(r_saga.in2.end() == in2.end());
    };
}

TEST_CASE("find - minimal")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src, Value const & value)
    {
        auto const r_std = std::find(src.begin(), src.end(), value);

        auto src_istream = saga_test::make_istringstream_from_range(src);
        auto r_saga = saga::find(saga::make_istream_cursor<Value>(src_istream), value);

        REQUIRE(!r_saga == (r_std == src.end()));

        REQUIRE((!r_saga || *r_saga == value));

        REQUIRE((src.end() - r_std) == saga::cursor::size(std::move(r_saga)));
    };
}

TEST_CASE("find - subcursor")
{
    using Value = long;

    saga_test::property_checker << [](std::vector<Value> const & src, Value const & value)
    {
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const r_std = std::find(input.begin(), input.end(), value);

        auto const r_saga = saga::find(input, value);

        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(r_saga.end() == input.end());
    };
}

TEST_CASE("find - invented, true")
{
    using Value = long;

    saga_test::property_checker << [](std::vector<Value> src, Value const & value)
    {
        src.push_back(value);

        auto const input = saga::cursor::all(src);

        auto const r_std = std::find(input.begin(), input.end(), value);

        auto const r_saga = saga::find(input, value);

        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(r_saga.end() == input.end());

        REQUIRE(!!r_saga);
        REQUIRE(*r_saga == value);
    };
}

TEST_CASE("find - subcursor, custom binary predicate")
{
    using Value = long;

    saga_test::property_checker << [](std::vector<Value> const & src, Value const & value)
    {
        auto const bin_pred = [](Value const & lhs, Value const & rhs)
        {
            return lhs % 5 == rhs % 5;
        };

        auto const pred = [&](Value const & arg) { return bin_pred(arg, value); };

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const r_bin_pred = saga::find(input, value, bin_pred);
        auto const r_unary_pred = saga::find_if(input, pred);

        REQUIRE(r_bin_pred == r_unary_pred);
    };
}

TEST_CASE("find_if - minimal")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto const pred = [](Value const & arg) { return arg % 4 == 0; };
        auto const r_std = std::find_if(src.begin(), src.end(), pred);

        auto src_istream = saga_test::make_istringstream_from_range(src);
        auto r_saga = saga::find_if(saga::make_istream_cursor<Value>(src_istream), pred);

        REQUIRE(!r_saga == (r_std == src.end()));

        if(!!r_saga)
        {
            REQUIRE(pred(*r_saga));
        }

        REQUIRE((src.end() - r_std) == saga::cursor::size(std::move(r_saga)));
    };
}

TEST_CASE("find_if - subcursor")
{
    using Value = long;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto const pred = [](Value const & x) { return x % 2 == 0; };

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const r_std = std::find_if(input.begin(), input.end(), pred);

        auto const r_saga = saga::find_if(input, pred);

        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(r_saga.end() == input.end());
    };
}

TEST_CASE("find_if_not - minimal")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto const pred = [](Value const & arg) { return arg % 2 == 0; };
        auto const r_std = std::find_if_not(src.begin(), src.end(), pred);

        auto src_istream = saga_test::make_istringstream_from_range(src);
        auto r_saga = saga::find_if_not(saga::make_istream_cursor<Value>(src_istream), pred);

        REQUIRE(!r_saga == (r_std == src.end()));

        if(!!r_saga)
        {
            REQUIRE(!pred(*r_saga));
        }

        REQUIRE((src.end() - r_std) == saga::cursor::size(std::move(r_saga)));
    };
}

TEST_CASE("find_if_not - subcursor")
{
    using Value = long;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto const pred = [](Value const & x) { return x % 2 == 0; };

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const r_std = std::find_if_not(input.begin(), input.end(), pred);

        auto const r_saga = saga::find_if_not(input, pred);

        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(r_saga.end() == input.end());
    };
}

TEST_CASE("find_end: default predicate, minimalistic")
{
    using Value = int;

    saga_test::property_checker
    <<[](std::forward_list<Value> const & haystack, std::forward_list<Value> const & needle)
    {
        auto const haystack_cur = saga_test::random_subcursor_of(saga::cursor::all(haystack));
        auto const needle_cur = saga_test::random_subcursor_of(saga::cursor::all(needle));

        auto const r_saga = saga::find_end(haystack_cur, needle_cur);
        auto const r_std = std::find_end(haystack_cur.begin(), haystack_cur.end()
                                         , needle_cur.begin(), needle_cur.end());

        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(saga::cursor::size(r_saga)
                == (r_std != haystack_cur.end() ? saga::cursor::size(needle_cur) : 0));
        REQUIRE(r_saga.dropped_front().begin() == haystack_cur.begin());
        REQUIRE(r_saga.dropped_back().end() == haystack_cur.end());
    };
}

TEST_CASE("find_end: custom predicate, minimalistic")
{
    using Value = int;

    saga_test::property_checker
    <<[](std::forward_list<Value> const & haystack, std::forward_list<Value> const & needle)
    {
        auto const pred = saga::equivalent_up_to([](Value const & arg) { return arg % 2;});

        auto const haystack_cur = saga_test::random_subcursor_of(saga::cursor::all(haystack));
        auto const needle_cur = saga_test::random_subcursor_of(saga::cursor::all(needle));

        auto const r_saga = saga::find_end(haystack_cur, needle_cur, pred);
        auto const r_std = std::find_end(haystack_cur.begin(), haystack_cur.end()
                                         , needle_cur.begin(), needle_cur.end(), pred);

        REQUIRE(r_saga.begin() == r_std);
        if(r_std != haystack_cur.end())
        {
            REQUIRE(saga::cursor::size(r_saga) == saga::cursor::size(needle_cur));
        }
        REQUIRE(r_saga.dropped_front().begin() == haystack_cur.begin());
        REQUIRE(r_saga.dropped_back().end() == haystack_cur.end());
    };
}

TEST_CASE("find_end: default predicate, guaranty")
{
    using Value = int;

    saga_test::property_checker
    <<[](std::list<Value> const & prefix, std::forward_list<Value> const & needle
         , std::list<Value> const & suffix)
    {
        auto const haystack = [&]
        {
            auto tmp = prefix;
            tmp.insert(tmp.end(), needle.begin(), needle.end());
            tmp.insert(tmp.end(), suffix.begin(), suffix.end());
            return tmp;
        }();

        auto const haystack_cur = saga::cursor::all(haystack);
        auto const needle_cur = saga::cursor::all(needle);

        auto const r_saga = saga::find_end(haystack_cur, needle_cur);
        auto const r_std = std::find_end(haystack_cur.begin(), haystack_cur.end()
                                         , needle_cur.begin(), needle_cur.end());

        CAPTURE(haystack_cur, needle_cur);

        REQUIRE(std::distance(haystack_cur.begin(), r_saga.begin())
                == std::distance(haystack_cur.begin(), r_std));
        if(r_std != haystack_cur.end())
        {
            REQUIRE(saga::cursor::size(r_saga) == saga::cursor::size(needle_cur));
        }
        REQUIRE(r_saga.dropped_front().begin() == haystack_cur.begin());
        REQUIRE(r_saga.dropped_back().end() == haystack_cur.end());
    };
}

TEST_CASE("find_first_of - minimalistic")
{
    using Value = int;

    saga_test::property_checker
    << [](std::vector<Value> const & haystack_src, std::forward_list<Value> const & needle)
    {
        auto haystack = saga_test::make_istringstream_from_range(haystack_src);

        auto const needle_cur = saga_test::random_subcursor_of(saga::cursor::all(needle));

        auto const r_std = std::find_first_of(haystack_src.begin(), haystack_src.end()
                                              , needle_cur.begin(), needle_cur.end());

        auto r_saga = saga::find_first_of(saga::make_istream_cursor<Value>(haystack), needle_cur);

        REQUIRE(saga::cursor::size(std::move(r_saga)) == (haystack_src.end() - r_std));

        if(!!r_saga)
        {
            REQUIRE(*r_saga == *r_std);
        }
    };
}

TEST_CASE("find_first_of - minimalistic, custom predicate")
{
    using Value = int;

    saga_test::property_checker
    << [](std::vector<Value> const & haystack_src, std::forward_list<Value> const & needle)
    {
        auto haystack = saga_test::make_istringstream_from_range(haystack_src);

        auto const needle_cur = saga_test::random_subcursor_of(saga::cursor::all(needle));

        auto const pred = saga::equivalent_up_to([](Value const & arg) { return arg % 5; });

        auto const r_std = std::find_first_of(haystack_src.begin(), haystack_src.end()
                                              , needle_cur.begin(), needle_cur.end(), pred);

        auto r_saga
            = saga::find_first_of(saga::make_istream_cursor<Value>(haystack), needle_cur, pred);

        REQUIRE(saga::cursor::size(std::move(r_saga)) == (haystack_src.end() - r_std));

        if(!!r_saga)
        {
            REQUIRE(pred(*r_saga, *r_std));
        }
    };
}

TEST_CASE("find_first_of - subcursors")
{
    using Value = int;

    saga_test::property_checker
    << [](std::forward_list<Value> const & haystack, std::list<Value> const & needle)
    {
        auto const needle_cur = saga_test::random_subcursor_of(saga::cursor::all(needle));
        auto const haystack_cur = saga_test::random_subcursor_of(saga::cursor::all(haystack));

        auto const r_std = std::find_first_of(haystack_cur.begin(), haystack_cur.end()
                                              , needle_cur.begin(), needle_cur.end());

        auto const r_saga = saga::find_first_of(haystack_cur, needle_cur);

        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(r_saga.end() == haystack_cur.end());

        REQUIRE(r_saga.dropped_front().begin() == haystack.begin());
        REQUIRE(r_saga.dropped_back().end() == haystack.end());
    };
}

TEST_CASE("find_first_of - subcursors, custom predicate")
{
    using Value = int;

    saga_test::property_checker
    << [](std::list<Value> const & haystack, std::forward_list<Value> const & needle)
    {
        auto const needle_cur = saga_test::random_subcursor_of(saga::cursor::all(needle));
        auto const haystack_cur = saga_test::random_subcursor_of(saga::cursor::all(haystack));

        auto const pred = saga::equivalent_up_to([](Value const & arg) { return arg % 5; });

        auto const r_std = std::find_first_of(haystack_cur.begin(), haystack_cur.end()
                                              , needle_cur.begin(), needle_cur.end(), pred);

        auto const r_saga = saga::find_first_of(haystack_cur, needle_cur, pred);

        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(r_saga.end() == haystack_cur.end());

        REQUIRE(r_saga.dropped_front().begin() == haystack.begin());
        REQUIRE(r_saga.dropped_back().end() == haystack.end());
    };
}

TEST_CASE("adjacent_find - subcursor, default predicate")
{
    using Value = long;

    saga_test::property_checker << [](std::forward_list<Value> const & src)
    {
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const r_std = std::adjacent_find(input.begin(), input.end());
        auto const r_saga = saga::adjacent_find(input);

        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(r_saga.end() == input.end());

        REQUIRE(r_saga.dropped_front().begin() == src.begin());
        REQUIRE(r_saga.dropped_front().end() == r_saga.begin());
    };
}

TEST_CASE("adjacent_find - guaranty, default predicate")
{
    using Value = long;

    saga_test::property_checker << [](std::forward_list<Value> src, Value const & value)
    {
        src.push_front(value);
        src.push_front(value);

        auto const r_saga = saga::adjacent_find(saga::cursor::all(src));

        REQUIRE(r_saga.begin() == src.begin());
        REQUIRE(r_saga.end() == src.end());
    };
}

TEST_CASE("adjacent_find - subcursor, custom predicate")
{
    using Value = long;

    saga_test::property_checker << [](std::forward_list<Value> const & src)
    {
        auto const pred = std::greater<>{};

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const r_std = std::adjacent_find(input.begin(), input.end(), pred);
        auto const r_saga = saga::adjacent_find(input, pred);

        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(r_saga.end() == input.end());

        REQUIRE(r_saga.dropped_front().begin() == src.begin());
        REQUIRE(r_saga.dropped_front().end() == r_saga.begin());
    };
}

TEST_CASE("adjacent_find - guaranty, custom predicate")
{
    using Value = long;

    saga_test::property_checker << [](std::forward_list<Value> src, Value const & value)
    {
        src.push_front(value);
        src.push_front(value);
        src.sort();

        auto const pred = std::less_equal<>{};

        CAPTURE(src);

        auto const r_saga = saga::adjacent_find(saga::cursor::all(src), pred);

        REQUIRE(r_saga.begin() == src.begin());
        REQUIRE(r_saga.end() == src.end());
    };
}

TEST_CASE("search: default predicate, minimalistic")
{
    using Value = int;

    saga_test::property_checker
    <<[](std::forward_list<Value> const & haystack, std::forward_list<Value> const & needle)
    {
        auto const haystack_cur = saga_test::random_subcursor_of(saga::cursor::all(haystack));
        auto const needle_cur = saga_test::random_subcursor_of(saga::cursor::all(needle));

        auto const r_saga = saga::search(haystack_cur, needle_cur);
        auto const r_std = std::search(haystack_cur.begin(), haystack_cur.end()
                                       , needle_cur.begin(), needle_cur.end());

        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(saga::cursor::size(r_saga)
                == (r_std == haystack_cur.end() ? 0 : saga::cursor::size(needle_cur)));
        REQUIRE(r_saga.dropped_front().begin() == haystack_cur.begin());
        REQUIRE(r_saga.dropped_back().end() == haystack_cur.end());
    };
}

TEST_CASE("search: custom predicate, minimalistic")
{
    using Value = int;

    saga_test::property_checker
    <<[](std::forward_list<Value> const & haystack, std::forward_list<Value> const & needle)
    {
        auto const pred = saga::equivalent_up_to([](Value const & arg) { return arg % 2; });

        auto const haystack_cur = saga_test::random_subcursor_of(saga::cursor::all(haystack));
        auto const needle_cur = saga_test::random_subcursor_of(saga::cursor::all(needle));

        auto const r_saga = saga::search(haystack_cur, needle_cur, pred);
        auto const r_std = std::search(haystack_cur.begin(), haystack_cur.end()
                                       , needle_cur.begin(), needle_cur.end(), pred);

        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(saga::cursor::size(r_saga)
                == (r_std == haystack_cur.end() ? 0 : saga::cursor::size(needle_cur)));
        REQUIRE(r_saga.dropped_front().begin() == haystack_cur.begin());
        REQUIRE(r_saga.dropped_back().end() == haystack_cur.end());
    };
}

TEST_CASE("search: default predicate, guaranty")
{
    using Value = int;

    saga_test::property_checker
    <<[](std::list<Value> const & prefix, std::forward_list<Value> const & needle
         , std::list<Value> const & suffix)
    {
        auto const haystack = [&]
        {
            auto tmp = prefix;
            tmp.insert(tmp.end(), needle.begin(), needle.end());
            tmp.insert(tmp.end(), suffix.begin(), suffix.end());
            return tmp;
        }();

        auto const haystack_cur = saga::cursor::all(haystack);
        auto const needle_cur = saga::cursor::all(needle);

        auto const r_saga = saga::search(haystack_cur, needle_cur);
        auto const r_std = std::search(haystack_cur.begin(), haystack_cur.end()
                                       , needle_cur.begin(), needle_cur.end());

        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(saga::cursor::size(r_saga)
                == (r_std == haystack_cur.end() ? 0 : saga::cursor::size(needle_cur)));
        REQUIRE(r_saga.dropped_front().begin() == haystack_cur.begin());
        REQUIRE(r_saga.dropped_back().end() == haystack_cur.end());
    };
}

TEST_CASE("search_n: default predicate, minimalistic")
{
    using Value = int;

    saga_test::property_checker
    <<[](std::forward_list<Value> const & haystack
         , saga_test::container_size<std::ptrdiff_t> const & num, Value const & value)
    {
        auto const haystack_cur = saga_test::random_subcursor_of(saga::cursor::all(haystack));

        auto const r_saga = saga::search_n(haystack_cur, num.value, value);
        auto const r_std = std::search_n(haystack_cur.begin(), haystack_cur.end()
                                         , num.value, value);

        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(saga::cursor::size(r_saga) == (r_std == haystack_cur.end() ? 0 : num.value));
        REQUIRE(r_saga.dropped_front().begin() == haystack_cur.begin());
        REQUIRE(r_saga.dropped_back().end() == haystack_cur.end());
    };
}

TEMPLATE_TEST_CASE("search_n: custom predicate", "search_n", std::forward_list<int>
                   , std::list<int>, std::vector<int>)
{
    using Value = typename TestType::value_type;

    saga_test::property_checker
    <<[](TestType const & haystack
         , saga_test::container_size<typename TestType::difference_type> const & num
         , Value const & value)
    {
        auto const pred = saga::equivalent_up_to([](Value const & arg) { return arg % 2; });

        auto const haystack_cur = saga_test::random_subcursor_of(saga::cursor::all(haystack));

        auto const r_saga = saga::search_n(haystack_cur, num.value, value, pred);
        auto const r_std = std::search_n(haystack_cur.begin(), haystack_cur.end()
                                         , num.value, value, pred);

        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(saga::cursor::size(r_saga) == (r_std == haystack_cur.end() ? 0 : num.value));
        REQUIRE(r_saga.dropped_front().begin() == haystack_cur.begin());
        REQUIRE(r_saga.dropped_back().end() == haystack_cur.end());
    };
}

TEST_CASE("search_n: default predicate, guaranty")
{
    using Value = int;

    saga_test::property_checker
    <<[](std::list<Value> const & prefix, std::list<Value> const & suffix
         , saga_test::container_size<std::ptrdiff_t> const & num, Value const & value)
    {
        auto const haystack = [&]
        {
            auto tmp = prefix;
            std::fill_n(std::inserter(tmp, tmp.end()), num.value, value);
            tmp.insert(tmp.end(), suffix.begin(), suffix.end());

            return tmp;
        }();

        auto const haystack_cur = saga::cursor::all(haystack);

        auto const r_saga = saga::search_n(haystack_cur, num.value, value);
        auto const r_std = std::search_n(haystack_cur.begin(), haystack_cur.end()
                                         , num.value, value);

        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(saga::cursor::size(r_saga) == (r_std == haystack_cur.end() ? 0 : num.value));
        REQUIRE(r_saga.dropped_front().begin() == haystack_cur.begin());
        REQUIRE(r_saga.dropped_back().end() == haystack_cur.end());
    };
}

TEST_CASE("copy")
{
    using Value = int;
    saga_test::property_checker << [](std::vector<Value> const & src,
                                      std::vector<Value> const & dest_old)
    {
        auto dest = dest_old;

        auto const src_cur = saga_test::random_subcursor_of(saga::cursor::all(src));
        auto const dest_cur = saga_test::random_subcursor_of(saga::cursor::all(dest));

        auto const result = saga::copy(src_cur, dest_cur);

        // Проверка содержимого dest
        auto const n = std::min(src_cur.size(), dest_cur.size());

        auto dest_expected = dest_old;
        std::copy(src_cur.begin(), result.in.begin()
                  , saga::rebase_cursor(dest_cur, dest_expected).begin());

        REQUIRE(dest == dest_expected);

        // Проверяем возвращаемое значение
        REQUIRE((!result.in || !result.out));
        REQUIRE(!result.in == (src_cur.size() <= dest_cur.size()));

        REQUIRE(result.in.begin() == src_cur.begin() + n);
        REQUIRE(result.in.end() == src_cur.end());
        REQUIRE(result.in.dropped_front().begin() == src.begin());
        REQUIRE(result.in.dropped_back().end() == src.end());

        REQUIRE(result.out.begin() == dest_cur.begin() + n);
        REQUIRE(result.out.end() == dest_cur.end());
        REQUIRE(result.out.dropped_front().begin() == dest.begin());
        REQUIRE(result.out.dropped_back().end() == dest.end());
    };
}

TEST_CASE("copy: container to back_inserter")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        std::vector<Value> dest;

        auto const result = saga::copy(saga::cursor::all(src), saga::back_inserter(dest));

        REQUIRE(!result.in);
        REQUIRE(!!result.out);

        REQUIRE(dest == src);
    };
}

TEST_CASE("copy: minimal")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto src_in = saga_test::make_istringstream_from_range(src);

        std::vector<Value> dest;

        saga::copy(saga::make_istream_cursor<Value>(src_in), saga::back_inserter(dest));

        REQUIRE(dest == src);
    };
}

TEST_CASE("copy_if: minimal")
{
    using ValueIn = int;
    using ValueOut = long;

    saga_test::property_checker << [](std::vector<ValueIn> const & src)
    {
        auto src_in = saga_test::make_istringstream_from_range(src);
        auto const pred = [](ValueIn const & x) { return x % 2 == 0; };

        // saga
        std::vector<ValueOut> dest_saga;
        saga::copy_if(saga::make_istream_cursor<ValueIn>(src_in)
                      , saga::back_inserter(dest_saga), pred);

        // std
        std::vector<ValueOut> dest_std;
        std::copy_if(src.begin(), src.end(), std::back_inserter(dest_std), pred);

        // Проверка
        REQUIRE(dest_saga == dest_std);
    };
}

TEST_CASE("copy_if: subcursor")
{
    using ValueIn = int;
    using ValueOut = long;

    saga_test::property_checker
    << [](std::vector<ValueIn> const & src, std::vector<ValueOut> const & dest_old)
    {
        auto const pred = [](ValueIn const & x) { return x % 3 == 0; };

        // Подынтервалы
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto dest_saga = dest_old;
        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto const result = saga::copy_if(input, out_saga, pred);

        // std
        auto dest_std = dest_old;

        std::copy_if(input.begin(), result.in.begin()
                     , saga::rebase_cursor(out_saga, dest_std).begin(), pred);

        // Проверка
        CHECK(dest_saga == dest_std);

        auto const n_copied = std::min(saga::count_if(input, pred), out_saga.size());

        REQUIRE((!result.in || !result.out));

        REQUIRE(result.in.begin() - input.begin() >= n_copied);
        REQUIRE(result.in.end() == input.end());
        REQUIRE(result.in.dropped_front().begin() == src.begin());
        REQUIRE(result.in.dropped_back().end() == src.end());

        REQUIRE(result.out.begin() == out_saga.begin() + n_copied);
        REQUIRE(result.out.end() == out_saga.end());
        REQUIRE(result.out.dropped_front().begin() == dest_saga.begin());
        REQUIRE(result.out.dropped_back().end() == dest_saga.end());
    };
}

TEST_CASE("copy_n: minimal")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src
                                      , saga_test::container_size<std::size_t> num)
    {
        auto src_in = saga_test::make_istringstream_from_range(src);

        std::vector<Value> dest;
        saga::copy_n(saga::make_istream_cursor<Value>(src_in), num.value
                     , saga::back_inserter(dest));

        auto const n_common = std::min(src.size(), num.value);

        REQUIRE(dest.size() == n_common);
        REQUIRE(std::equal(dest.begin(), dest.end(), src.begin(), src.begin() + n_common));
    };
}

TEST_CASE("copy_n: subcursor")
{
    using Value = int;

    saga_test::property_checker << [](std::list<Value> const & src
                                      , saga_test::container_size<std::ptrdiff_t> num
                                      , std::vector<Value> const & dest_old)
    {
        // Подготовка
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto dest_saga = dest_old;

        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto const r_saga = saga::copy_n(input, num.value, out_saga);

        // std
        auto const n_common = std::min({num.value, saga::cursor::size(input)
                                       , saga::cursor::size(out_saga)});

        auto dest_std = dest_old;
        auto const out_std = saga::rebase_cursor(out_saga, dest_std);

        std::copy_n(input.begin(), n_common, out_std.begin());

        // Проверка
        REQUIRE(dest_saga == dest_std);

        REQUIRE(r_saga.in == saga::cursor::drop_front_n(input, n_common));
        REQUIRE(r_saga.out == saga::cursor::drop_front_n(out_saga, n_common));
    };
}

TEST_CASE("copy_backward")
{
    using SrcValue = int;
    using DestValue = long;

    saga_test::property_checker
    << [](std::list<SrcValue> const & src, std::list<DestValue> const & dest_old)
    {
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto dest_saga = dest_old;

        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto const r_saga = saga::copy_backward(input, out_saga);

        // std
        auto dest_std = dest_old;

        auto const out_std = saga::rebase_cursor(out_saga, dest_std);

        auto const r_std = std::copy_backward(r_saga.in.end(), input.end(), out_std.end());

        // Проверка
        REQUIRE(dest_saga == dest_std);

        auto const n_common = std::min(saga::cursor::size(input), saga::cursor::size(out_saga));

        REQUIRE(r_saga.in.begin() == input.begin());
        REQUIRE(std::distance(r_saga.in.end(), input.end()) == n_common);
        REQUIRE(r_saga.in.dropped_front().begin() == src.begin());
        REQUIRE(r_saga.in.dropped_back().end() == src.end());

        REQUIRE(r_saga.out.begin() == out_saga.begin());
        REQUIRE(std::distance(r_saga.out.end(), out_saga.end()) == n_common);
        REQUIRE(std::distance(r_saga.out.end(), out_saga.end())
                == std::distance(r_std, out_std.end()));
        REQUIRE(r_saga.out.dropped_front().begin() == dest_saga.begin());
        REQUIRE(r_saga.out.dropped_back().end() == dest_saga.end());
    };
}

TEST_CASE("move: minimal with not moveable input")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto src_in = saga_test::make_istringstream_from_range(src);

        std::vector<Value> dest;

        saga::move(saga::make_istream_cursor<Value>(src_in), saga::back_inserter(dest));

        REQUIRE(dest == src);
    };
}

TEST_CASE("move - subcursors")
{
    using Value = std::vector<int>;
    using Container = std::vector<Value>;

    saga_test::property_checker << [](Container const & src, Container const & dest)
    {
        // saga
        auto src_saga = src;
        auto dest_saga = dest;

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src_saga));
        auto const output = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        std::size_t const src_prefix_size = input.dropped_front().size();
        std::size_t const dest_prefix_size = output.dropped_front().size();
        auto const n_common = std::min(saga::cursor::size(input), saga::cursor::size(output));

        std::vector<Value::const_pointer> addresses;

        for(auto index : saga::cursor::indices(dest_prefix_size))
        {
            addresses.push_back(dest_saga[index].data());
        }

        for(auto index : saga::cursor::indices(n_common))
        {
            addresses.push_back(src_saga.at(src_prefix_size + index).data());
        }

        for(auto index : saga::cursor::indices(dest_prefix_size + n_common, dest.size()))
        {
            addresses.push_back(dest_saga[index].data());
        }

        REQUIRE(addresses.size() == dest.size());

        auto const result_saga = saga::move(input, output);

        // std
        auto src_std = src;

        auto const first_std = src_std.begin() + input.dropped_front().size();
        auto const last_std = first_std + n_common;

        std::vector<Value> dest_std;
        std::move(first_std, last_std, std::back_inserter(dest_std));

        // Проверить возвращаемое значение
        REQUIRE(result_saga.in.begin() == input.begin() + n_common);
        REQUIRE(result_saga.in.end() == input.end());
        REQUIRE(result_saga.out.begin() == output.begin() + n_common);
        REQUIRE(result_saga.out.end() == output.end());

        // Проверить значения
        REQUIRE(src_saga == src_std);

        for(auto index : saga::cursor::indices(dest_prefix_size))
        {
            REQUIRE(dest_saga[index] == dest[index]);
        }

        for(auto index : saga::cursor::indices(n_common))
        {
            REQUIRE(dest_saga[dest_prefix_size + index] == src[src_prefix_size + index]);
        }

        for(auto index : saga::cursor::indices(dest_prefix_size + n_common, dest.size()))
        {
            REQUIRE(dest_saga[index] == dest[index]);
        }

        // Проверить адреса
        for(auto index : saga::cursor::indices_of(dest))
        {
            REQUIRE(dest_saga[index].data() == addresses[index]);
        }
    };
}

TEST_CASE("move_backward: move_only")
{
    using Value = int;

    saga_test::property_checker
    << [](std::list<Value> const & src, std::list<Value> const & dest_old)
    {
        // saga
        std::list<saga_test::move_only<Value>> src_saga(src.begin(), src.end());

        std::list<saga_test::move_only<Value>> dest_saga(dest_old.begin(), dest_old.end());

        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));
        auto const input_saga = saga_test::random_subcursor_of(saga::cursor::all(src_saga));

        auto const r_saga = saga::move_backward(input_saga, out_saga);

        // std
        auto dest_std = dest_old;

        auto const out_std = saga::rebase_cursor(out_saga, dest_std);

        auto const r_std = std::move_backward(saga::rebase_cursor(r_saga.in, src).end()
                                              , saga::rebase_cursor(input_saga, src).end()
                                              , out_std.end());

        // Проверка
        auto const n_common
            = std::min(saga::cursor::size(input_saga), saga::cursor::size(out_saga));

        REQUIRE(r_saga.in.begin() == input_saga.begin());
        REQUIRE(std::distance(r_saga.in.end(), input_saga.end()) == n_common);
        REQUIRE(r_saga.in.dropped_front().begin() == src_saga.begin());
        REQUIRE(r_saga.in.dropped_back().end() == src_saga.end());

        REQUIRE(r_saga.out.begin() == out_saga.begin());
        REQUIRE(std::distance(r_saga.out.end(), out_saga.end()) == n_common);
        REQUIRE(std::distance(r_saga.out.end(), out_saga.end())
                == std::distance(r_std, out_std.end()));
        REQUIRE(r_saga.out.dropped_front().begin() == dest_saga.begin());
        REQUIRE(r_saga.out.dropped_back().end() == dest_saga.end());

        REQUIRE(saga::equal(saga::cursor::all(dest_saga), saga::cursor::all(dest_std)));
    };
}

TEST_CASE("move_backward - vectors")
{
    using Value = std::vector<int>;
    using Container = std::vector<Value>;

    saga_test::property_checker << [](Container const & src, Container const & dest)
    {
        // saga
        auto src_saga = src;
        auto dest_saga = dest;

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src_saga));
        auto const output = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto const n_common = std::min(saga::cursor::size(input), saga::cursor::size(output));

        std::size_t const dest_prefix_size
            = dest.size() - output.dropped_back().size() - n_common;
        auto const src_prefix_size = src.size() - input.dropped_back().size() - n_common;

        std::vector<Value::const_pointer> addresses;

        for(auto index : saga::cursor::indices(dest_prefix_size))
        {
            addresses.push_back(dest_saga[index].data());
        }

        for(auto index : saga::cursor::indices(n_common))
        {
            addresses.push_back(src_saga.at(src_prefix_size + index).data());
        }

        for(auto index : saga::cursor::indices(dest_saga.size() - output.dropped_back().size()
                                             , dest_saga.size()))
        {
            addresses.push_back(dest_saga[index].data());
        }

        REQUIRE(addresses.size() == dest.size());

        auto const result_saga = saga::move_backward(input, output);

        // std
        auto src_std = src;

        auto const last_std = src_std.end() - input.dropped_back().size();
        auto const first_std = last_std - n_common;

        std::vector<Value> dest_std(n_common);
        std::move_backward(first_std, last_std, dest_std.end());

        // Проверить возвращаемое значение
        REQUIRE(result_saga.in.begin() == input.begin());
        REQUIRE(result_saga.in.end() == input.end() - n_common);
        REQUIRE(result_saga.out.begin() == output.begin());
        REQUIRE(result_saga.out.end() == output.end() - n_common);

        // Проверить значения
        REQUIRE(src_saga == src_std);

        for(auto index : saga::cursor::indices(dest_prefix_size))
        {
            REQUIRE(dest_saga.at(index) == dest.at(index));
        }

        for(auto index : saga::cursor::indices(n_common))
        {
            REQUIRE(dest_saga.at(dest_prefix_size + index)
                    == src.at(src_prefix_size + index));
        }

        for(auto index : saga::cursor::indices(dest_saga.size() - output.dropped_back().size()
                                             , dest_saga.size()))
        {
            REQUIRE(dest_saga.at(index) == dest.at(index));
        }

        // Проверить адреса
        for(auto index : saga::cursor::indices_of(dest))
        {
            REQUIRE(dest_saga[index].data() == addresses[index]);
        }
    };
}

TEST_CASE("fill: minimal")
{
    using Value = int;

    saga_test::property_checker
    << [](Value const & value, saga_test::container_size<std::size_t> count)
    {
        std::vector<Value> dest;

        auto const output = saga::cursor::take(saga::back_inserter(dest), count.value);

        saga::fill(output, value);

        REQUIRE(dest.size() == count.value);

        REQUIRE(dest == std::vector<Value>(count.value, value));
    };
}

TEST_CASE("fill - subrange")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src, Value const & value)
    {
        // saga
        std::forward_list<Value> xs_saga(src.begin(), src.end());

        auto const sub_saga = saga_test::random_subcursor_of(saga::cursor::all(xs_saga));

        saga::fill(sub_saga, value);

        // Проверка
        auto const sub_src = saga::rebase_cursor(sub_saga, src);

        CHECK(saga::equal(sub_saga.dropped_front(), sub_src.dropped_front()));

        CHECK(saga::all_of(sub_saga, [&](auto const & arg) { return arg == value; }));

        CHECK(saga::equal(sub_saga.dropped_back(), sub_src.dropped_back()));
    };
}

TEST_CASE("fill_n: mimimal")
{
    using Value = int;

    saga_test::property_checker
    << [](saga_test::container_size<std::size_t> num, Value const & value)
    {
        std::vector<Value> dest;
        saga::fill_n(saga::back_inserter(dest), num.value, value);

        REQUIRE(dest.size() == num.value);

        CHECK(saga::all_of(saga::cursor::all(dest), [&](auto const & arg) {return arg == value;}));
    };
}

TEST_CASE("fill_n - subrange")
{
    using Value = int;

    saga_test::property_checker << [](std::list<Value> const & src
                                      , saga_test::container_size<std::ptrdiff_t> num
                                      , Value const & value)
    {
        // saga
        std::vector<Value> xs_saga(src.begin(), src.end());

        auto const sub_saga = saga_test::random_subcursor_of(saga::cursor::all(xs_saga));

        auto const result = saga::fill_n(sub_saga, num.value, value);

        // std
        std::vector<Value> xs_std(src.begin(), src.end());

        auto const n_common = std::min(sub_saga.size(), num.value);

        auto const sub_std = saga::rebase_cursor(sub_saga, xs_std);

        std::fill_n(sub_std.begin(), n_common, value);

        // Проверка
        CHECK(result == saga::cursor::drop_front_n(sub_saga, n_common));

        CHECK(xs_saga == xs_std);
    };
}

TEST_CASE("transform: minimal")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src, int mask)
    {
        auto fun = [&](Value const & arg) { return arg ^ mask; };

        // std
        std::vector<Value> result_std;
        std::transform(src.begin(), src.end(), saga::back_inserter(result_std), fun);

        // saga
        auto src_istream = saga_test::make_istringstream_from_range(src);

        std::vector<Value> result_saga;
        saga::transform(saga::make_istream_cursor<Value>(src_istream)
                        , saga::back_inserter(result_saga), fun);

        REQUIRE(result_saga == result_std);
    };
}

TEST_CASE("transform")
{
    saga_test::property_checker << [](std::vector<char> const & src, std::string const & dest)
    {
        auto fun = SAGA_OVERLOAD_SET(std::toupper);

        auto const n_common = std::min(src.size(), dest.size());
        // std
        auto dest_std = dest;
        std::transform(src.begin(), src.begin() + n_common, dest_std.begin(), fun);

        // saga
        auto dest_saga = dest;
        auto r_saga
            = saga::transform(saga::cursor::all(src), saga::cursor::all(dest_saga), fun);

        REQUIRE(dest_saga == dest_std);

        REQUIRE(r_saga.in.begin() == src.begin() + n_common);
        REQUIRE(r_saga.in.end() == src.end());
        REQUIRE(r_saga.out.begin() == dest_saga.begin() + n_common);
        REQUIRE(r_saga.out.end() == dest_saga.end());
    };
}

namespace
{
    struct multiplies
    {
        constexpr int operator()(int x, int y) const
        {
            return x*y;
        }
    };

    constexpr bool check_unary_transform_constexpr()
    {
        int arr[] = {1, 2, 3, 4, 5};
        const int expected[] = {1, 4, 9, 16, 25};

        saga::transform(saga::cursor::all(arr), saga::cursor::all(arr), saga::square);

        return saga::equal(saga::cursor::all(arr), saga::cursor::all(expected));
    }

    constexpr bool check_binary_transform_constexpr()
    {
        int lhs[] = {1, 2, 3, 4, 5};
        int rhs[] = {2, 4, 6, 8, 10};

        const int expected[] = {2, 8, 18, 32, 50};

        saga::transform(saga::cursor::all(lhs), saga::cursor::all(rhs),
                        saga::cursor::all(lhs), ::multiplies{});

        return saga::equal(saga::cursor::all(lhs), saga::cursor::all(expected));
    }
}

TEST_CASE("transform constexpr")
{
    static_assert(check_unary_transform_constexpr(), "");
    static_assert(check_binary_transform_constexpr(), "");
}

TEST_CASE("transform binary")
{
    saga_test::property_checker << [](std::vector<char> const & lhs,
                                      std::string const & rhs,
                                      std::string const & dest_old)
    {
        auto const n_common = std::min({lhs.size(), rhs.size(), dest_old.size()});
        auto const fun = SAGA_OVERLOAD_SET(std::min);

        // std
        auto dest_std = dest_old;
        std::transform(lhs.begin(), lhs.begin() + n_common, rhs.begin(), dest_std.begin(), fun);

        // saga
        auto dest_saga = dest_old;

        auto const result_saga = saga::transform(saga::cursor::all(lhs), saga::cursor::all(rhs)
                                                 , saga::cursor::all(dest_saga), fun);

        // Проверка результатов
        REQUIRE(dest_saga == dest_std);

        REQUIRE(result_saga.in1.begin() == lhs.begin() + n_common);
        REQUIRE(result_saga.in1.end() == lhs.end());

        REQUIRE(result_saga.in2.begin() == rhs.begin() + n_common);
        REQUIRE(result_saga.in2.end() == rhs.end());

        REQUIRE(result_saga.out.begin() == dest_saga.begin() + n_common);
        REQUIRE(result_saga.out.end() == dest_saga.end());
    };
}

TEST_CASE("generate - minimal")
{
    using Value = int;

    struct Iota
    {
        Value operator()()
        {
            return ++value_;
        }

    private:
        Value value_ = 0;
    };

    saga_test::property_checker << [](saga_test::container_size<std::size_t> const & count)
    {
        // generate + take
        std::vector<Value> dest;
        saga::generate(saga::cursor::take(saga::back_inserter(dest), count.value), Iota{});

        // generete_n
        std::vector<Value> dest_expected;
        saga::generate_n(saga::back_inserter(dest_expected), count.value, Iota{});

        // Проверка
        REQUIRE(dest == dest_expected);
    };
}

TEST_CASE("generate - subrange")
{
    using Value = int;

    struct Iota
    {
        Value operator()()
        {
            return ++value_;
        }

    private:
        Value value_ = 0;
    };

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        // saga
        std::forward_list<Value> xs_saga(src.begin(), src.end());

        auto const sub_saga = saga_test::random_subcursor_of(saga::cursor::all(xs_saga));

        saga::generate(sub_saga, Iota{});

        // std
        std::forward_list<Value> xs_std(src.begin(), src.end());

        auto const n_front = std::distance(xs_saga.begin(), sub_saga.begin());

        auto const std_first = std::next(xs_std.begin(), n_front);
        auto const std_last = std::next(std_first, saga::cursor::size(sub_saga));

        std::generate(std_first, std_last, Iota{});

        // Проверка
        REQUIRE(xs_saga == xs_std);
    };
}

TEST_CASE("generate_n - minimal")
{
    using Value = int;

    struct Iota
    {
        Value operator()()
        {
            return ++value_;
        }

    private:
        Value value_ = 0;
    };

    saga_test::property_checker << [](saga_test::container_size<std::size_t> num)
    {
        // saga
        std::vector<Value> dest_saga;
        saga::generate_n(saga::back_inserter(dest_saga), num.value, Iota{});

        REQUIRE(dest_saga.size() == num.value);

        // std
        std::vector<Value> dest_std;
        std::generate_n(saga::back_inserter(dest_std), num.value, Iota{});

        // Проверка
        REQUIRE(dest_saga == dest_std);
    };
}

TEST_CASE("generate_n - subrange")
{
    using Value = int;

    struct Iota
    {
        Value operator()()
        {
            return ++value_;
        }

    private:
        Value value_ = 0;
    };

    saga_test::property_checker << [](std::list<Value> const & src
                                      , saga_test::container_size<std::ptrdiff_t> num)
    {
        // saga
        std::vector<Value> xs_saga(src.begin(), src.end());

        auto const sub_saga = saga_test::random_subcursor_of(saga::cursor::all(xs_saga));

        auto const result = saga::generate_n(sub_saga, num.value, Iota{});

        // std
        std::vector<Value> xs_std(src.begin(), src.end());

        auto const n_common = std::min(sub_saga.size(), num.value);

        auto const sub_std = saga::rebase_cursor(sub_saga, xs_std);

        std::generate_n(sub_std.begin(), n_common, Iota{});

        // Проверка
        CHECK(result == saga::cursor::drop_front_n(sub_saga, n_common));

        CHECK(xs_saga == xs_std);
    };
}

TEST_CASE("remove")
{
    using Value = int;

    saga_test::property_checker << [](std::forward_list<Value> const & src_old, Value const & value)
    {
        // saga
        auto src_saga = [&]()
        {
            std::forward_list<saga_test::move_only<Value>> src_saga;
            auto pos = src_saga.before_begin();

            for(auto const & value : src_old)
            {
                pos = src_saga.emplace_after(pos, value);
            }

            return src_saga;
        }();

        auto const cur_saga = saga_test::random_subcursor_of(saga::cursor::all(src_saga));

        auto const result_saga = saga::remove(cur_saga, value);

        // std
        auto src_std = src_old;
        auto const cur_std = saga::rebase_cursor(cur_saga, src_std);

        auto const result_std = std::remove(cur_std.begin(), cur_std.end(), value);

        // Проверка
        REQUIRE(std::none_of(cur_saga.begin(), result_saga.begin(),
                             [&](saga_test::move_only<Value>  const & arg)
                                { return arg == value; }));

        REQUIRE(std::distance(src_saga.begin(), result_saga.begin())
                == std::distance(src_std.begin(), result_std));

        REQUIRE(saga::equal(result_saga.dropped_front()
                           , saga::rebase_cursor(result_saga, src_std).dropped_front()));

        CHECK(saga::equal(cur_saga.dropped_back(), cur_std.dropped_back()));
    };
}

TEST_CASE("remove: guaranteed")
{
    using Value = int;

    saga_test::property_checker << [](std::forward_list<Value> src_old, Value const & value)
    {
        src_old.push_front(value);

        // saga
        auto src_saga = src_old;

        auto const cur_saga = saga::cursor::all(src_saga);

        auto const result_saga = saga::remove(cur_saga, value);

        // std
        auto src_std = src_old;
        auto const cur_std = saga::rebase_cursor(cur_saga, src_std);

        auto const result_std = std::remove(cur_std.begin(), cur_std.end(), value);

        // Проверка
        REQUIRE(std::none_of(cur_saga.begin(), result_saga.begin(),
                             [&](Value const & arg) { return arg == value; }));

        REQUIRE(std::distance(src_saga.begin(), result_saga.begin())
                == std::distance(src_std.begin(), result_std));

        REQUIRE(saga::equal(result_saga.dropped_front()
                           , saga::rebase_cursor(result_saga, src_std).dropped_front()));

        CHECK(saga::equal(cur_saga.dropped_back(), cur_std.dropped_back()));
    };
}

TEST_CASE("remove: subcursor, custom binary predicate")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> src_old, Value const & value)
    {
        auto const bin_pred = std::less<>{};
        auto const pred = [&](Value const & arg) { return bin_pred(arg, value); };

        auto src_bin = src_old;
        saga::remove(saga::cursor::all(src_bin), value, bin_pred);

        auto src_if = src_old;
        saga::remove_if(saga::cursor::all(src_if), pred);

        REQUIRE(src_bin == src_if);
    };
}

TEST_CASE("remove_if")
{
    using Value = int;

    saga_test::property_checker << [](std::forward_list<Value> const & src_old)
    {
        auto const pred_std = [](Value const & arg) { return arg % 3 == 2; };

        // saga
        auto src_saga = [&]()
        {
            std::forward_list<saga_test::move_only<Value>> src_saga;
            auto pos = src_saga.before_begin();

            for(auto const & value : src_old)
            {
                pos = src_saga.emplace_after(pos, value);
            }

            return src_saga;
        }();

        auto const cur_saga = saga_test::random_subcursor_of(saga::cursor::all(src_saga));

        auto const pred_saga = [&pred_std](saga_test::move_only<Value> const & arg)
                                { return pred_std(arg.value); };

        auto const result_saga = saga::remove_if(cur_saga, pred_saga);

        // std
        auto src_std = src_old;
        auto const cur_std = saga::rebase_cursor(cur_saga, src_std);

        auto const result_std = std::remove_if(cur_std.begin(), cur_std.end(), pred_std);

        // Проверка
        REQUIRE(std::none_of(cur_saga.begin(), result_saga.begin(), pred_saga));

        REQUIRE(std::distance(src_saga.begin(), result_saga.begin())
                == std::distance(src_std.begin(), result_std));

        REQUIRE(saga::equal(result_saga.dropped_front()
                           , saga::rebase_cursor(result_saga, src_std).dropped_front()));

        CHECK(saga::equal(cur_saga.dropped_back(), cur_std.dropped_back()));
    };
}

TEST_CASE("remove_copy: minimal")
{
    using Value = long;

    saga_test::property_checker << [](std::vector<Value> const & src, Value const & value)
    {
        // saga
        auto src_saga = saga_test::make_istringstream_from_range(src);

        std::vector<Value> dest_saga;
        saga::remove_copy(saga::make_istream_cursor<Value>(src_saga)
                          , saga::back_inserter(dest_saga), value);

        // std
        std::vector<Value> dest_std;
        std::remove_copy(src.begin(), src.end(), std::back_inserter(dest_std), value);

        // Сравнение
        REQUIRE(dest_saga == dest_std);
    };
}

TEST_CASE("remove_copy: subcursors")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src
                                      , std::vector<Value> const & dest_old, Value const & value)
    {
        // Подготовка
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto dest_saga = dest_old;
        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto const result_saga = saga::remove_copy(input, out_saga, value);

        // std
        auto dest_std = dest_old;
        auto const out_std = dest_std.begin() + (out_saga.begin() - dest_saga.begin());

        auto const result_std = std::remove_copy(input.begin(), result_saga.in.begin()
                                                 , out_std, value);

        // Проверки
        REQUIRE(dest_saga == dest_std);

        REQUIRE(result_saga.in.end() == input.end());

        REQUIRE((result_saga.out.begin() - out_saga.begin()) == (result_std - out_std));
        REQUIRE(result_saga.out.end() == out_saga.end());
    };
}

TEST_CASE("remove_copy_if: minimal")
{
    using Value = long;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto const pred = [](Value const & x) { return x % 2 == 0; };

        // saga
        auto src_saga = saga_test::make_istringstream_from_range(src);

        std::vector<Value> dest_saga;
        saga::remove_copy_if(saga::make_istream_cursor<Value>(src_saga)
                             , saga::back_inserter(dest_saga), pred);

        // std
        std::vector<Value> dest_std;
        std::remove_copy_if(src.begin(), src.end(), std::back_inserter(dest_std), pred);

        // Сравнение
        REQUIRE(dest_saga == dest_std);
    };
}

TEST_CASE("remove_copy_if: subcursors")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src
                                      , std::vector<Value> const & dest_old)
    {
        auto const pred = [&](Value const & x) { return x % 3 == 0; };

        // Подготовка
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto dest_saga = dest_old;
        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto const result_saga = saga::remove_copy_if(input, out_saga, pred);

        // std
        auto dest_std = dest_old;
        auto const out_std = dest_std.begin() + (out_saga.begin() - dest_saga.begin());

        auto const result_std = std::remove_copy_if(input.begin(), result_saga.in.begin()
                                                    , out_std, pred);

        // Проверки
        REQUIRE(dest_saga == dest_std);

        REQUIRE(result_saga.in.end() == input.end());

        REQUIRE((result_saga.out.begin() - out_saga.begin()) == (result_std - out_std));
        REQUIRE(result_saga.out.end() == out_saga.end());
    };
}

TEST_CASE("remove_copy: custom binary predicate")
{
    using Value = long;

    saga_test::property_checker << [](std::vector<Value> const & src, Value const & value)
    {
        auto const bin_pred = saga::equivalent_up_to([](Value const & arg) { return arg % 7;});

        auto const pred = [&](Value const & arg) {   return bin_pred(arg, value); };

        std::vector<Value> dest_bin;
        saga::remove_copy(saga::cursor::all(src), saga::back_inserter(dest_bin), value, bin_pred);

        std::vector<Value> dest_if;
        saga::remove_copy_if(saga::cursor::all(src), saga::back_inserter(dest_if), pred);

        // Сравнение
        REQUIRE(dest_bin == dest_if);
    };
}

TEST_CASE("replace")
{
    using Value = int;
    using NewValue = short;

    saga_test::property_checker
    << [](std::forward_list<Value> const & src
          , NewValue const & old_value, NewValue const & new_value)
    {
        // saga
        auto src_saga = src;

        auto const in_saga = saga_test::random_subcursor_of(saga::cursor::all(src_saga));

        saga::replace(in_saga, old_value, new_value);

        // std
        auto src_std = src;

        auto const first_std
            = std::next(src_std.begin(), std::distance(src_saga.begin(), in_saga.begin()));
        auto const last_std = std::next(first_std, saga::cursor::size(in_saga));

        std::replace(first_std, last_std, old_value, new_value);

        // Сравнение
        REQUIRE(src_saga == src_std);
    };
}

TEST_CASE("replace_if")
{
    using OldValue = long;
    using NewValue = int;

    saga_test::property_checker
    << [](std::forward_list<OldValue> const & src, NewValue const & new_value)
    {
        auto const pred = [](OldValue const & x) { return x % 2 == 0; };

        // saga
        auto src_saga = src;

        auto const in_saga = saga_test::random_subcursor_of(saga::cursor::all(src_saga));

        saga::replace_if(in_saga, pred, new_value);

        // std
        auto src_std = src;

        auto const first_std
            = std::next(src_std.begin(), std::distance(src_saga.begin(), in_saga.begin()));
        auto const last_std = std::next(first_std, saga::cursor::size(in_saga));

        std::replace_if(first_std, last_std, pred, new_value);

        // Сравнение
        REQUIRE(src_saga == src_std);
    };
}

TEST_CASE("replace: subcursor, custom binary predicate")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> src_old
                                      , Value const & old_value, Value const & new_value)
    {
        auto const bin_pred = std::less<>{};
        auto const pred = [&](Value const & arg) { return bin_pred(arg, old_value); };

        auto src_bin = src_old;
        saga::replace(saga::cursor::all(src_bin), old_value, new_value, bin_pred);

        auto src_if = src_old;
        saga::replace_if(saga::cursor::all(src_if), pred, new_value);

        REQUIRE(src_bin == src_if);
    };
}

TEST_CASE("replace_copy: minimal")
{
    using OldValue = long;
    using NewValue = int;

    saga_test::property_checker << [](std::vector<OldValue> const & src
                                      , OldValue const & old_value, NewValue const & new_value)
    {
        // saga
        auto src_saga = saga_test::make_istringstream_from_range(src);

        std::vector<OldValue> dest_saga;
        saga::replace_copy(saga::make_istream_cursor<OldValue>(src_saga)
                          , saga::back_inserter(dest_saga), old_value, new_value);

        // std
        std::vector<OldValue> dest_std;
        std::replace_copy(src.begin(), src.end(), std::back_inserter(dest_std)
                          , old_value, OldValue{new_value});

        // Сравнение
        REQUIRE(dest_saga == dest_std);
    };
}

TEST_CASE("replace_copy: subcursors")
{
    using OldValue = long;
    using NewValue = int;

    saga_test::property_checker << [](std::vector<OldValue> const & src
                                      , std::vector<OldValue> const & dest_old
                                      , OldValue const & old_value
                                      , NewValue const & new_value)
    {
        // Подготовка
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto dest_saga = dest_old;
        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto const result_saga = saga::replace_copy(input, out_saga, old_value, new_value);

        // std
        auto dest_std = dest_old;
        auto const out_std = dest_std.begin() + (out_saga.begin() - dest_saga.begin());

        auto const result_std = std::replace_copy(input.begin(), result_saga.in.begin()
                                                 , out_std, old_value, OldValue{new_value});

        // Проверки
        REQUIRE(dest_saga == dest_std);

        REQUIRE(result_saga.in.end() == input.end());

        REQUIRE((result_saga.out.begin() - out_saga.begin()) == (result_std - out_std));
        REQUIRE(result_saga.out.end() == out_saga.end());
    };
}

TEST_CASE("replace_copy_if : minimal")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src, Value const & new_value)
    {
        auto const pred = [](Value const & x) { return x % 2 == 0; };

        // saga
        auto src_saga = saga_test::make_istringstream_from_range(src);

        std::vector<Value> dest_saga;
        saga::replace_copy_if(saga::make_istream_cursor<Value>(src_saga)
                              , saga::back_inserter(dest_saga), pred, new_value);

        // std
        std::vector<Value> dest_std;
        std::replace_copy_if(src.begin(), src.end(), std::back_inserter(dest_std), pred, new_value);

        // Сравнение
        REQUIRE(dest_saga == dest_std);
    };
}

TEST_CASE("replace_copy_if: subcursors")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src
                                      , std::vector<Value> const & dest_old
                                      , Value const & new_value)
    {
        auto const pred = [&](Value const & x) { return x % 3 == 0; };

        // Подготовка
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto dest_saga = dest_old;
        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto const result_saga = saga::replace_copy_if(input, out_saga, pred, new_value);

        // std
        auto dest_std = dest_old;
        auto const out_std = dest_std.begin() + (out_saga.begin() - dest_saga.begin());

        auto const result_std = std::replace_copy_if(input.begin(), result_saga.in.begin()
                                                    , out_std, pred, new_value);

        // Проверки
        REQUIRE(dest_saga == dest_std);

        REQUIRE(result_saga.in.end() == input.end());

        REQUIRE((result_saga.out.begin() - out_saga.begin()) == (result_std - out_std));
        REQUIRE(result_saga.out.end() == out_saga.end());
    };
}

TEST_CASE("replace_copy: custom binary predicate")
{
    using Value = long;

    saga_test::property_checker
    << [](std::vector<Value> const & src, Value const & old_value, Value const & new_value)
    {
        auto const bin_pred = saga::equivalent_up_to([](Value const & arg) { return arg % 7;});

        auto const pred = [&](Value const & arg) {   return bin_pred(arg, old_value); };

        std::vector<Value> dest_bin;
        saga::replace_copy(saga::cursor::all(src), saga::back_inserter(dest_bin)
                           , old_value, new_value, bin_pred);

        std::vector<Value> dest_if;
        saga::replace_copy_if(saga::cursor::all(src), saga::back_inserter(dest_if)
                              , pred, new_value);

        // Сравнение
        REQUIRE(dest_bin == dest_if);
    };
}

namespace
{
    template <class Container1, class Container2>
    void check_swap_ranges(Container1 const & lhs_old, Container2 const & rhs_old)
    {
        // saga
        auto lhs_saga = lhs_old;
        auto const lhs_saga_cur = saga_test::random_subcursor_of(saga::cursor::all(lhs_saga));

        auto rhs_saga = rhs_old;
        auto const rhs_saga_cur = saga_test::random_subcursor_of(saga::cursor::all(rhs_saga));

        auto const r_saga = saga::swap_ranges(lhs_saga_cur, rhs_saga_cur);

        // std
        auto lhs_std = lhs_old;
        auto const lhs_std_cur = saga::rebase_cursor(lhs_saga_cur, lhs_std);

        auto rhs_std = rhs_old;
        auto const rhs_std_cur = saga::rebase_cursor(rhs_saga_cur, rhs_std);

        auto const n_common = std::min(saga::cursor::size(lhs_std_cur)
                                       , saga::cursor::size(rhs_std_cur));

        auto const r_std = std::swap_ranges(lhs_std_cur.begin()
                                            , std::next(lhs_std_cur.begin(), n_common)
                                            , rhs_std_cur.begin());
        // Сравнение
        REQUIRE(lhs_saga == lhs_std);
        REQUIRE(rhs_saga == rhs_std);

        REQUIRE(r_saga.in1 == saga::cursor::drop_front_n(lhs_saga_cur, n_common));
        REQUIRE(r_saga.in2 == saga::cursor::drop_front_n(rhs_saga_cur, n_common));

        REQUIRE(saga::rebase_cursor(r_saga.in2, rhs_std).begin() == r_std);
    }
}

TEST_CASE("swap_ranges")
{
    using Value = int;

    saga_test::property_checker
        << ::check_swap_ranges<std::forward_list<Value>, std::forward_list<Value>>
        << ::check_swap_ranges<std::list<Value>, std::vector<Value>>;
}

TEST_CASE("reverse : whole container")
{
    using Container = std::list<int>;

    saga_test::property_checker << [](Container const & src)
    {
        auto src_std = src;
        std::reverse(src_std.begin(), src_std.end());

        auto src_saga = src;
        saga::reverse(saga::cursor::all(src_saga));

        REQUIRE(src_std == src_saga);
    };
}

TEST_CASE("reverse : subrange")
{
    using Container = std::list<int>;

    saga_test::property_checker << [](Container const & values_old)
    {
        auto values = values_old;

        auto const src = saga_test::random_subcursor_of(saga::cursor::all(values));

        std::reverse(src.begin(), src.end());

        saga::reverse(saga::cursor::all(src));

        REQUIRE(values == values_old);
    };
}

TEST_CASE("reverse_copy : to back_inserter")
{
    using Element = int;
    using Container = std::list<Element>;

    saga_test::property_checker << [](Container const & src)
    {
        auto const src_sub = saga_test::random_subcursor_of(saga::cursor::all(src));

        // std
        Container out_std;
        std::reverse_copy(src_sub.begin(), src_sub.end(), std::back_inserter(out_std));

        // saga
        Container out_saga;
        saga::reverse_copy(src_sub, saga::back_inserter(out_saga));

        // Сравнение результатов
        REQUIRE(out_std == out_saga);
    };
}

TEST_CASE("reverse_copy : subcontainer to subcontainer")
{
    saga_test::property_checker << [](std::vector<int> const & src_container,
                                      std::vector<long> dest_container)
    {
        static_assert(!std::is_same<decltype(src_container), decltype(dest_container)>{}, "");

        auto const src = saga_test::random_subcursor_of(saga::cursor::all(src_container));
        auto const dest = saga_test::random_subcursor_of(saga::cursor::all(dest_container));

        auto dest_container_std = dest_container;

        // saga
        auto const result = saga::reverse_copy(src, dest);

        static_assert(std::is_same<decltype(saga::reverse_copy(src, dest))
                                  ,saga::in_out_result<std::remove_cv_t<decltype(src)>
                                                      ,std::remove_cv_t<decltype(dest)>>>{},"");

        // std
        auto const n_common = std::min(src.size(), dest.size());

        std::reverse_copy(src.end() - n_common, src.end(),
                          dest_container_std.begin() + (dest.begin() - dest_container.begin()));

        // Сравнение
        CAPTURE(src_container);

        REQUIRE(dest_container == dest_container_std);

        REQUIRE(result.in.begin() == src.begin());
        REQUIRE(result.in.end() == src.end() - n_common);

        REQUIRE(result.out.begin() == dest.begin() + n_common);
        REQUIRE(result.out.end() == dest.end());
    };
}

namespace
{
    template <class T, std::size_t N>
    constexpr bool check_reverse_copy_constexpr(T(&arr)[N])
    {
        std::remove_cv_t<T> result[N] = {};
        saga::reverse_copy(saga::cursor::all(arr), saga::cursor::all(result));

        return saga::equal(saga::cursor::all(result)
                          , saga::make_subrange_cursor(saga::make_reverse_iterator(std::end(arr))
                                                      , saga::make_reverse_iterator(std::begin(arr))
                                                      , saga::unsafe_tag_t{}));
    }
}

TEST_CASE("reverse_copy : constexpr")
{
    constexpr int values[] = {1, 2, 3, 4, 5};

    static_assert(::check_reverse_copy_constexpr(values), "");
}

TEST_CASE("rotate: forward")
{
    using Value = int;

    saga_test::property_checker << [](std::forward_list<Value> const & values_old)
    {
        // saga
        auto values_saga = values_old;

        auto const input_saga = saga_test::random_subcursor_of(saga::cursor::all(values_saga));
        auto const r_saga = saga::rotate(input_saga);

        // std
        auto values_std = values_old;

        auto const input_std = saga::rebase_cursor(input_saga, values_std);
        auto const r_std
            = std::rotate(input_std.dropped_front().begin(), input_std.begin(), input_std.end());

        // Сравнение
        CAPTURE(values_old, input_saga.dropped_front(), input_saga, input_saga.dropped_back());

        REQUIRE(values_saga == values_std);

        REQUIRE(saga::rebase_cursor(r_saga, values_std).begin() == r_std);
        REQUIRE(r_saga.end() == input_saga.end());
        REQUIRE(r_saga.dropped_front().begin() == input_saga.dropped_front().begin());
        REQUIRE(r_saga.dropped_back().end() == input_saga.dropped_back().end());
    };
}

TEMPLATE_LIST_TEST_CASE("rotate_copy: minimal", "rotate_copy", Containers)
{
    using Container = TestType;
    using Value = typename Container::value_type;

    saga_test::property_checker << [](Container const & src)
    {
        auto const cur = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        std::vector<Value> dest_saga;
        saga::rotate_copy(cur, saga::back_inserter(dest_saga));

        // std
        std::vector<Value> dest_std;
        std::rotate_copy(src.begin(), cur.begin(), cur.end(), std::back_inserter(dest_std));

        // Сравнение
        CAPTURE(src, cur.dropped_front(), cur);
        REQUIRE(dest_saga == dest_std);
    };
}

TEST_CASE("rotate_copy: subcursors")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src
                                      , std::vector<Value> const & dest_old)
    {
        auto const cur = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto dest_saga = dest_old;
        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto const result = saga::rotate_copy(cur, out_saga);

        // std
        auto dest_std = dest_old;
        auto const out_std = dest_std.begin() + (out_saga.begin() - dest_saga.begin());
        auto const r1_std = std::copy(cur.begin(), result.in2.begin(), out_std);
        auto const r2_std = std::copy(src.begin(), result.in1.begin(), r1_std);

        // Сравнение
        REQUIRE(dest_saga == dest_std);
        REQUIRE((result.out.begin() - out_saga.begin()) == (r2_std - out_std));
        REQUIRE(result.out.end() == out_saga.end());
    };
}

TEST_CASE("shuffle")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & values_old)
    {
        auto values = values_old;

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(values));

        saga::shuffle(input, saga_test::random_engine());

        // Проверки
        auto const input_old = saga::rebase_cursor(input, values_old);

        REQUIRE(saga::is_permutation(input, input_old));

        REQUIRE(saga::equal(input.dropped_front(), input_old.dropped_front()));
        REQUIRE(saga::equal(input.dropped_back(), input_old.dropped_back()));
    };
}

TEST_CASE("sample: Input -> RandomAccess")
{
    using Value = long;

    saga_test::property_checker
    << [](std::vector<Value> const & src, std::vector<Value> const & dest_old)
    {
        auto dest = dest_old;

        auto src_in = saga_test::make_istringstream_from_range(src);

        auto const out = saga_test::random_subcursor_of(saga::cursor::all(dest));

        auto const num
            = static_cast<std::size_t>(saga_test::random_uniform(0, out.size()));

        auto const result = saga::sample(saga::make_istream_cursor<Value>(src_in)
                                         , out, num, saga_test::random_engine());

        // Проверки
        REQUIRE(result == saga::cursor::drop_front_n(out, std::min(num, src.size())));

        for(auto iter = out.begin(); iter != result.begin(); ++iter)
        {
            REQUIRE(!!saga::find(saga::cursor::all(src), *iter));
        }
    };
}

TEST_CASE("sample: Forward -> RandomAccess")
{
    using Value = long;

    saga_test::property_checker
    << [](std::forward_list<Value> const & src, std::vector<Value> const & dest_old)
    {
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto dest = dest_old;
        auto const out = saga_test::random_subcursor_of(saga::cursor::all(dest));

        auto const num = saga_test::random_uniform(0, out.size());

        auto const result = saga::sample(input, out, num, saga_test::random_engine());

        // Проверки
        REQUIRE(result == saga::cursor::drop_front_n(out, std::min(num, saga::cursor::size(input))));

        {
            auto pos = input;

            for(auto iter = out.begin(); iter != result.begin(); ++iter)
            {
                pos = saga::find(pos, *iter);

                REQUIRE(!!pos);
            }
        }
    };
}

TEST_CASE("sample: Forward -> Output")
{
    using Value = long;

    saga_test::property_checker
    << [](std::forward_list<Value> const & src, saga_test::container_size<std::ptrdiff_t> num)
    {
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        std::vector<Value> dest;
        saga::sample(input, saga::back_inserter(dest), num.value, saga_test::random_engine());

        // Проверки
        REQUIRE(static_cast<std::ptrdiff_t>(dest.size())
                == std::min(num.value, saga::cursor::size(input)));

        {
            auto pos = input;

            for(auto const & item : dest)
            {
                pos = saga::find(pos, item);

                REQUIRE(!!pos);
            }
        }
    };
}

namespace
{
    template <class Value, class... Args>
    void check_unique_copy_minimal(std::vector<Value> const & src, Args... bin_pred)
    {
        static_assert(sizeof...(Args) <= 1);

        // saga
        auto src_in = saga_test::make_istringstream_from_range(src);

        std::vector<Value> dest_saga;

        saga::unique_copy(saga::make_istream_cursor<Value>(src_in)
                          , saga::back_inserter(dest_saga), bin_pred...);

        // std
        std::vector<Value> dest_std;
        std::unique_copy(src.begin(), src.end(), std::back_inserter(dest_std), bin_pred...);

        // Сравнение
        REQUIRE(dest_saga == dest_std);
    }

    template <class Value, class... Args>
    void check_unique_copy_input_to_forward(std::vector<Value> const & src
                                            , std::vector<Value> const & dest_old
                                            , Args... bin_pred)
    {
        static_assert(sizeof...(Args) <= 1);

        // Из входного
        auto src_in = saga_test::make_istringstream_from_range(src);

        auto dest_in = dest_old;
        auto const out_in = saga_test::random_subcursor_of(saga::cursor::all(dest_in));

        auto const result_in = saga::unique_copy(saga::make_istream_cursor<Value>(src_in)
                                                 , out_in, bin_pred...);

        // Из прямого
        auto dest_fwd = dest_old;
        auto const out_fwd = saga::rebase_cursor(out_in, dest_fwd);

        auto const result_fwd = saga::unique_copy(saga::cursor::all(src), out_fwd, bin_pred...);

        // Проверки
        REQUIRE(dest_in == dest_fwd);

        REQUIRE(result_fwd.out == saga::rebase_cursor(result_in.out, dest_fwd));
    }

    template <class Value, class... Args>
    void check_unique_copy_subcursor(std::vector<Value> const & src
                                     , std::vector<Value> const & dest_old
                                     , Args... bin_pred)
    {
        static_assert(sizeof...(Args) <= 1);

        // Подготовка
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto dest_saga = dest_old;
        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto const result_saga = saga::unique_copy(input, out_saga, bin_pred...);

        // std
        auto dest_std = dest_old;
        auto const out_std = dest_std.begin() + (out_saga.begin() - dest_saga.begin());

        auto const result_std = std::unique_copy(input.begin(), result_saga.in.begin(), out_std
                                                 , bin_pred...);

        // Проверки
        REQUIRE(dest_saga == dest_std);

        REQUIRE(result_saga.in.end() == input.end());

        REQUIRE((result_saga.out.begin() - out_saga.begin()) == (result_std - out_std));
        REQUIRE(result_saga.out.end() == out_saga.end());
    }
}

TEST_CASE("unique_copy: minimal")
{
    using Value = long;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto const bin_pred = [](Value const & x, Value const & y) { return x % 7 == y % 7;};

        ::check_unique_copy_minimal(src);
        ::check_unique_copy_minimal(src, std::move(bin_pred));
    };
}

TEST_CASE("unique_copy - subcursors")
{
    using Value = std::string;

    saga_test::property_checker << [](std::vector<Value> const & src
                                      , std::vector<Value> const & dest_old)
    {
        auto const bin_pred = [](Value const & x, Value const & y) {return x.size() == y.size();};

        ::check_unique_copy_subcursor(src, dest_old);
        ::check_unique_copy_subcursor(src, dest_old, bin_pred);
    };
}

TEST_CASE("unique_copy - input to forward")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src
                                      , std::vector<Value> const & dest_old)
    {
        auto const bin_pred = [](Value const & x, Value const & y) { return x % 7 == y % 7;};

        ::check_unique_copy_input_to_forward(src, dest_old);
        ::check_unique_copy_input_to_forward(src, dest_old, bin_pred);
    };
}

TEST_CASE("unique: default predicate")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src_old)
    {
        // unique
        auto src_saga = [&]()
        {
            std::forward_list<saga_test::move_only<Value>> src_saga;
            auto pos = src_saga.before_begin();

            for(auto const & value : src_old)
            {
                pos = src_saga.emplace_after(pos, value);
            }

            return src_saga;
        }();

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src_saga));

        auto const result = saga::unique(input);

        // unique_copy
        std::vector<Value> expected;
        saga::unique_copy(saga::rebase_cursor(input, src_old), saga::back_inserter(expected));

        // Проверка
        REQUIRE(std::equal(input.begin(), result.begin(), expected.begin(), expected.end()));

        REQUIRE(result == saga::cursor::drop_front_n(input, expected.size()));
    };
}

TEST_CASE("unique: custom predicate")
{
    using Value = char;

    saga_test::property_checker << [](std::vector<Value> const & src_old)
    {
        auto const bin_pred = [](char x, char y) { return x == ' ' && y == ' '; };

        // unique
        auto src_saga = [&]()
        {
            std::forward_list<saga_test::move_only<Value>> src_saga;
            auto pos = src_saga.before_begin();

            for(auto const & value : src_old)
            {
                pos = src_saga.emplace_after(pos, value);
            }

            return src_saga;
        }();

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src_saga));

        auto const result = saga::unique(input, bin_pred);

        // unique_copy
        std::vector<Value> expected;
        saga::unique_copy(saga::rebase_cursor(input, src_old)
                          , saga::back_inserter(expected), bin_pred);

        // Проверка
        REQUIRE(std::equal(input.begin(), result.begin(), expected.begin(), expected.end()));

        REQUIRE(result == saga::cursor::drop_front_n(input, expected.size()));
    };
}

TEST_CASE("unique: custom predicate - compare mod 2")
{
    using Value = int;

    saga_test::property_checker << [](std::forward_list<Value> const & values_old)
    {
        auto const pred = saga::equivalent_up_to([](Value const & arg) { return arg % 2; });

        // saga
        auto values_saga = values_old;
        auto const r_saga = saga::unique(saga::cursor::all(values_saga), pred);

        // std
        auto values_std = values_old;
        auto const r_std = std::unique(values_std.begin(), values_std.end(), pred);

        // Проверка
        REQUIRE(values_saga == values_std);

        REQUIRE(r_saga == saga::cursor::drop_front_n(saga::cursor::all(values_saga)
                                                     , std::distance(values_std.begin(), r_std)));
    };
}

TEST_CASE("is_partitioned: minimal")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto const pred = [](Value const & arg) { return arg % 3 == 0; };

        auto src_in = saga_test::make_istringstream_from_range(src);

        REQUIRE(saga::is_partitioned(saga::make_istream_cursor<Value>(src_in), pred)
                == std::is_partitioned(src.begin(), src.end(), pred));
    };
}

TEST_CASE("is_partitioned: subcursor")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto const pred = [](Value const & arg) { return arg % 3 == 0; };

        auto const cur = saga_test::random_subcursor_of(saga::cursor::all(src));

        REQUIRE(saga::is_partitioned(cur, pred)
                == std::is_partitioned(cur.begin(), cur.end(), pred));
    };
}

TEST_CASE("is_partitioned: invented true")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> src)
    {
        auto const pred = [](Value const & arg) { return arg % 3 == 0; };

        auto const cur = saga_test::random_subcursor_of(saga::cursor::all(src));

        std::partition(src.begin(), src.end(), pred);

        REQUIRE(saga::is_partitioned(cur, pred));
    };
}

TEST_CASE("partition_copy: minimal")
{
    using Value = long;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto const pred = [](auto const & x) { return x % 2 == 1; };

        // saga
        auto src_in = saga_test::make_istringstream_from_range(src);

        std::vector<Value> dest_true_saga;
        std::list<Value> dest_false_saga;

        saga::partition_copy(saga::make_istream_cursor<Value>(src_in)
                             , saga::back_inserter(dest_true_saga)
                             , saga::back_inserter(dest_false_saga), pred);

        // std
        std::vector<Value> dest_true_std;
        std::list<Value> dest_false_std;

        std::partition_copy(src.begin(), src.end()
                            , std::back_inserter(dest_true_std)
                            , std::back_inserter(dest_false_std), pred);

        // Проверка
        REQUIRE(dest_true_saga == dest_true_std);
        REQUIRE(dest_false_saga == dest_false_std);
    };
}

TEST_CASE("partition")
{
    using Value = int;

    saga_test::property_checker << [](std::list<Value> const & src_old)
    {
        auto const pred = [](Value const & arg) { return arg % 2 == 1; };

        // saga
        auto src = src_old;
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));
        auto const result = saga::partition(input, pred);

        // Проверка
        REQUIRE(saga::is_partitioned(input, pred));

        REQUIRE(result.end() == input.end());
        REQUIRE(result.dropped_front().begin() == input.begin());
        REQUIRE(result.dropped_back().end() == input.end());

        auto const input_old = saga::rebase_cursor(input, src_old);
        REQUIRE(saga::is_permutation(input, input_old));

        REQUIRE(saga::all_of(result.dropped_front(), pred));
        REQUIRE(saga::none_of(result, pred));

        REQUIRE(saga::equal(input.dropped_front(), input_old.dropped_front()));
        REQUIRE(saga::equal(input.dropped_back(), input_old.dropped_back()));
    };
}

TEST_CASE("partition_copy: subcursor")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src
                                      , std::vector<Value> const & dest_true_old
                                      , std::vector<Value> const & dest_false_old)
    {
        auto const pred = [](Value const & x) { return x % 2 == 1; };

        // Подготовка
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto dest_true_saga = dest_true_old;
        auto dest_false_saga = dest_false_old;

        auto const out_true_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_true_saga));
        auto const out_false_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_false_saga));

        auto const result_saga = saga::partition_copy(input, out_true_saga, out_false_saga, pred);

        // std
        auto dest_true_std = dest_true_old;
        auto dest_false_std = dest_false_old;

        auto const out_true_std
            = dest_true_std.begin() + (out_true_saga.begin() - dest_true_saga.begin());
        auto const out_false_std
            = dest_false_std.begin() + (out_false_saga.begin() - dest_false_saga.begin());

        auto const result_std = std::partition_copy(input.begin(), result_saga.in.begin()
                                                    , out_true_std, out_false_std, pred);

        // Проверки
        REQUIRE(dest_true_saga == dest_true_std);
        REQUIRE(dest_false_saga == dest_false_std);

        REQUIRE(result_saga.in.end() == input.end());

        REQUIRE((result_saga.out1.begin() - out_true_saga.begin())
                == (result_std.first - out_true_std));
        REQUIRE((result_saga.out2.begin() - out_false_saga.begin())
                == (result_std.second - out_false_std));

        REQUIRE(result_saga.out1.end() == out_true_saga.end());
        REQUIRE(result_saga.out2.end() == out_false_saga.end());
    };
}

TEST_CASE("stable_partition")
{
    using Value = int;

    saga_test::property_checker << [](std::list<Value> const & src_old)
    {
        auto const pred = [](Value const & arg) { return arg % 2 == 1; };

        // saga
        auto src_saga = src_old;
        auto const input_saga = saga_test::random_subcursor_of(saga::cursor::all(src_saga));

        auto const result_saga = saga::stable_partition(input_saga, pred);

        // std
        auto src_std = src_old;
        auto const input_std = saga::rebase_cursor(input_saga, src_std);

        auto const result_std = std::stable_partition(input_std.begin(), input_std.end(), pred);

        // Проверка
        REQUIRE(src_saga == src_std);

        CAPTURE(src_saga, input_saga, result_saga);

        REQUIRE(std::distance(input_std.begin(), result_std)
                 == std::distance(input_saga.begin(), result_saga.begin()));
        REQUIRE(result_saga.end() == input_saga.end());
        REQUIRE(result_saga.dropped_front().begin() == input_saga.begin());
        REQUIRE(result_saga.dropped_back().end() == input_saga.end());
    };
}

TEST_CASE("partition_point")
{
    using Value = int;

    saga_test::property_checker << [](std::forward_list<Value> const & src_old)
    {
        // Подготовка
        auto const pred = [](Value const & arg) { return arg % 2 == 0; };

        auto const src = [&]()
        {
            auto src = src_old;
            std::partition(src.begin(), src.end(), pred);
            return src;
        }();

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // Выполнение
        auto const r_std = std::partition_point(input.begin(), input.end(), pred);

        auto const r_saga = saga::partition_point(input, pred);

        // Проверка
        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(r_saga.end() == input.end());

        REQUIRE(r_saga.dropped_front().begin() == src.begin());
        REQUIRE(r_saga.dropped_back().end() == src.end());
    };
}

namespace
{
    template <class Container, class... Args>
    void check_is_sorted(Container const & src, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const result_std = std::is_sorted(input.begin(), input.end(), cmp...);
        auto const result_saga = saga::is_sorted(input, cmp...);

        REQUIRE(result_std == result_saga);
    }

    template <class Container, class... Args>
    void check_is_sorted_after_sort(Container src, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        std::sort(input.begin(), input.end(), cmp...);

        REQUIRE(saga::is_sorted(input, cmp...));
    }

    template <class Container, class... Args>
    void check_is_sorted_until(Container const & src, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const result_std = std::is_sorted_until(input.begin(), input.end(), cmp...);
        auto const result_saga = saga::is_sorted_until(input, cmp...);

        CAPTURE(src);

        REQUIRE(result_saga.begin() == result_std);
        REQUIRE(result_saga.end() == input.end());

        REQUIRE(result_saga.dropped_front().begin() == src.begin());
        REQUIRE(result_saga.dropped_front().end() == result_std);
    }
}

TEST_CASE("is_sorted")
{
    using Value = int;
    using Container = std::forward_list<Value>;

    saga_test::property_checker
    << ::check_is_sorted<Container>
    << ::check_is_sorted_after_sort<std::vector<Value>>
    << [](Container const & src)
    {
        ::check_is_sorted(src, std::greater<>{});
    }
    << [](std::vector<Value> const & src)
    {
        ::check_is_sorted_after_sort(src, std::greater<>{});
    };
}

TEST_CASE("is_sorted_until")
{
    using Value = int;
    using Container = std::forward_list<Value>;

    saga_test::property_checker
    << ::check_is_sorted_until<Container>
    << [](Container const & src)
    {
        auto const cmp = std::greater<>{};

        ::check_is_sorted_until(src, cmp);
    };
}

namespace
{
    template <class Container, class... Args>
    void check_merge_minimal(Container lhs, Container rhs, Args... cmp)
    {
        std::sort(lhs.begin(), lhs.end(), cmp...);
        std::sort(rhs.begin(), rhs.end(), cmp...);

        using Value = typename Container::value_type;

        // std
        std::vector<Value> diff_std;

        std::merge(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()
                   , std::back_inserter(diff_std), cmp...);

        // saga
        std::vector<Value> diff_saga;

        auto lhs_in = saga_test::make_istringstream_from_range(lhs);
        auto rhs_in = saga_test::make_istringstream_from_range(rhs);

        saga::merge(saga::make_istream_cursor<Value>(lhs_in)
                    , saga::make_istream_cursor<Value>(rhs_in)
                    , saga::back_inserter(diff_saga), cmp...);

        // Проверка
        REQUIRE(diff_saga == diff_std);
    }

    template <class Value, class... Args>
    void check_merge_subcursor(std::vector<Value> in1_src, std::vector<Value> in2_src
                               , std::vector<Value> const & out_src_old, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        // Исходные данные
        auto const in1 = saga_test::random_subcursor_of(saga::cursor::all(in1_src));
        auto const in2 = saga_test::random_subcursor_of(saga::cursor::all(in2_src));

        std::sort(in1.begin(), in1.end(), cmp...);
        std::sort(in2.begin(), in2.end(), cmp...);

        // saga
        std::vector<Value> out_saga(out_src_old);
        auto const out = saga_test::random_subcursor_of(saga::cursor::all(out_saga));

        auto const result = saga::merge(in1, in2, out, cmp...);

        // std
        auto out_std = out_src_old;

        std::merge(in1.begin(), result.in1.begin(), in2.begin(), result.in2.begin()
                   , saga::rebase_cursor(out, out_std).begin(), cmp...);

        // Проверка
        REQUIRE(out_saga == out_std);
    }
}

TEST_CASE("merge")
{
    using Value = int;
    using Container = std::vector<Value>;

    saga_test::property_checker
    << ::check_merge_minimal<Container>
    << ::check_merge_subcursor<Container>
    << [](Container lhs, Container rhs)
    {
        ::check_merge_minimal(lhs, rhs, std::greater<>{});
    }
    << [](Container lhs, Container rhs, std::vector<Value> const & out_src_old)
    {
        ::check_merge_subcursor(std::move(lhs), std::move(rhs), out_src_old, std::greater<>{});
    };
}

namespace
{
    template <class Value, class... Args>
    void check_inplace_merge(std::vector<Value> const & values_old, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        // Подготовка
        auto src = values_old;

        auto const input_src = saga_test::random_subcursor_of(saga::cursor::all(src));

        std::sort(input_src.dropped_front().begin(), input_src.begin(), cmp...);
        std::sort(input_src.begin(), input_src.end(), cmp...);

        std::list<Value> values(src.begin(), src.end());

        auto const input = saga::rebase_cursor(input_src, values);

        // Выполняем
        saga::inplace_merge(input, cmp...);

        // Проверка
        REQUIRE(saga::is_permutation(saga::cursor::all(values)
                                     , saga::cursor::all(values_old)));

        REQUIRE(saga::equal(input.dropped_back()
                            , saga::rebase_cursor(input, values_old).dropped_back()));

        REQUIRE(std::is_sorted(input.dropped_front().begin(), input.end(), cmp...));
    }
}

TEST_CASE("inplace_merge")
{
    using Value = int;

    saga_test::property_checker
    << check_inplace_merge<Value>
    << [](std::vector<Value> const & values_old)
    {
        auto const cmp = saga::compare_by([](Value const & arg) { return arg % 2017; });

        check_inplace_merge(values_old, cmp);
    };
}

namespace
{
    template <class Value, class... Args>
    void check_includes_minimal(std::vector<Value> in1, std::vector<Value> in2, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        std::sort(in1.begin(), in1.end(), cmp...);
        std::sort(in2.begin(), in2.end(), cmp...);

        auto in1_istream = saga_test::make_istringstream_from_range(in1);
        auto in2_istream = saga_test::make_istringstream_from_range(in2);

        REQUIRE(saga::includes(saga::make_istream_cursor<Value>(in1_istream),
                               saga::make_istream_cursor<Value>(in2_istream), cmp...)
                == std::includes(in1.begin(), in1.end(), in2.begin(), in2.end(), cmp...));
    }

    template <class Value, class... Args>
    void check_includes_subrange(std::vector<Value> in1_src, std::vector<Value> in2_src
                                 , Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        auto const in1 = saga_test::random_subcursor_of(saga::cursor::all(in1_src));
        auto const in2 = saga_test::random_subcursor_of(saga::cursor::all(in2_src));

        std::sort(in1.begin(), in1.end(), cmp...);
        std::sort(in2.begin(), in2.end(), cmp...);

        REQUIRE(saga::includes(in1, in2, cmp...)
                == std::includes(in1.begin(), in1.end(), in2.begin(), in2.end(), cmp...));
    }
}

TEST_CASE("includes - minimal")
{
    using Value = int;

    saga_test::property_checker
    << ::check_includes_minimal<Value>
    << ::check_includes_subrange<Value>
    << [](std::vector<Value> in1, std::vector<Value> in2)
    {
        ::check_includes_minimal(std::move(in1), std::move(in2), std::greater<>{});
    }
    << [](std::vector<Value> in1, std::vector<Value> in2)
    {
        ::check_includes_subrange(std::move(in1), std::move(in2), std::greater<>{});
    };
}

TEST_CASE("includes - always true, custom compare")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> in1, std::vector<Value> in2)
    {
        auto const cmp = std::greater<>{};

        in1.insert(in1.end(), in2.begin(), in2.end());

        std::sort(in1.begin(), in1.end(), cmp);
        std::sort(in2.begin(), in2.end(), cmp);

        CAPTURE(in1, in2);

        REQUIRE(std::includes(in1.begin(), in1.end(), in2.begin(), in2.end(), cmp));
        REQUIRE(saga::includes(saga::cursor::all(in1), saga::cursor::all(in2), cmp));
    };
}

namespace
{
    template <class Container, class... Args>
    void check_set_difference_minimal(Container lhs, Container rhs, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        std::sort(lhs.begin(), lhs.end(), cmp...);
        std::sort(rhs.begin(), rhs.end(), cmp...);

        using Value = typename Container::value_type;

        // std
        std::vector<Value> diff_std;

        std::set_difference(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                            std::back_inserter(diff_std), cmp...);

        // saga
        std::vector<Value> diff_saga;

        auto lhs_in = saga_test::make_istringstream_from_range(lhs);
        auto rhs_in = saga_test::make_istringstream_from_range(rhs);

        saga::set_difference(saga::make_istream_cursor<Value>(lhs_in),
                             saga::make_istream_cursor<Value>(rhs_in),
                             saga::back_inserter(diff_saga), cmp...);

        // Проверка
        REQUIRE(diff_saga == diff_std);
    }

    template <class Value, class... Args>
    void check_set_difference_subcursor(std::vector<Value> in1_src
                                        , std::vector<Value> in2_src
                                        , std::vector<Value> const & out_src_old
                                        , Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        std::sort(in1_src.begin(), in1_src.end(), cmp...);
        std::sort(in2_src.begin(), in2_src.end(), cmp...);

        // Исходные данные
        auto const in1 = saga_test::random_subcursor_of(saga::cursor::all(in1_src));
        auto const in2 = saga_test::random_subcursor_of(saga::cursor::all(in2_src));

        // saga
        std::vector<Value> out_saga(out_src_old);
        auto const out = saga_test::random_subcursor_of(saga::cursor::all(out_saga));

        auto const result = saga::set_difference(in1, in2, out, cmp...);

        // std
        auto out_std = out_src_old;

        std::set_difference(in1.begin(), result.in.begin(), in2.begin(), in2.end()
                            , saga::rebase_cursor(out, out_std).begin(), cmp...);

        // Проверка
        REQUIRE(out_saga == out_std);
    }
}

TEST_CASE("set_difference")
{
    using Value = int;
    using Container = std::vector<Value>;

    saga_test::property_checker
    << ::check_set_difference_minimal<Container>
    << ::check_set_difference_subcursor<Container>
    << [](Container lhs, Container rhs)
    {
        ::check_set_difference_minimal(std::move(lhs), std::move(rhs), std::greater<>{});
    }
    << [](Container lhs, Container rhs, Container const & out_src_old)
    {
        ::check_set_difference_subcursor(std::move(lhs), std::move(rhs)
                                         , out_src_old, std::greater<>{});
    };
}

namespace
{
    template <class Container, class... Args>
    void test_set_difference_minimal(Container lhs, Container rhs, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        std::sort(lhs.begin(), lhs.end(), cmp...);
        std::sort(rhs.begin(), rhs.end(), cmp...);

        using Value = typename Container::value_type;

        // std
        std::vector<Value> diff_std;

        std::set_intersection(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()
                             , std::back_inserter(diff_std), cmp...);

        // saga
        std::vector<Value> diff_saga;

        auto lhs_in = saga_test::make_istringstream_from_range(lhs);
        auto rhs_in = saga_test::make_istringstream_from_range(rhs);

        saga::set_intersection(saga::make_istream_cursor<Value>(lhs_in)
                              , saga::make_istream_cursor<Value>(rhs_in)
                              , saga::back_inserter(diff_saga), cmp...);

        // Проверка
        REQUIRE(diff_saga == diff_std);
    }

    template <class Container, class... Args>
    void test_set_difference_subcursor(Container in1_src, Container in2_src
                                      , Container const & out_src_old, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        std::sort(in1_src.begin(), in1_src.end(), cmp...);
        std::sort(in2_src.begin(), in2_src.end(), cmp...);

        // Исходные данные
        auto const in1 = saga_test::random_subcursor_of(saga::cursor::all(in1_src));
        auto const in2 = saga_test::random_subcursor_of(saga::cursor::all(in2_src));

        // saga
        auto out_saga = out_src_old;
        auto const out = saga_test::random_subcursor_of(saga::cursor::all(out_saga));

        auto const result = saga::set_intersection(in1, in2, out, cmp...);

        // std
        auto out_std = out_src_old;
        std::set_intersection(in1.begin(), result.in1.begin(), in2.begin(), result.in2.begin()
                              , saga::rebase_cursor(out, out_std).begin(), cmp...);

        // Проверка
        REQUIRE(out_saga == out_std);
    }
}

TEST_CASE("set_intersection")
{
    using Value = int;
    using Container = std::vector<Value>;

    saga_test::property_checker
    << ::test_set_difference_minimal<Container>
    << ::test_set_difference_subcursor<Container>
    << [](Container lhs, Container rhs)
    {
        ::test_set_difference_minimal(std::move(lhs), std::move(rhs), std::greater<>{});
    }
    << [](Container lhs, Container rhs, Container const & out_src_old)
    {
        ::test_set_difference_subcursor(std::move(lhs), std::move(rhs)
                                        , out_src_old, std::greater<>{});
    };
}

namespace
{
    template <class Container, class... Args>
    void test_set_symmetric_difference_minimal(Container lhs, Container rhs, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        std::sort(lhs.begin(), lhs.end(), cmp...);
        std::sort(rhs.begin(), rhs.end(), cmp...);

        using Value = typename Container::value_type;

        // std
        std::vector<Value> diff_std;

        std::set_symmetric_difference(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()
                                      , std::back_inserter(diff_std), cmp...);

        // saga
        std::vector<Value> diff_saga;

        auto lhs_in = saga_test::make_istringstream_from_range(lhs);
        auto rhs_in = saga_test::make_istringstream_from_range(rhs);

        saga::set_symmetric_difference(saga::make_istream_cursor<Value>(lhs_in)
                                       , saga::make_istream_cursor<Value>(rhs_in)
                                       , saga::back_inserter(diff_saga), cmp...);

        // Проверка
        REQUIRE(diff_saga == diff_std);
    }

    template <class Container, class... Args>
    void test_set_symmetric_difference_subcursor(Container in1_src, Container in2_src
                                                 , Container const & out_src_old, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        std::sort(in1_src.begin(), in1_src.end(), cmp...);
        std::sort(in2_src.begin(), in2_src.end(), cmp...);

        // Исходные данные
        auto const in1 = saga_test::random_subcursor_of(saga::cursor::all(in1_src));
        auto const in2 = saga_test::random_subcursor_of(saga::cursor::all(in2_src));

        // saga
        auto out_saga = out_src_old;
        auto const out = saga_test::random_subcursor_of(saga::cursor::all(out_saga));

        auto const result = saga::set_symmetric_difference(in1, in2, out, cmp...);

        // std
        auto out_std = out_src_old;
        std::set_symmetric_difference(in1.begin(), result.in1.begin()
                                      , in2.begin(), result.in2.begin()
                                      , saga::rebase_cursor(out, out_std).begin(), cmp...);

        // Проверка
        REQUIRE(out_saga == out_std);
    }
}

TEST_CASE("set_symmetric_difference")
{
    using Value = int;
    using Container = std::vector<Value>;

    saga_test::property_checker
    <<  ::test_set_symmetric_difference_minimal<Container>
    << ::test_set_symmetric_difference_subcursor<Container>
    << [](Container lhs, Container rhs)
    {
        ::test_set_symmetric_difference_minimal(std::move(lhs), std::move(rhs), std::greater<>{});
    }
    << [](Container lhs, Container rhs, Container const & out_src_old)
    {
        ::test_set_symmetric_difference_subcursor(std::move(lhs), std::move(rhs)
                                                  , out_src_old, std::greater<>{});
    };
}

namespace
{
    template <class Container, class... Args>
    void test_set_union_minimal(Container lhs, Container rhs, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        std::sort(lhs.begin(), lhs.end(), cmp...);
        std::sort(rhs.begin(), rhs.end(), cmp...);

        using Value = typename Container::value_type;

        // std
        std::vector<Value> diff_std;

        std::set_union(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()
                       , std::back_inserter(diff_std), cmp...);

        // saga
        std::vector<Value> diff_saga;

        auto lhs_in = saga_test::make_istringstream_from_range(lhs);
        auto rhs_in = saga_test::make_istringstream_from_range(rhs);

        saga::set_union(saga::make_istream_cursor<Value>(lhs_in)
                                       , saga::make_istream_cursor<Value>(rhs_in)
                                       , saga::back_inserter(diff_saga), cmp...);

        // Проверка
        REQUIRE(diff_saga == diff_std);
    }

    template <class Container, class... Args>
    void test_set_union_subcursor(Container in1_src, Container in2_src
                                  , Container const & out_src_old, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        std::sort(in1_src.begin(), in1_src.end(), cmp...);
        std::sort(in2_src.begin(), in2_src.end(), cmp...);

        // Исходные данные
        auto const in1 = saga_test::random_subcursor_of(saga::cursor::all(in1_src));
        auto const in2 = saga_test::random_subcursor_of(saga::cursor::all(in2_src));

        // saga
        auto out_saga = out_src_old;
        auto const out = saga_test::random_subcursor_of(saga::cursor::all(out_saga));

        auto const result = saga::set_union(in1, in2, out, cmp...);

        // std
        auto out_std = out_src_old;
        std::set_union(in1.begin(), result.in1.begin(), in2.begin(), result.in2.begin()
                       , saga::rebase_cursor(out, out_std).begin(), cmp...);

        // Проверка
        REQUIRE(out_saga == out_std);
    }
}

TEST_CASE("set_union")
{
    using Value = int;
    using Container = std::vector<Value>;

    saga_test::property_checker
    <<  ::test_set_union_minimal<Container>
    << ::test_set_union_minimal<Container>
    << [](Container lhs, Container rhs)
    {
        ::test_set_union_minimal(std::move(lhs), std::move(rhs), std::greater<>{});
    }
    << [](Container lhs, Container rhs, Container const & out_src_old)
    {
        ::test_set_union_subcursor(std::move(lhs), std::move(rhs), out_src_old, std::greater<>{});
    };
}

namespace
{
    template <class Container, class... Args>
    void test_is_heap_until(Container const & src, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const result_std = std::is_heap_until(input.begin(), input.end(), cmp...);
        auto const result_saga = saga::is_heap_until(input, cmp...);

        REQUIRE(result_saga.begin() == result_std);
        REQUIRE(result_saga.end() == input.end());

        REQUIRE(result_saga.dropped_front().begin() == src.begin());
        REQUIRE(result_saga.dropped_front().end() == result_saga.begin());
    }

    template <class Container, class... Args>
    void test_is_heap_until_for_heap(Container src, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        std::make_heap(src.begin(), src.end(), cmp...);

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const result_std = std::is_heap_until(input.begin(), input.end(), cmp...);
        auto const result_saga = saga::is_heap_until(input, cmp...);

        REQUIRE(result_saga.begin() == result_std);
        REQUIRE(result_saga.end() == input.end());

        REQUIRE(result_saga.dropped_front().begin() == src.begin());
        REQUIRE(result_saga.dropped_front().end() == result_saga.begin());
    }
}

TEST_CASE("is_heap_until")
{
    using Value = int;

    saga_test::property_checker
    << ::test_is_heap_until<std::vector<Value>>
    << ::test_is_heap_until_for_heap<std::vector<Value>>
    << [](std::vector<Value> const & src)
    {
        ::test_is_heap_until(src, std::greater<>{});
    }
    << [](std::vector<Value> src)
    {
        ::test_is_heap_until_for_heap(std::move(src), std::greater<>{});
    };
}

namespace
{
    template <class Container, class... Args>
    void test_is_heap(Container const & src, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const result_std = std::is_heap(input.begin(), input.end(), cmp...);
        auto const result_saga = saga::is_heap(input, cmp...);

        REQUIRE(result_saga == result_std);
    }

    template <class Container, class... Args>
    void test_is_heap_for_heap(Container src, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        std::make_heap(src.begin(), src.end(), cmp...);

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const result_std = std::is_heap(input.begin(), input.end(), cmp...);
        auto const result_saga = saga::is_heap(input, cmp...);

        REQUIRE(result_saga == result_std);
    }
}

TEST_CASE("is_heap")
{
    using Value = int;

    saga_test::property_checker
    << ::test_is_heap<std::vector<Value>>
    << ::test_is_heap_for_heap<std::vector<Value>>
    << [](std::vector<Value> const & src)
    {
        ::test_is_heap(src, std::greater<>{});
    }
    << [](std::vector<Value> src)
    {
        ::test_is_heap_for_heap(std::move(src), std::greater<>{});
    };
}

namespace
{
    template <class Value, class... Args>
    void test_push_heap(std::vector<Value> src, Value const & value, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        src.push_back(value);

        auto const input = saga::cursor::all(src);

        for(auto cur = saga::cursor::all(input); !!cur;)
        {
            auto const src_old = src;

            REQUIRE(saga::is_heap(cur.dropped_front(), cmp...));

            ++ cur;

            saga::push_heap(cur.dropped_front(), cmp...);

            auto const input_src_old = saga::rebase_cursor(input, src_old);
            auto const cur_src_old = saga::rebase_cursor(cur, src_old);

            CHECK(saga::equal(input.dropped_front(), input_src_old.dropped_front()));
            CHECK(saga::equal(cur, cur_src_old));
            CHECK(saga::equal(cur.dropped_back(), cur_src_old.dropped_back()));

            REQUIRE(saga::is_heap(cur.dropped_front(), cmp...));

            CHECK(saga::is_permutation(cur.dropped_front(), cur_src_old.dropped_front()));
        }
    }
}

TEST_CASE("push_heap")
{
    using Value = long;

    saga_test::property_checker
    << ::test_push_heap<Value>
    << [](std::vector<Value> src, Value const & value)
    {
        ::test_push_heap(std::move(src), value, std::greater<>{});
    };
}

namespace
{
    template <class Value, class... Args>
    void test_make_heap(std::vector<Value> const & src_old, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        auto src = src_old;
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        saga::make_heap(input, cmp...);

        CAPTURE(src_old, src);

        auto const input_src_old = saga::rebase_cursor(input, src_old);

        CHECK(saga::equal(input.dropped_front(), input_src_old.dropped_front()));
        CHECK(saga::is_permutation(input, input_src_old));
        CHECK(saga::equal(input.dropped_back(), input_src_old.dropped_back()));

        CHECK(saga::is_heap(input, cmp...));
    }
}

TEST_CASE("make_heap")
{
    using Value = long;

    saga_test::property_checker
    << ::test_make_heap<Value>
    << [](std::vector<Value> const & src_old)
    {
        ::test_make_heap(src_old, std::greater<>{});
    };
}

namespace
{
    template <class Value, class... Args>
    void test_pop_heap(std::vector<Value> src, Value value, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        src.push_back(std::move(value));

        auto input = saga_test::random_subcursor_of(saga::cursor::all(src));

        saga::make_heap(input, cmp...);
        REQUIRE(saga::is_heap(input, cmp...));

        for(;!!input;)
        {
            auto const src_old = src;

            saga::pop_heap(input, cmp...);

            CAPTURE(src_old, src);

            auto const input_src_old = saga::rebase_cursor(input, src_old);

            CHECK(saga::equal(input.dropped_front(), input_src_old.dropped_front()));
            CHECK(saga::is_permutation(input, input_src_old));
            CHECK(saga::equal(input.dropped_back(), input_src_old.dropped_back()));

            REQUIRE(input.back() == input_src_old.front());

            input.drop_back();

            REQUIRE(saga::is_heap(input, cmp...));
        }
    }
}

TEST_CASE("pop_heap")
{
    using Value = int;

    saga_test::property_checker
    << ::test_pop_heap<Value>
    << [](std::vector<Value> src, Value value)
    {
        ::test_pop_heap(std::move(src), std::move(value), std::greater<>{});
    };
}

namespace
{
    template <class Value, class... Args>
    void test_sort_heap(std::vector<Value> src_old, Args... cmp)
    {
        // Подготовка
        auto src = src_old;
        auto input = saga_test::random_subcursor_of(saga::cursor::all(src));

        saga::make_heap(input, cmp...);

        // Алгоритм
        saga::sort_heap(input, cmp...);

        // Проверки
        REQUIRE(saga::is_sorted(input, cmp...));

        auto const cur_src_old = saga::rebase_cursor(input, src_old);

        REQUIRE(std::is_permutation(input.begin(), input.end(), cur_src_old.begin()));

        REQUIRE(saga::equal(input.dropped_front(), cur_src_old.dropped_front()));
        REQUIRE(saga::equal(input.dropped_back(), cur_src_old.dropped_back()));
    }
}

TEST_CASE("sort_heap")
{
    using Value = int;

    saga_test::property_checker
    << ::test_sort_heap<Value>
    << [](std::vector<Value> const & src_old)
    {
        ::test_sort_heap(src_old, std::greater<>{});
    };
}

namespace
{
    template <class Value, class... Args>
    void test_insertion_sort(std::list<Value> const & values_old, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        // Подготовка
        auto values = values_old;

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(values));

        // Выполнение
        saga::insertion_sort(input, cmp...);

        // Проверка
        REQUIRE(saga::is_sorted(input, cmp...));

        auto const input_old = saga::rebase_cursor(input, values_old);

        REQUIRE(saga::is_permutation(input, input_old));
        REQUIRE(saga::equal(input.dropped_front(), input_old.dropped_front()));
        REQUIRE(saga::equal(input.dropped_back(), input_old.dropped_back()));
    }

    template <class Value, class... Args>
    void test_sort(std::vector<Value> const & values_old, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        // Подготовка
        auto values = values_old;

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(values));

        // Выполнение
        saga::sort(input, cmp...);

        // Проверка
        REQUIRE(saga::is_sorted(input, cmp...));

        auto const input_old = saga::rebase_cursor(input, values_old);

        REQUIRE(saga::is_permutation(input, input_old));
        REQUIRE(saga::equal(input.dropped_front(), input_old.dropped_front()));
        REQUIRE(saga::equal(input.dropped_back(), input_old.dropped_back()));
    }
}

TEST_CASE("insertion_sort")
{
    using Value = long;

    saga_test::property_checker
    << ::test_insertion_sort<Value>
    << [](std::list<Value> const & values_old)
    {
        ::test_insertion_sort(values_old, std::greater<>{});
    };
}

TEST_CASE("sort")
{
    using Value = long;

    saga_test::property_checker
    << ::test_sort<Value>
    << [](std::vector<Value> const & values_old)
    {
        auto const cmp = saga::compare_by([](Value const & arg) { return arg % 17; }
                                         , std::greater<>{});
        ::test_sort(values_old, cmp);
    };
}

TEST_CASE("partial_sort - default compare")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src_old)
    {
        auto src = src_old;

        auto const subrange = saga_test::random_subcursor_of(saga::cursor::all(src));
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(subrange));

        saga::partial_sort(input);

        CAPTURE(src_old, src);

        REQUIRE(saga::is_sorted(input.dropped_front()));

        auto const subrange_src_old = saga::rebase_cursor(subrange, src_old);
        auto const input_src_old = saga::rebase_cursor(input, subrange_src_old);

        REQUIRE(saga::equal(subrange.dropped_front(), subrange_src_old.dropped_front()));
        REQUIRE(std::is_permutation(subrange.begin(), input.end(),
                                    subrange_src_old.begin(), input_src_old.end()));
        CHECK(saga::equal(input.dropped_back(), input_src_old.dropped_back()));
        CHECK(saga::equal(subrange.dropped_back(), subrange_src_old.dropped_back()));

        if(!!input)
        {
            REQUIRE(saga::none_of(input.dropped_front(), [&](auto const & x){return *input < x;}));
        }
    };
}

TEST_CASE("partial_sort - custom compare")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src_old)
    {
        auto const cmp = std::greater<>{};

        auto src = src_old;

        auto const subrange = saga_test::random_subcursor_of(saga::cursor::all(src));
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(subrange));

        saga::partial_sort(input, cmp);

        CAPTURE(src_old, src);

        REQUIRE(saga::is_sorted(input.dropped_front(), cmp));

        auto const subrange_src_old = saga::rebase_cursor(subrange, src_old);
        auto const input_src_old = saga::rebase_cursor(input, subrange_src_old);

        REQUIRE(saga::equal(subrange.dropped_front(), subrange_src_old.dropped_front()));
        REQUIRE(std::is_permutation(subrange.begin(), input.end(),
                                    subrange_src_old.begin(), input_src_old.end()));

        CHECK(saga::equal(input.dropped_back(), input_src_old.dropped_back()));
        CHECK(saga::equal(subrange.dropped_back(), subrange_src_old.dropped_back()));

        if(!!input)
        {
            CHECK(saga::none_of(input.dropped_front(),[&](auto const & x){return cmp(*input, x);}));
        }
    };
}

namespace
{
    template <class Value, class... Args>
    void test_partial_sort_copy_minimal(std::vector<Value> const & src
                                        , std::vector<Value> const & dest_old, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        // saga
        auto dest_saga = dest_old;

        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto src_saga = saga_test::make_istringstream_from_range(src);

        auto const r_saga = saga::partial_sort_copy(saga::make_istream_cursor<Value>(src_saga)
                                                    , out_saga, cmp...);

        // std
        auto dest_std = dest_old;

        auto const out_std = saga::rebase_cursor(out_saga, dest_std);

        auto const r_std = std::partial_sort_copy(src.begin(), src.end()
                                                  , out_std.begin(), out_std.end(), cmp...);

        // Проверка
        REQUIRE(dest_saga == dest_std);

        if(!!out_saga)
        {
            REQUIRE(!r_saga.in);
        }

        REQUIRE(r_saga.out.size() == (out_std.end() - r_std));
        REQUIRE(r_saga.out.end() == out_saga.end());
        REQUIRE(r_saga.out.dropped_front().begin() == dest_saga.begin());
        REQUIRE(r_saga.out.dropped_back().end() == dest_saga.end());
    }

    template <class Value, class... Args>
    void test_partial_sort_copy_input_subcursor(std::vector<Value> const & src
                                                , std::vector<Value> const & dest_old
                                                , Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto dest_saga = dest_old;

        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto const r_saga = saga::partial_sort_copy(input, out_saga, cmp...);

        // std
        auto dest_std = dest_old;

        auto const out_std = saga::rebase_cursor(out_saga, dest_std);

        auto const r_std = std::partial_sort_copy(input.begin(), input.end()
                                                  , out_std.begin(), out_std.end(), cmp...);

        // Проверка
        REQUIRE(dest_saga == dest_std);

        REQUIRE(r_saga.in.begin() == (!!out_saga ? input.end() : input.begin()));

        REQUIRE(r_saga.in.end() == input.end());
        REQUIRE(r_saga.in.dropped_front().begin() == src.begin());
        REQUIRE(r_saga.in.dropped_back().end() == src.end());

        REQUIRE(r_saga.out.size() == (out_std.end() - r_std));
        REQUIRE(r_saga.out.end() == out_saga.end());
        REQUIRE(r_saga.out.dropped_front().begin() == dest_saga.begin());
        REQUIRE(r_saga.out.dropped_back().end() == dest_saga.end());
    }
}

TEST_CASE("partial_sort_copy")
{
    using Value = int;

    saga_test::property_checker
    << ::test_partial_sort_copy_minimal<Value>
    << ::test_partial_sort_copy_input_subcursor<Value>
    << [](std::vector<Value> const & src, std::vector<Value> const & dest_old)
    {
        ::test_partial_sort_copy_minimal(src, dest_old, std::greater<>{});
    }
    << [](std::vector<Value> const & src, std::vector<Value> const & dest_old)
    {
        ::test_partial_sort_copy_input_subcursor(src, dest_old, std::greater<>{});
    };
}

namespace
{
    template <class Value, class... Args>
    void test_stable_sort(std::vector<Value> const & values_old, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        // saga
        auto values = values_old;

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(values));

        saga::stable_sort(input, cmp...);

        // std
        auto values_std = values_old;
        auto const input_std = saga::rebase_cursor(input, values_std);

        std::stable_sort(input_std.begin(), input_std.end(), cmp...);

        // Проверки
        REQUIRE(values == values_std);
    }
}

TEST_CASE("stable_sort")
{
    using Value = int;

    saga_test::property_checker
    << ::test_stable_sort<Value>
    << [](std::vector<Value> const & values_old)
    {
        ::test_stable_sort(values_old
                           , saga::compare_by([](Value const & arg) { return arg % 2017; }));
    };
}

TEST_CASE("nth_element: default compare")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & values_old)
    {
        // Подготовка
        auto values = values_old;
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(values));

        // Выполнение
        saga::nth_element(input);

        // Проверка
        auto const input_old = saga::rebase_cursor(input, values_old);
        CAPTURE(values_old, values, input_old, input);

        if(!input)
        {
            REQUIRE(values == values_old);
        }
        else
        {
            REQUIRE(saga::equal(input.dropped_back(), input_old.dropped_back()));
            REQUIRE(saga::is_permutation(saga::cursor::all(values)
                                         , saga::cursor::all(values_old)));

            auto const border = *input;

            REQUIRE(saga::none_of(input
                                  , [&](Value const & arg) { return arg < border; }));
            REQUIRE(saga::none_of(input.dropped_front()
                                  , [&](Value const & arg) { return border < arg; }));
        }
    };
}

TEST_CASE("nth_element: custom compare")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & values_old)
    {
        auto const cmp = saga::compare_by([](Value const & arg) { return arg % 101; });

        // Подготовка
        auto values = values_old;
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(values));

        // Выполнение
        saga::nth_element(input, cmp);

        // Проверка
        auto const input_old = saga::rebase_cursor(input, values_old);
        CAPTURE(values_old, values, input, input_old);

        if(!input)
        {
            REQUIRE(values == values_old);
        }
        else
        {
            REQUIRE(saga::equal(input.dropped_back(), input_old.dropped_back()));
            REQUIRE(saga::is_permutation(saga::cursor::all(values)
                                         , saga::cursor::all(values_old)));

            auto const border = *input;

            REQUIRE(saga::none_of(input
                                  , [&](Value const & arg) { return cmp(arg, border); }));
            REQUIRE(saga::none_of(input.dropped_front()
                                  , [&](Value const & arg) { return cmp(border, arg); }));
        }
    };
}

namespace
{
    template <class Value, class... Args>
    void test_lower_bound(std::forward_list<Value> const & src_old
                          , Value const & value, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        // Подготовка
        auto const src = [&]()
        {
            auto src = src_old;
            src.sort(cmp...);
            return src;
        }();

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // Выполнение
        auto const r_std = std::lower_bound(input.begin(), input.end(), value, cmp...);

        auto const r_saga = saga::lower_bound(input, value, cmp...);

        // Проверка
        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(r_saga.end() == input.end());

        REQUIRE(r_saga.dropped_front().begin() == src.begin());
        REQUIRE(r_saga.dropped_back().end() == src.end());
    }
}

TEST_CASE("lower_bound")
{
    using Value = int;

    saga_test::property_checker
    << ::test_lower_bound<Value>
    << [](std::forward_list<Value> const & src_old, Value const & value)
    {
        ::test_lower_bound(src_old, value, std::greater<>{});
    };
}

namespace
{
    template <class Value, class... Args>
    void test_upper_bound(std::forward_list<Value> const & src_old
                          , Value const & value, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        // Подготовка
        auto const src = [&]()
        {
            auto src = src_old;
            src.sort(cmp...);
            return src;
        }();

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // Выполнение
        auto const r_std = std::upper_bound(input.begin(), input.end(), value, cmp...);

        auto const r_saga = saga::upper_bound(input, value, cmp...);

        // Проверка
        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(r_saga.end() == input.end());

        REQUIRE(r_saga.dropped_front().begin() == src.begin());
        REQUIRE(r_saga.dropped_back().end() == src.end());
    }
}

TEST_CASE("upper_bound")
{
    using Value = int;

    saga_test::property_checker
    << ::test_upper_bound<Value>
    << [](std::forward_list<Value> const & src_old, Value const & value)
    {
        ::test_upper_bound(src_old, value, std::greater<>{});
    };
}

namespace
{
    template <class Value, class... Args>
    void test_equal_range(std::forward_list<Value> const & src_old
                          , Value const & value, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        // Подготовка
        auto const src = [&]()
        {
            auto src = src_old;
            src.sort(cmp...);
            return src;
        }();

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // std
        auto const r_std = std::equal_range(input.begin(), input.end(), value, cmp...);

        // saga
        auto const r_saga = saga::equal_range(input, value, cmp...);

        // Проверки
        REQUIRE(r_saga.begin() == r_std.first);
        REQUIRE(r_saga.end() == r_std.second);

        REQUIRE(r_saga.dropped_front().begin() == input.begin());
        REQUIRE(r_saga.dropped_back().end() == input.end());
    }
}

// equal_range
TEST_CASE("equal_range")
{
    using Value = long;

    saga_test::property_checker
    << ::test_equal_range<Value>
    << [](std::forward_list<Value> const & src_old, Value const & value)
    {
        ::test_equal_range(src_old, value, std::greater<>{});
    };
}

namespace
{
    template <class Value, class... Args>
    void test_binary_search(std::forward_list<Value> const & src_old
                            , Value const & value, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        // Подготовка
        auto const src = [&]()
        {
            auto src = src_old;
            src.sort(cmp...);
            return src;
        }();

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // Выполнение
        auto const r_std = std::binary_search(input.begin(), input.end(), value, cmp...);

        auto const r_saga = saga::binary_search(input, value, cmp...);

        // Проверка
        REQUIRE(r_saga == r_std);
    }
}

TEST_CASE("binary_search")
{
    using Value = int;

    saga_test::property_checker
    << ::test_binary_search<Value>
    << [](std::forward_list<Value> const & src_old, Value const & value)
    {
        ::test_binary_search(src_old, value, std::greater<>{});
    };
}

namespace
{
    template <class Value, class... Args>
    void test_min_element(std::forward_list<Value> const & src, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const r_std = std::min_element(input.begin(), input.end(), cmp...);
        auto const r_saga = saga::min_element(input, cmp...);

        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(r_saga.end() == input.end());

        REQUIRE(r_saga.dropped_front().begin() == src.begin());
        REQUIRE(r_saga.dropped_front().end() == r_std);

        REQUIRE(r_saga.dropped_back().begin() == input.end());
        REQUIRE(r_saga.dropped_back().end() == src.end());
    }
}

TEST_CASE("min_element")
{
    using Value = int;

    saga_test::property_checker
    << ::test_min_element<Value>
    << [](std::forward_list<Value> const & src)
    {
        ::test_min_element(src, std::greater<>{});
    };
}

namespace
{
    template <class Value, class... Args>
    void test_max_element(std::forward_list<Value> const & src, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const r_saga = saga::max_element(input, cmp...);
        auto const r_std = std::minmax_element(input.begin(), input.end(), cmp...);

        // Проверка
        REQUIRE(r_saga.begin() == r_std.second);
        REQUIRE(r_saga.end() == input.end());
        REQUIRE(r_saga.dropped_front().begin() == src.begin());
        REQUIRE(r_saga.dropped_back().end() == src.end());
    }
}

TEST_CASE("max_element")
{
    using Value = int;

    saga_test::property_checker
    << ::test_max_element<Value>
    << [](std::forward_list<Value> const & src)
    {
        auto const cmp = saga::compare_by([](Value const & arg) { return arg % 5; });

        ::test_max_element(src, cmp);
    };
}

namespace
{
    template <class Value, class... Args>
    void test_minmax_element(std::forward_list<Value> const & src, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const r_saga = saga::minmax_element(input, cmp...);
        auto const r_std = std::minmax_element(input.begin(), input.end(), cmp...);

        // Проверка
        REQUIRE(r_saga.min.begin() == r_std.first);
        REQUIRE(r_saga.min.end() == input.end());
        REQUIRE(r_saga.min.dropped_front().begin() == src.begin());
        REQUIRE(r_saga.min.dropped_back().end() == src.end());

        REQUIRE(r_saga.max.begin() == r_std.second);
        REQUIRE(r_saga.max.end() == input.end());
        REQUIRE(r_saga.max.dropped_front().begin() == src.begin());
        REQUIRE(r_saga.max.dropped_back().end() == src.end());
    }
}

TEST_CASE("minmax_element")
{
    using Value = int;

    saga_test::property_checker
    << ::test_minmax_element<Value>
    << [](std::forward_list<Value> const & src)
    {
        auto const cmp = saga::compare_by([](Value const & arg) { return arg % 5; });

        ::test_minmax_element(src, cmp);
    };
}

TEST_CASE("clamp: default compare")
{
    using Value = int;

    saga_test::property_checker
    <<[](Value const & value, Value const & border1, Value const & border2)
    {
        auto const borders = std::minmax(border1, border2);

        Value const & result = saga::clamp(value, borders.first, borders.second);

        REQUIRE((borders.first <= result && result <= borders.second));

        if(value < borders.first)
        {
            REQUIRE(std::addressof(result) == std::addressof(borders.first));
        }
        else if(borders.second < value)
        {
            REQUIRE(std::addressof(result) == std::addressof(borders.second));
        }
        else
        {
            REQUIRE(std::addressof(result) == std::addressof(value));
        }
    };

    static_assert(saga::clamp(1, 3, 5) == 3, "");
    static_assert(saga::clamp(4, 3, 5) == 4, "");
    static_assert(saga::clamp(7, 3, 5) == 5, "");
}

TEST_CASE("clamp: custom compare")
{
    using Value = int;

    saga_test::property_checker
    <<[](Value const & value, Value const & border1, Value const & border2)
    {
        auto const cmp = std::greater<>{};

        auto const borders = std::minmax(border1, border2, cmp);

        Value const & result = saga::clamp(value, borders.first, borders.second, cmp);

        REQUIRE((!cmp(result, borders.first) && !cmp(borders.second, result)));

        if(cmp(value, borders.first))
        {
            REQUIRE(std::addressof(result) == std::addressof(borders.first));
        }
        else if(cmp(borders.second, value))
        {
            REQUIRE(std::addressof(result) == std::addressof(borders.second));
        }
        else
        {
            REQUIRE(std::addressof(result) == std::addressof(value));
        }
    };

    static_assert(saga::clamp(1, 5, 3, std::greater<>{}) == 3, "");
    static_assert(saga::clamp(4, 5, 3, std::greater<>{}) == 4, "");
    static_assert(saga::clamp(7, 5, 3, std::greater<>{}) == 5, "");
}

namespace
{
    template <class Container1, class Container2, class... Args>
    void test_is_permutation(Container1 const & src1, Container2 const & src2, Args... bin_pred)
    {
        static_assert(sizeof...(Args) <= 1);

        auto const cur1 = saga_test::random_subcursor_of(saga::cursor::all(src1));
        auto const cur2 = saga_test::random_subcursor_of(saga::cursor::all(src2));

        CAPTURE(src1, src2, Container1(cur1.begin(), cur1.end())
                , Container2(cur2.begin(), cur2.end()));

        REQUIRE(saga::is_permutation(cur1, cur2, bin_pred...)
                == std::is_permutation(cur1.begin(), cur1.end()
                                       , cur2.begin(), cur2.end(), bin_pred...));
    }

    template <class Container, class... Args>
    void test_is_permutation_always_true(Container const & src1, Args... bin_pred)
    {
        static_assert(sizeof...(Args) <= 1);

        auto const cur1 = saga_test::random_subcursor_of(saga::cursor::all(src1));

        auto src2 = src1;
        auto const cur2 = saga::rebase_cursor(cur1, src2);

        std::shuffle(cur2.begin(), cur2.end(), saga_test::random_engine());

        REQUIRE(saga::is_permutation(cur1, cur2, bin_pred...));
    }
}

TEST_CASE("is_permutation")
{
    using Value1 = int;
    using Value2 = long;

    using Container1 = std::forward_list<Value1>;
    using Container2 = std::forward_list<Value2>;

    saga_test::property_checker
    << ::test_is_permutation<Container1, Container2>
    << ::test_is_permutation_always_true<std::vector<Value1>>
    << [](Container1 const & src1, Container2 const & src2)
    {
        auto const bin_pred = saga::equivalent_up_to([](Value2 const & arg) { return arg % 10; });

        ::test_is_permutation(src1, src2, bin_pred);
    }
    << [](std::vector<Value2> const & src1)
    {
        auto const bin_pred = saga::equivalent_up_to([](Value2 const & arg) { return arg % 10; });

        ::test_is_permutation_always_true(src1, bin_pred);
    };
}

namespace
{
    template <class Value, class... Args>
    void test_next_permutation(std::list<Value> const & values_old, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        // saga
        auto values_saga = values_old;

        auto const input_saga = saga_test::random_subcursor_of(saga::cursor::all(values_saga));

        auto const result_saga = saga::next_permutation(input_saga, cmp...);

        // std
        auto values_std = values_old;

        auto const input_std = saga::rebase_cursor(input_saga, values_std);

        auto const result_std = std::next_permutation(input_std.begin(), input_std.end(), cmp...);

        // Сравнение
        REQUIRE(values_saga == values_std);
        REQUIRE(result_saga == result_std);
    }
}

TEST_CASE("next_permutation")
{
    using Value = int;

    saga_test::property_checker
    << ::test_next_permutation<Value>
    << [](std::list<Value> const & values_old)
    {
        auto const cmp = saga::compare_by([](Value const & arg) { return arg % 100; });

        ::test_next_permutation(values_old, cmp);
    };
}

namespace
{
    template <class Value, class... Args>
    void test_prev_permutation(std::list<Value> const & values_old, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        // saga
        auto values_saga = values_old;

        auto const input_saga = saga_test::random_subcursor_of(saga::cursor::all(values_saga));

        auto const result_saga = saga::prev_permutation(input_saga, cmp...);

        // std
        auto values_std = values_old;

        auto const input_std = saga::rebase_cursor(input_saga, values_std);

        auto const result_std = std::prev_permutation(input_std.begin(), input_std.end(), cmp...);

        // Сравнение
        REQUIRE(values_saga == values_std);
        REQUIRE(result_saga == result_std);
    }
}

TEST_CASE("prev_permutation")
{
    using Value = int;

    saga_test::property_checker
    << ::test_prev_permutation<Value>
    << [](std::list<Value> const & values_old)
    {
        auto const cmp = saga::compare_by([](Value const & arg) { return arg % 100; });

        ::test_prev_permutation(values_old, cmp);
    };
}

namespace
{
    template <class Value1, class Value2, class... Args>
    void test_lexicographical_compare_minimal(std::vector<Value1> const & src1
                                              , std::vector<Value2> const & src2, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        auto src1_in = saga_test::make_istringstream_from_range(src1);
        auto src2_in = saga_test::make_istringstream_from_range(src2);

        REQUIRE(saga::lexicographical_compare(saga::make_istream_cursor<Value1>(src1_in)
                                              , saga::make_istream_cursor<Value2>(src2_in)
                                              , cmp...)
                == std::lexicographical_compare(src1.begin(), src1.end()
                                                , src2.begin(), src2.end(), cmp...));

        REQUIRE(!saga::lexicographical_compare(saga::cursor::all(src1)
                                               , saga::cursor::all(src1), cmp...));
        REQUIRE(!saga::lexicographical_compare(saga::cursor::all(src2)
                                               , saga::cursor::all(src2), cmp...));
    }

    template <class Value1, class Value2, class... Args>
    void test_lexicographical_compare_subcursor(std::vector<Value1> const & src1
                                                , std::vector<Value2> const & src2, Args... cmp)
    {
        static_assert(sizeof...(Args) <= 1);

        auto in1 = saga_test::random_subcursor_of(saga::cursor::all(src1));
        auto in2 = saga_test::random_subcursor_of(saga::cursor::all(src2));

        REQUIRE(saga::lexicographical_compare(in1, in2, cmp...)
                == std::lexicographical_compare(in1.begin(), in1.end()
                                                , in2.begin(), in2.end(), cmp...));

        REQUIRE(!saga::lexicographical_compare(in1, in1, cmp...));
        REQUIRE(!saga::lexicographical_compare(in2, in2, cmp...));
    }
}

TEST_CASE("lexicographical_compare")
{
    using Value1 = int;
    using Value2 = long;

    saga_test::property_checker
    << ::test_lexicographical_compare_minimal<Value1, Value2>
    << ::test_lexicographical_compare_subcursor<Value1, Value2>
    << [](std::vector<Value1> const & src1, std::vector<Value2> const & src2)
    {
        ::test_lexicographical_compare_minimal(src1, src2, std::greater<>{});
    }
    << [](std::vector<Value1> const & src1, std::vector<Value2> const & src2)
    {
        ::test_lexicographical_compare_subcursor(src1, src2, std::greater<>{});
    };
}

TEST_CASE("lexicographical_compare - prefix")
{
    saga_test::property_checker << [](std::string const & str)
    {
        auto const sub = std::string(str.begin(), saga_test::random_iterator_of(str));

        REQUIRE(saga::lexicographical_compare(saga::cursor::all(sub), saga::cursor::all(str))
                == (sub.size() < str.size()));
    };
}

TEST_CASE("lexicographical_compare - prefix, custom compare")
{
    saga_test::property_checker << [](std::string const & str)
    {
        auto const fun = SAGA_OVERLOAD_SET(std::tolower);

        auto sub = std::string(str.begin(), saga_test::random_iterator_of(str));
        saga::transform(saga::cursor::all(sub), saga::cursor::all(sub), fun);

        auto const cmp = saga::compare_by(fun);

        REQUIRE(saga::lexicographical_compare(saga::cursor::all(sub), saga::cursor::all(str), cmp)
                == (sub.size() < str.size()));
    };
}

TEST_CASE("starts_with : prefix")
{
    saga_test::property_checker << [](std::string const & str)
    {
        auto const prefix = str.substr(0, str.size() / 2);

        REQUIRE(::saga::starts_with(saga::cursor::all(str), saga::cursor::all(prefix)));
    };
}

TEST_CASE("starts_with : common")
{
    saga_test::property_checker << [](std::string const & str, std::string const & test)
    {
        auto const expected = test.size() <= str.size() && test == str.substr(0, test.size());

        CAPTURE(str, test);

        REQUIRE(::saga::starts_with(saga::cursor::all(str), saga::cursor::all(test)) == expected);
    };
}

TEST_CASE("ends_with : suffix")
{
    saga_test::property_checker << [](std::string const & str)
    {
        auto const suffix = str.substr(str.size() / 2);

        REQUIRE(::saga::ends_with(saga::cursor::all(str), saga::cursor::all(suffix)));
    };
}

TEST_CASE("ends_with : common")
{
    saga_test::property_checker << [](std::string const & str, std::string const & test)
    {
        auto const expected = test.size() <= str.size()
                            && test == str.substr(str.size() - test.size());

        CAPTURE(str, test);

        REQUIRE(::saga::ends_with(saga::cursor::all(str), saga::cursor::all(test)) == expected);
    };
}

TEST_CASE("starts_with, ends_with: minimalistic")
{
    using Value = int;
    using Container = std::forward_list<Value>;

    saga_test::property_checker << [](Container const & str)
    {
        auto const input = saga::cursor::all(str);

        REQUIRE(saga::starts_with(input, input));
        REQUIRE(saga::ends_with(input, input));
    };
}

namespace
{
    struct counter
    {
        int value = 0;

        constexpr void operator()()
        {
            ++ value;
        }
    };

    constexpr int add_via_counter(int initial, int num)
    {
        return saga::for_n(num, counter{initial}).value;
    }
}

TEST_CASE("for_n")
{
    saga_test::property_checker << [](saga_test::container_size<int> const & initial
                                     ,saga_test::container_size<int> const & num)
    {
        auto result = saga::for_n(num.value, ::counter{initial.value});

        REQUIRE(result.value == initial.value + num.value);

        static_assert(std::is_same<decltype(result), counter>{}, "");
    };
}

TEST_CASE("for_n: constexpr")
{
    constexpr auto const initial = 21;
    constexpr auto const num = 5;

    static_assert(::add_via_counter(initial, num) == initial + num, "");
}

namespace
{
    template <class Container, class... Args>
    void test_is_palindrome(Container const & values, Args... bin_pred)
    {
        static_assert(sizeof...(Args) <= 1);

        auto const src = saga_test::random_subcursor_of(saga::cursor::all(values));

        Container src_r(src.begin(), src.end());
        src_r.reverse();

        REQUIRE(saga::is_palindrome(src, bin_pred...)
                == saga::equal(src, saga::cursor::all(src_r), bin_pred...));
    }
}

TEST_CASE("is_palindrome : subrange")
{
    using Container = std::list<int>;

    saga_test::property_checker
    << ::test_is_palindrome<Container>
    << [](Container const & values)
    {
        using Value = typename Container::value_type;

        auto const bin_pred = saga::equivalent_up_to([](Value const & arg) { return arg % 5; });

        ::test_is_palindrome(values, bin_pred);
    };
}

TEST_CASE("is_palindrome: always true")
{
    using Container = std::list<int>;

    saga_test::property_checker << [](Container const & values)
    {
        Container values_p(values);
        saga::reverse_copy(saga::cursor::all(values), saga::back_inserter(values_p));

        REQUIRE(saga::is_palindrome(saga::cursor::all(values_p)));
    };
}

#include <saga/numeric.hpp>

TEST_CASE("adjacent_count: default predicate")
{
    using Value = int;
    using Container = std::forward_list<Value>;

    saga_test::property_checker <<[](Container const & values)
    {
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(values));

        auto const actual = saga::adjacent_count(saga::cursor::all(input));

        // Проверка
        if(!input)
        {
            REQUIRE(actual == 0);
        }
        else
        {
            auto const input2 = saga::cursor::drop_front_n(input, 1);

            auto const expected = saga::inner_product(input, input2, std::ptrdiff_t{0}
                                                       , std::plus<>{}, std::equal_to<>{});

            REQUIRE(actual == expected);
        }
    };
}

TEST_CASE("adjacent_count: custom predicate")
{
    using Value = int;
    using Container = std::forward_list<Value>;

    saga_test::property_checker <<[](Container const & values)
    {
        auto const pred = saga::compare_by([](Value const & arg){ return arg % 3; }
                                          , std::not_equal_to<>{});

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(values));

        auto const actual = saga::adjacent_count(saga::cursor::all(input), pred);

        // Проверка
        if(!input)
        {
            REQUIRE(actual == 0);
        }
        else
        {
            auto const input2 = saga::cursor::drop_front_n(input, 1);

            auto const expected = saga::inner_product(input, input2, std::ptrdiff_t{0}
                                                       , std::plus<>{}, pred);

            REQUIRE(actual == expected);
        }
    };
}

namespace
{
    struct with_unspec_state
    {
        int value{0};
        bool specified_state{true};

        with_unspec_state(int v = 0) : value{v} {}
        with_unspec_state(with_unspec_state const& rhs) = default;
        with_unspec_state(with_unspec_state&& rhs) { *this = std::move(rhs); }
        with_unspec_state& operator=(with_unspec_state const& rhs) = default;
        with_unspec_state& operator=(with_unspec_state&& rhs)
        {
            if (this != &rhs)
            {
                value = rhs.value;
                specified_state = rhs.specified_state;
                rhs.specified_state = false;
            }
            return *this;
        }
    };

    bool operator==(with_unspec_state const & lhs, with_unspec_state const & rhs)
    {
        if(!lhs.specified_state)
        {
            return !rhs.specified_state;
        }

        return lhs.value == rhs.value;
    }

    std::ostream& operator<< (std::ostream& os, with_unspec_state const & val)
    {
        val.specified_state ? os << val.value : os << '.';

        return os;
    }
}

template <>
struct saga_test::arbitrary<with_unspec_state>
{
    template <class UniformRandomBitGenerator>
    static with_unspec_state generate(generation_t generation, UniformRandomBitGenerator & urbg)
    {
        if(generation == 0)
        {
            return {};
        }
        else
        {
            return saga_test::arbitrary<int>::generate(generation - 1, urbg);
        }
    }
};

namespace
{
    using Shift_algo_containers = std::tuple<std::forward_list<with_unspec_state>
                                            , std::list<with_unspec_state>
                                            , std::vector<with_unspec_state>>;
}

TEMPLATE_LIST_TEST_CASE("shift_left : generic", "saga_test", Shift_algo_containers)
{
    using Container = TestType;

    saga_test::property_checker << [](Container const & values_old)
    {
        // Подготовка
        auto values = values_old;

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(values));
        auto const input_size = saga::cursor::size(input);

        auto const num = saga_test::random_uniform(0, input_size + 10);

        // Выполнение
        auto const result = saga::shift_left(input, num);

        // Проверка
        if(num == 0 || num >= input_size)
        {
            REQUIRE(values == values_old);
        }
        else
        {
            auto values_move = values_old;
            auto const input_move = saga::rebase_cursor(input, values_move);

            saga::move(saga::cursor::drop_front_n(input_move, num), input_move);

            REQUIRE(values == values_move);
        }

        // Сравнение
        CAPTURE(values_old, values, input, num);

        REQUIRE(result.dropped_front().begin() == input.begin());
        REQUIRE(result.begin() == std::next(input.begin(), input_size - std::min(num, input_size)));
        REQUIRE(result.end() == input.end());
    };
}

TEMPLATE_LIST_TEST_CASE("shift_right : generic", "saga_test", Shift_algo_containers)
{
    using Container = TestType;

    saga_test::property_checker << [](Container const & values_old)
    {
        // Подготовка
        auto values = values_old;

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(values));
        auto const input_size = saga::cursor::size(input);

        auto const num = saga_test::random_uniform(0, input_size);

        // Выполнение
        auto const result = saga::shift_right(input, num);

        // Проверка
        if(num == 0 || num >= input_size)
        {
            REQUIRE(values == values_old);
        }
        else
        {
            auto values_move
                = std::vector<typename Container::value_type>(values_old.begin(), values_old.end());
            auto const input_move = saga::rebase_cursor(input, values_move);

            saga::move_backward(saga::cursor::drop_back_n(input_move, num), input_move);

            CAPTURE(values_old, values, input, values_move, num);

            REQUIRE(saga::equal(saga::cursor::all(values), saga::cursor::all(values_move)));
        }

        // Сравнение
        REQUIRE(result.dropped_front().begin() == input.begin());
        REQUIRE(result.begin() == std::next(input.begin(), std::min(num, input_size)));
        REQUIRE(result.end() == input.end());
    };
}
