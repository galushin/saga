/* (c) 2020-2021 Галушин Павел Викторович, galushin@gmail.com

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
#include <catch/catch.hpp>

// Вспомогательные файлы
#include <saga/cursor/subrange.hpp>
#include <saga/cursor/istream_cursor.hpp>
#include <saga/iterator/reverse.hpp>
#include <saga/view/indices.hpp>

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

TEST_CASE("equal - minimal, default predicate")
{
    using Value1 = int;
    using Value2 = long;

    saga_test::property_checker
    << [](std::vector<Value1> const & src1, std::vector<Value2> const & src2)
    {
        auto src1_in = saga_test::make_istringstream_from_range(src1);
        auto src2_in = saga_test::make_istringstream_from_range(src2);

        REQUIRE(saga::equal(saga::make_istream_cursor<Value1>(src1_in),
                            saga::make_istream_cursor<Value2>(src2_in))
                == std::equal(src1.begin(), src1.end(), src2.begin(), src2.end()));

        REQUIRE(saga::equal(saga::cursor::all(src1), saga::cursor::all(src1)));
        REQUIRE(saga::equal(saga::cursor::all(src2), saga::cursor::all(src2)));
    };
}

TEST_CASE("equal - minimal, custom predicate")
{
    using Value1 = int;
    using Value2 = long;

    saga_test::property_checker
    << [](std::vector<Value1> const & src1, std::vector<Value2> const & src2)
    {
        CAPTURE(src1, src2);

        auto const bin_pred = [](auto const & x, auto const & y) { return x % 2 == y % 2; };

        auto src1_in = saga_test::make_istringstream_from_range(src1);
        auto src2_in = saga_test::make_istringstream_from_range(src2);

        REQUIRE(saga::equal(saga::make_istream_cursor<Value1>(src1_in),
                            saga::make_istream_cursor<Value2>(src2_in), bin_pred)
                == std::equal(src1.begin(), src1.end(), src2.begin(), src2.end(), bin_pred));

        REQUIRE(saga::equal(saga::cursor::all(src1), saga::cursor::all(src1), bin_pred));
        REQUIRE(saga::equal(saga::cursor::all(src2), saga::cursor::all(src2), bin_pred));
    };
}

TEST_CASE("equal - custom predicate, invented true")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src1)
    {
        std::vector<Value> src2;
        saga::transform(saga::cursor::all(src1), saga::back_inserter(src2),
                        [](Value const & x) { return x % 2; });

        CAPTURE(src1, src2);

        auto const bin_pred
            = [](Value const & x, Value const & y) { return x % 2 == y % 2; };

        REQUIRE(saga::equal(saga::cursor::all(src1), saga::cursor::all(src2), bin_pred));
    };
}

TEST_CASE("equal: subcursor, default predicate")
{
    using Value1 = int;
    using Value2 = long;

    saga_test::property_checker
    << [](std::vector<Value1> const & src1, std::list<Value2> const & src2)
    {
        auto const in1 = saga_test::random_subcursor_of(saga::cursor::all(src1));
        auto const in2 = saga_test::random_subcursor_of(saga::cursor::all(src2));

        REQUIRE(saga::equal(in1, in2)
                == std::equal(in1.begin(), in1.end(), in2.begin(), in2.end()));

        REQUIRE(saga::equal(in1, in1));
        REQUIRE(saga::equal(in2, in2));
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

TEST_CASE("equal: subcursor, custom predicate")
{
    using Value1 = int;
    using Value2 = long;

    saga_test::property_checker
    << [](std::vector<Value1> const & src1, std::vector<Value2> const & src2)
    {
        auto const bin_pred = [](auto const & x, auto const & y) { return x % 2 == y % 2; };

        auto const in1 = saga_test::random_subcursor_of(saga::cursor::all(src1));
        auto const in2 = saga_test::random_subcursor_of(saga::cursor::all(src2));

        REQUIRE(saga::equal(in1, in2, bin_pred)
                == std::equal(in1.begin(), in1.end(), in2.begin(), in2.end(), bin_pred));

        REQUIRE(saga::equal(in1, in1, bin_pred));
        REQUIRE(saga::equal(in2, in2, bin_pred));
    };
}

TEST_CASE("equal - subcursor, custom predicate, invented")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src1)
    {
        std::vector<Value> src2;
        saga::transform(saga::cursor::all(src1), saga::back_inserter(src2),
                        [](Value const & x) { return x % 2; });

        CAPTURE(src1, src2);

        auto const bin_pred = [](auto const & x, auto const & y) { return x % 2 == y % 2; };

        auto const in1 = saga_test::random_subcursor_of(saga::cursor::all(src1));
        auto const in2 = saga_test::random_subcursor_of(saga::cursor::all(src2));

        REQUIRE(saga::equal(in1, in2, bin_pred)
                == std::equal(in1.begin(), in1.end(), in2.begin(), in2.end(), bin_pred));
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

namespace
{
    template <class T>
    struct sum_accumulator
    {
        void operator()(T const & x)
        {
            this->sum += x;
        }

        T sum{0};
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
                                                , ::sum_accumulator<Value>{});

        // std
        auto const result_std = std::for_each(src.begin(), src.end(), ::sum_accumulator<Value>{});

        // Сравение
        REQUIRE(result_saga.fun.sum == result_std.sum);
    };
}

TEST_CASE("for_each - subcursor, const")
{
    using Value = unsigned;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto const sub = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto const result_saga = saga::for_each(sub, ::sum_accumulator<Value>{});

        // std
        auto const result_std = std::for_each(sub.begin(), sub.end(), ::sum_accumulator<Value>{});

        // Сравение
        REQUIRE(result_saga.in.begin() == sub.end());
        REQUIRE(result_saga.in.end() == sub.end());

        REQUIRE(result_saga.fun.sum == result_std.sum);
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
        auto const num = saga_test::random_uniform(0*src.size(), src.size());

        auto src_in = saga_test::make_istringstream_from_range(src);

        auto const result_saga = saga::for_each_n(saga::make_istream_cursor<Value>(src_in)
                                                  , num, ::sum_accumulator<Value>{});

        // std
        auto const result_std = std::for_each(src.begin(), src.begin() + num
                                              , ::sum_accumulator<Value>{});

        // Сравение
        REQUIRE(result_saga.fun.sum == result_std.sum);
        REQUIRE(result_saga.fun.sum == result_std.sum);
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
        auto const result_saga = saga::for_each_n(sub, num.value, ::sum_accumulator<Value>{});

        // std
        auto const n_common = std::min(sub.size(), num.value);

        auto const result_std = std::for_each(sub.begin(), sub.begin() + n_common
                                              , ::sum_accumulator<Value>{});

        // Сравение
        auto result_expected = sub;
        result_expected.drop_front(n_common);

        REQUIRE(result_saga.in == result_expected);

        REQUIRE(result_saga.fun.sum == result_std.sum);
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

TEST_CASE("cout_if - minimal")
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

TEST_CASE("cout_if - subcursor")
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
        auto const pred = [](Value1 const & x, Value2 const & y) { return x % 7 == y % 7; };

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
        auto const bin_pred = [](auto const & x, auto const & y) { return x % 7 == y % 7; };

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
        REQUIRE(r_saga.end() == haystack_cur.end());
        REQUIRE(r_saga.dropped_front().begin() == haystack.begin());
        REQUIRE(r_saga.dropped_back().end() == haystack.end());
    };
}

TEST_CASE("find_end: custom predicate, minimalistic")
{
    using Value = int;

    saga_test::property_checker
    <<[](std::forward_list<Value> const & haystack, std::forward_list<Value> const & needle)
    {
        auto const pred = [](Value const & lhs, Value const & rhs)
        {
            return lhs % 2 == rhs % 2;
        };

        auto const haystack_cur = saga_test::random_subcursor_of(saga::cursor::all(haystack));
        auto const needle_cur = saga_test::random_subcursor_of(saga::cursor::all(needle));

        auto const r_saga = saga::find_end(haystack_cur, needle_cur, pred);
        auto const r_std = std::find_end(haystack_cur.begin(), haystack_cur.end()
                                         , needle_cur.begin(), needle_cur.end(), pred);

        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(r_saga.end() == haystack_cur.end());
        REQUIRE(r_saga.dropped_front().begin() == haystack.begin());
        REQUIRE(r_saga.dropped_back().end() == haystack.end());
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

        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(r_saga.end() == haystack_cur.end());
        REQUIRE(r_saga.dropped_front().begin() == haystack.begin());
        REQUIRE(r_saga.dropped_back().end() == haystack.end());
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

        auto const pred = [](Value const & lhs, Value const & rhs)
        {
            return lhs % 5 == rhs % 5;
        };

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

        auto const pred = [](Value const & lhs, Value const & rhs)
        {
            return lhs % 5 == rhs % 5;
        };

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
        REQUIRE(r_saga.end() == haystack_cur.end());
        REQUIRE(r_saga.dropped_front().begin() == haystack.begin());
        REQUIRE(r_saga.dropped_back().end() == haystack.end());
    };
}

TEST_CASE("search: custom predicate, minimalistic")
{
    using Value = int;

    saga_test::property_checker
    <<[](std::forward_list<Value> const & haystack, std::forward_list<Value> const & needle)
    {
        auto const pred = [](Value const & lhs, Value const & rhs)
        {
            return lhs % 2 == rhs % 2;
        };

        auto const haystack_cur = saga_test::random_subcursor_of(saga::cursor::all(haystack));
        auto const needle_cur = saga_test::random_subcursor_of(saga::cursor::all(needle));

        auto const r_saga = saga::search(haystack_cur, needle_cur, pred);
        auto const r_std = std::search(haystack_cur.begin(), haystack_cur.end()
                                       , needle_cur.begin(), needle_cur.end(), pred);

        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(r_saga.end() == haystack_cur.end());
        REQUIRE(r_saga.dropped_front().begin() == haystack.begin());
        REQUIRE(r_saga.dropped_back().end() == haystack.end());
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
        REQUIRE(r_saga.end() == haystack_cur.end());
        REQUIRE(r_saga.dropped_front().begin() == haystack.begin());
        REQUIRE(r_saga.dropped_back().end() == haystack.end());
    };
}

TEST_CASE("search_n: default predicate, minimalistic")
{
    using Value = int;

    saga_test::property_checker
    <<[](std::forward_list<Value> const & haystack
         , saga_test::container_size<std::size_t> const & num, Value const & value)
    {
        auto const haystack_cur = saga_test::random_subcursor_of(saga::cursor::all(haystack));

        auto const r_saga = saga::search_n(haystack_cur, num.value, value);
        auto const r_std = std::search_n(haystack_cur.begin(), haystack_cur.end()
                                         , num.value, value);

        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(r_saga.end() == haystack_cur.end());
        REQUIRE(r_saga.dropped_front().begin() == haystack.begin());
        REQUIRE(r_saga.dropped_back().end() == haystack.end());
    };
}

TEST_CASE("search_n: custom predicate, minimalistic")
{
    using Value = int;

    saga_test::property_checker
    <<[](std::forward_list<Value> const & haystack
         , saga_test::container_size<std::size_t> const & num, Value const & value)
    {
        auto const pred = [](Value const & lhs, Value const & rhs)
        {
            return lhs % 2 == rhs % 2;
        };

        auto const haystack_cur = saga_test::random_subcursor_of(saga::cursor::all(haystack));

        auto const r_saga = saga::search_n(haystack_cur, num.value, value, pred);
        auto const r_std = std::search_n(haystack_cur.begin(), haystack_cur.end()
                                         , num.value, value, pred);

        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(r_saga.end() == haystack_cur.end());
        REQUIRE(r_saga.dropped_front().begin() == haystack.begin());
        REQUIRE(r_saga.dropped_back().end() == haystack.end());
    };
}

TEST_CASE("search_n: default predicate, guaranty")
{
    using Value = int;

    saga_test::property_checker
    <<[](std::list<Value> const & prefix, std::list<Value> const & suffix
         , saga_test::container_size<std::size_t> const & num, Value const & value)
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
        REQUIRE(r_saga.end() == haystack_cur.end());
        REQUIRE(r_saga.dropped_front().begin() == haystack.begin());
        REQUIRE(r_saga.dropped_back().end() == haystack.end());
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

        for(auto index : saga::view::indices(dest_prefix_size))
        {
            addresses.push_back(dest_saga[index].data());
        }

        for(auto index : saga::view::indices(n_common))
        {
            addresses.push_back(src_saga.at(src_prefix_size + index).data());
        }

        for(auto index : saga::view::indices(dest_prefix_size + n_common, dest.size()))
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

        for(auto index : saga::view::indices(dest_prefix_size))
        {
            REQUIRE(dest_saga[index] == dest[index]);
        }

        for(auto index : saga::view::indices(n_common))
        {
            REQUIRE(dest_saga[dest_prefix_size + index] == src[src_prefix_size + index]);
        }

        for(auto index : saga::view::indices(dest_prefix_size + n_common, dest.size()))
        {
            REQUIRE(dest_saga[index] == dest[index]);
        }

        // Проверить адреса
        for(auto index : saga::view::indices_of(dest))
        {
            REQUIRE(dest_saga[index].data() == addresses[index]);
        }
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
        auto result_expected = sub_saga;
        saga::cursor::drop_front_n(result_expected, n_common);

        CHECK(result == result_expected);

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
        auto fun = [](char arg) { return std::toupper(arg); };

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
    struct square_fn
    {
        constexpr int operator()(int x) const
        {
            return x*x;
        }
    };

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

        saga::transform(saga::cursor::all(arr), saga::cursor::all(arr), ::square_fn{});

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
        auto const fun = [](char x, char y) { return std::min(x, y); };

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

    saga_test::property_checker << [](saga_test::container_size<std::ptrdiff_t> num)
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
        auto result_expected = sub_saga;
        saga::cursor::drop_front_n(result_expected, n_common);

        CHECK(result == result_expected);

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

        auto in1_saga_expected = lhs_saga_cur;
        saga::cursor::drop_front_n(in1_saga_expected, n_common);

        auto in2_saga_expected = rhs_saga_cur;
        saga::cursor::drop_front_n(in2_saga_expected, n_common);

        REQUIRE(r_saga.in1 == in1_saga_expected);
        REQUIRE(r_saga.in2 == in2_saga_expected);

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

TEST_CASE("unique_copy: minimal, default predicate")
{
    using Value = long;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        // saga
        auto src_in = saga_test::make_istringstream_from_range(src);

        std::vector<Value> dest_saga;

        saga::unique_copy(saga::make_istream_cursor<Value>(src_in), saga::back_inserter(dest_saga));

        // std
        std::vector<Value> dest_std;
        std::unique_copy(src.begin(), src.end(), std::back_inserter(dest_std));

        // Сравнение
        REQUIRE(dest_saga == dest_std);
    };
}

TEST_CASE("unique_copy: minimal, custom predicate")
{
    using Value = long;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto const bin_pred = [](Value const & x, Value const & y) { return x % 7 == y % 7;};
        // saga
        auto src_in = saga_test::make_istringstream_from_range(src);

        std::vector<Value> dest_saga;

        saga::unique_copy(saga::make_istream_cursor<Value>(src_in)
                          , saga::back_inserter(dest_saga), bin_pred);

        // std
        std::vector<Value> dest_std;
        std::unique_copy(src.begin(), src.end(), std::back_inserter(dest_std), bin_pred);

        // Сравнение
        REQUIRE(dest_saga == dest_std);
    };
}

TEST_CASE("unique - subcursors")
{
    using Value = char;

    saga_test::property_checker << [](std::vector<Value> const & src
                                      , std::vector<Value> const & dest_old)
    {
        auto const bin_pred = [](char x, char y) { return x == ' ' && y == ' '; };
        // Подготовка
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto dest_saga = dest_old;
        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto const result_saga = saga::unique_copy(input, out_saga, bin_pred);

        // std
        auto dest_std = dest_old;
        auto const out_std = dest_std.begin() + (out_saga.begin() - dest_saga.begin());

        auto const result_std = std::unique_copy(input.begin(), result_saga.in.begin(), out_std
                                                 , bin_pred);

        // Проверки
        REQUIRE(dest_saga == dest_std);

        REQUIRE(result_saga.in.end() == input.end());

        REQUIRE((result_saga.out.begin() - out_saga.begin()) == (result_std - out_std));
        REQUIRE(result_saga.out.end() == out_saga.end());
    };
}

TEST_CASE("unique - subcursors, custom predicate")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src
                                      , std::vector<Value> const & dest_old)
    {
        // Подготовка
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto dest_saga = dest_old;
        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto const result_saga = saga::unique_copy(input, out_saga);

        // std
        auto dest_std = dest_old;
        auto const out_std = dest_std.begin() + (out_saga.begin() - dest_saga.begin());

        auto const result_std = std::unique_copy(input.begin(), result_saga.in.begin(), out_std);

        // Проверки
        REQUIRE(dest_saga == dest_std);

        REQUIRE(result_saga.in.end() == input.end());

        REQUIRE((result_saga.out.begin() - out_saga.begin()) == (result_std - out_std));
        REQUIRE(result_saga.out.end() == out_saga.end());
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

TEST_CASE("is_sorted: default compare")
{
    using Value = int;
    using Container = std::forward_list<Value>;

    saga_test::property_checker << [](Container const & src)
    {
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const result_std = std::is_sorted(input.begin(), input.end());
        auto const result_saga = saga::is_sorted(input);

        REQUIRE(result_saga == result_std);
    };
}

TEST_CASE("is_sorted: default compare, sorted")
{
    using Value = int;
    using Container = std::vector<Value>;

    saga_test::property_checker << [](Container src)
    {
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        std::sort(input.begin(), input.end());

        REQUIRE(saga::is_sorted(input));
    };
}

TEST_CASE("is_sorted: custom compare")
{
    using Value = int;
    using Container = std::forward_list<Value>;

    saga_test::property_checker << [](Container const & src)
    {
        auto const cmp = std::greater<>{};

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const result_std = std::is_sorted(input.begin(), input.end(), cmp);
        auto const result_saga = saga::is_sorted(input, cmp);

        REQUIRE(result_std == result_saga);
    };
}

TEST_CASE("is_sorted: custom compare, sorted")
{
    using Value = int;
    using Container = std::vector<Value>;

    saga_test::property_checker << [](Container src)
    {
        auto const cmp = std::greater<>{};

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        std::sort(input.begin(), input.end(), cmp);

        REQUIRE(saga::is_sorted(input, cmp));
    };
}

TEST_CASE("is_sorted_until: default compare")
{
    using Value = int;
    using Container = std::forward_list<Value>;

    saga_test::property_checker << [](Container const & src)
    {
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const result_std = std::is_sorted_until(input.begin(), input.end());
        auto const result_saga = saga::is_sorted_until(input);

        CAPTURE(src);

        REQUIRE(result_saga.begin() == result_std);
        REQUIRE(result_saga.end() == input.end());

        REQUIRE(result_saga.dropped_front().begin() == src.begin());
        REQUIRE(result_saga.dropped_front().end() == result_std);
    };
}

TEST_CASE("is_sorted_until: custom compare")
{
    using Value = int;
    using Container = std::forward_list<Value>;

    saga_test::property_checker << [](Container const & src)
    {
        auto const cmp = std::greater<>{};

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const result_std = std::is_sorted_until(input.begin(), input.end(), cmp);
        auto const result_saga = saga::is_sorted_until(input, cmp);

        CAPTURE(src);

        REQUIRE(result_saga.begin() == result_std);
        REQUIRE(result_saga.end() == input.end());

        REQUIRE(result_saga.dropped_front().begin() == src.begin());
        REQUIRE(result_saga.dropped_front().end() == result_std);
    };
}

TEST_CASE("merge : minimal, default compare")
{
    using Value = int;
    using Container = std::vector<Value>;

    saga_test::property_checker << [](Container lhs, Container rhs)
    {
        std::sort(lhs.begin(), lhs.end());
        std::sort(rhs.begin(), rhs.end());

        CAPTURE(lhs, rhs);

        // std
        std::vector<Value> diff_std;

        std::merge(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()
                   , std::back_inserter(diff_std));

        // saga
        std::vector<Value> diff_saga;

        auto lhs_in = saga_test::make_istringstream_from_range(lhs);
        auto rhs_in = saga_test::make_istringstream_from_range(rhs);

        saga::merge(saga::make_istream_cursor<Value>(lhs_in)
                    , saga::make_istream_cursor<Value>(rhs_in)
                    , saga::back_inserter(diff_saga));

        // Проверка
        REQUIRE(diff_saga == diff_std);
    };
}

TEST_CASE("merge : minimal, custom compare")
{
    using Value = int;
    using Container = std::vector<Value>;

    saga_test::property_checker << [](Container lhs, Container rhs)
    {
        auto const cmp = std::greater<>{};

        std::sort(lhs.begin(), lhs.end(), cmp);
        std::sort(rhs.begin(), rhs.end(), cmp);

        // std
        std::vector<Value> diff_std;

        std::merge(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()
                   , std::back_inserter(diff_std), cmp);

        // saga
        std::vector<Value> diff_saga;

        auto lhs_in = saga_test::make_istringstream_from_range(lhs);
        auto rhs_in = saga_test::make_istringstream_from_range(rhs);

        saga::merge(saga::make_istream_cursor<Value>(lhs_in)
                    , saga::make_istream_cursor<Value>(rhs_in)
                    , saga::back_inserter(diff_saga), cmp);

        // Проверка
        REQUIRE(diff_saga == diff_std);
    };
}

TEST_CASE("includes - minimal")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> in1, std::vector<Value> in2)
    {
        std::sort(in1.begin(), in1.end());
        std::sort(in2.begin(), in2.end());

        CAPTURE(in1, in2);

        auto in1_istream = saga_test::make_istringstream_from_range(in1);
        auto in2_istream = saga_test::make_istringstream_from_range(in2);

        REQUIRE(saga::includes(saga::make_istream_cursor<Value>(in1_istream),
                               saga::make_istream_cursor<Value>(in2_istream))
                == std::includes(in1.begin(), in1.end(), in2.begin(), in2.end()));
    };
}

TEST_CASE("includes - minimal, custom compare")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> in1, std::vector<Value> in2)
    {
        auto const cmp = std::greater<>{};

        std::sort(in1.begin(), in1.end(), cmp);
        std::sort(in2.begin(), in2.end(), cmp);

        CAPTURE(in1, in2);

        auto in1_istream = saga_test::make_istringstream_from_range(in1);
        auto in2_istream = saga_test::make_istringstream_from_range(in2);

        REQUIRE(saga::includes(saga::make_istream_cursor<Value>(in1_istream),
                               saga::make_istream_cursor<Value>(in2_istream), cmp)
                == std::includes(in1.begin(), in1.end(), in2.begin(), in2.end(), cmp));
    };
}

TEST_CASE("includes - subrange")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> in1, std::vector<Value> in2)
    {
        std::sort(in1.begin(), in1.end());
        std::sort(in2.begin(), in2.end());

        CAPTURE(in1, in2);

        REQUIRE(saga::includes(saga::cursor::all(in1), saga::cursor::all(in2))
                == std::includes(in1.begin(), in1.end(), in2.begin(), in2.end()));
    };
}

TEST_CASE("includes - subrange, custom compare")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> in1, std::vector<Value> in2)
    {
        auto const cmp = std::greater<>{};

        std::sort(in1.begin(), in1.end(), cmp);
        std::sort(in2.begin(), in2.end(), cmp);

        CAPTURE(in1, in2);

        REQUIRE(saga::includes(saga::cursor::all(in1), saga::cursor::all(in2), cmp)
                == std::includes(in1.begin(), in1.end(), in2.begin(), in2.end(), cmp));
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

TEST_CASE("set_difference : minimal, default compare")
{
    using Value = int;
    using Container = std::vector<Value>;

    saga_test::property_checker << [](Container lhs, Container rhs)
    {
        std::sort(lhs.begin(), lhs.end());
        std::sort(rhs.begin(), rhs.end());

        CAPTURE(lhs, rhs);

        // std
        std::vector<Value> diff_std;

        std::set_difference(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                            std::back_inserter(diff_std));

        // saga
        std::vector<Value> diff_saga;

        auto lhs_in = saga_test::make_istringstream_from_range(lhs);
        auto rhs_in = saga_test::make_istringstream_from_range(rhs);

        saga::set_difference(saga::make_istream_cursor<Value>(lhs_in),
                             saga::make_istream_cursor<Value>(rhs_in),
                             saga::back_inserter(diff_saga));

        // Проверка
        REQUIRE(diff_saga == diff_std);
    };
}

TEST_CASE("set_difference : minimal, custom compare")
{
    using Value = int;
    using Container = std::vector<Value>;

    saga_test::property_checker << [](Container lhs, Container rhs)
    {
        auto const cmp = std::greater<>{};

        std::sort(lhs.begin(), lhs.end(), cmp);
        std::sort(rhs.begin(), rhs.end(), cmp);

        // std
        std::vector<Value> diff_std;

        std::set_difference(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                            std::back_inserter(diff_std), cmp);

        // saga
        std::vector<Value> diff_saga;

        auto lhs_in = saga_test::make_istringstream_from_range(lhs);
        auto rhs_in = saga_test::make_istringstream_from_range(rhs);

        saga::set_difference(saga::make_istream_cursor<Value>(lhs_in),
                             saga::make_istream_cursor<Value>(rhs_in),
                             saga::back_inserter(diff_saga), cmp);

        // Проверка
        REQUIRE(diff_saga == diff_std);
    };
}

TEST_CASE("set_difference - subcursor, custom compare")
{
    using Value = long;

    saga_test::property_checker
    << [](std::vector<Value> in1_src, std::vector<Value> in2_src,
          std::vector<Value> const & out_src_old)
    {
        auto const cmp = std::greater<>{};

        std::sort(in1_src.begin(), in1_src.end(), cmp);
        std::sort(in2_src.begin(), in2_src.end(), cmp);

        // Исходные данные
        auto const in1 = saga_test::random_subcursor_of(saga::cursor::all(in1_src));
        auto const in2 = saga_test::random_subcursor_of(saga::cursor::all(in2_src));

        // saga
        std::vector<Value> out_saga(out_src_old);
        auto const out = saga_test::random_subcursor_of(saga::cursor::all(out_saga));

        auto const result = saga::set_difference(in1, in2, out, cmp);

        // std
        auto out_std = out_src_old;

        std::set_difference(in1.begin(), result.in.begin(), in2.begin(), in2.end()
                            , saga::rebase_cursor(out, out_std).begin(), cmp);

        // Проверка
        REQUIRE(out_saga == out_std);
    };
}

TEST_CASE("set_intersection : minimal, default compare")
{
    using Value = int;
    using Container = std::vector<Value>;

    saga_test::property_checker << [](Container lhs, Container rhs)
    {
        std::sort(lhs.begin(), lhs.end());
        std::sort(rhs.begin(), rhs.end());

        CAPTURE(lhs, rhs);

        // std
        std::vector<Value> diff_std;

        std::set_intersection(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()
                             , std::back_inserter(diff_std));

        // saga
        std::vector<Value> diff_saga;

        auto lhs_in = saga_test::make_istringstream_from_range(lhs);
        auto rhs_in = saga_test::make_istringstream_from_range(rhs);

        saga::set_intersection(saga::make_istream_cursor<Value>(lhs_in)
                              , saga::make_istream_cursor<Value>(rhs_in)
                              , saga::back_inserter(diff_saga));

        // Проверка
        REQUIRE(diff_saga == diff_std);
    };
}

TEST_CASE("set_intersection : minimal, custom compare")
{
    using Value = int;
    using Container = std::vector<Value>;

    saga_test::property_checker << [](Container lhs, Container rhs)
    {
        auto const cmp = std::greater<>{};

        std::sort(lhs.begin(), lhs.end(), cmp);
        std::sort(rhs.begin(), rhs.end(), cmp);

        // std
        std::vector<Value> diff_std;

        std::set_intersection(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()
                             , std::back_inserter(diff_std), cmp);

        // saga
        std::vector<Value> diff_saga;

        auto lhs_in = saga_test::make_istringstream_from_range(lhs);
        auto rhs_in = saga_test::make_istringstream_from_range(rhs);

        saga::set_intersection(saga::make_istream_cursor<Value>(lhs_in)
                              , saga::make_istream_cursor<Value>(rhs_in)
                              , saga::back_inserter(diff_saga), cmp);

        // Проверка
        REQUIRE(diff_saga == diff_std);
    };
}

TEST_CASE("set_intersection - subcursor, custom compare")
{
    using Value = long;

    saga_test::property_checker
    << [](std::vector<Value> in1_src, std::vector<Value> in2_src,
          std::vector<Value> const & out_src_old)
    {
        auto const cmp = std::greater<>{};

        std::sort(in1_src.begin(), in1_src.end(), cmp);
        std::sort(in2_src.begin(), in2_src.end(), cmp);

        // Исходные данные
        auto const in1 = saga_test::random_subcursor_of(saga::cursor::all(in1_src));
        auto const in2 = saga_test::random_subcursor_of(saga::cursor::all(in2_src));

        // saga
        std::vector<Value> out_saga(out_src_old);
        auto const out = saga_test::random_subcursor_of(saga::cursor::all(out_saga));

        auto const result = saga::set_intersection(in1, in2, out, cmp);

        // std
        auto out_std = out_src_old;
        std::set_intersection(in1.begin(), result.in1.begin(), in2.begin(), result.in2.begin()
                              , saga::rebase_cursor(out, out_std).begin(), cmp);

        // Проверка
        REQUIRE(out_saga == out_std);
    };
}

TEST_CASE("set_symmetric_difference : minimal, default compare")
{
    using Value = int;
    using Container = std::vector<Value>;

    saga_test::property_checker << [](Container lhs, Container rhs)
    {
        std::sort(lhs.begin(), lhs.end());
        std::sort(rhs.begin(), rhs.end());

        CAPTURE(lhs, rhs);

        // std
        std::vector<Value> diff_std;

        std::set_symmetric_difference(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()
                                      , std::back_inserter(diff_std));

        // saga
        std::vector<Value> diff_saga;

        auto lhs_in = saga_test::make_istringstream_from_range(lhs);
        auto rhs_in = saga_test::make_istringstream_from_range(rhs);

        saga::set_symmetric_difference(saga::make_istream_cursor<Value>(lhs_in)
                                       , saga::make_istream_cursor<Value>(rhs_in)
                                       , saga::back_inserter(diff_saga));

        // Проверка
        REQUIRE(diff_saga == diff_std);
    };
}

TEST_CASE("set_symmetric_difference : minimal, custom compare")
{
    using Value = int;
    using Container = std::vector<Value>;

    saga_test::property_checker << [](Container lhs, Container rhs)
    {
        auto const cmp = std::greater<>{};

        std::sort(lhs.begin(), lhs.end(), cmp);
        std::sort(rhs.begin(), rhs.end(), cmp);

        // std
        std::vector<Value> diff_std;

        std::set_symmetric_difference(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()
                                      , std::back_inserter(diff_std), cmp);

        // saga
        std::vector<Value> diff_saga;

        auto lhs_in = saga_test::make_istringstream_from_range(lhs);
        auto rhs_in = saga_test::make_istringstream_from_range(rhs);

        saga::set_symmetric_difference(saga::make_istream_cursor<Value>(lhs_in)
                                       , saga::make_istream_cursor<Value>(rhs_in)
                                       , saga::back_inserter(diff_saga), cmp);

        // Проверка
        REQUIRE(diff_saga == diff_std);
    };
}

TEST_CASE("set_symmetric_difference - subcursor, custom compare")
{
    using Value = long;

    saga_test::property_checker
    << [](std::vector<Value> in1_src, std::vector<Value> in2_src,
          std::vector<Value> const & out_src_old)
    {
        auto const cmp = std::greater<>{};

        std::sort(in1_src.begin(), in1_src.end(), cmp);
        std::sort(in2_src.begin(), in2_src.end(), cmp);

        // Исходные данные
        auto const in1 = saga_test::random_subcursor_of(saga::cursor::all(in1_src));
        auto const in2 = saga_test::random_subcursor_of(saga::cursor::all(in2_src));

        // saga
        std::vector<Value> out_saga(out_src_old);
        auto const out = saga_test::random_subcursor_of(saga::cursor::all(out_saga));

        auto const result = saga::set_symmetric_difference(in1, in2, out, cmp);

        // std
        auto out_std = out_src_old;
        std::set_symmetric_difference(in1.begin(), result.in1.begin()
                                      , in2.begin(), result.in2.begin()
                                      , saga::rebase_cursor(out, out_std).begin(), cmp);

        // Проверка
        REQUIRE(out_saga == out_std);
    };
}

TEST_CASE("set_union : minimal, default compare")
{
    using Value = int;
    using Container = std::vector<Value>;

    saga_test::property_checker << [](Container lhs, Container rhs)
    {
        std::sort(lhs.begin(), lhs.end());
        std::sort(rhs.begin(), rhs.end());

        CAPTURE(lhs, rhs);

        // std
        std::vector<Value> diff_std;

        std::set_union(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()
                       , std::back_inserter(diff_std));

        // saga
        std::vector<Value> diff_saga;

        auto lhs_in = saga_test::make_istringstream_from_range(lhs);
        auto rhs_in = saga_test::make_istringstream_from_range(rhs);

        saga::set_union(saga::make_istream_cursor<Value>(lhs_in)
                        , saga::make_istream_cursor<Value>(rhs_in)
                        , saga::back_inserter(diff_saga));

        // Проверка
        REQUIRE(diff_saga == diff_std);
    };
}

TEST_CASE("set_union : minimal, custom compare")
{
    using Value = int;
    using Container = std::vector<Value>;

    saga_test::property_checker << [](Container lhs, Container rhs)
    {
        auto const cmp = std::greater<>{};

        std::sort(lhs.begin(), lhs.end(), cmp);
        std::sort(rhs.begin(), rhs.end(), cmp);

        // std
        std::vector<Value> diff_std;

        std::set_union(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()
                       , std::back_inserter(diff_std), cmp);

        // saga
        std::vector<Value> diff_saga;

        auto lhs_in = saga_test::make_istringstream_from_range(lhs);
        auto rhs_in = saga_test::make_istringstream_from_range(rhs);

        saga::set_union(saga::make_istream_cursor<Value>(lhs_in)
                        , saga::make_istream_cursor<Value>(rhs_in)
                        , saga::back_inserter(diff_saga), cmp);

        // Проверка
        REQUIRE(diff_saga == diff_std);
    };
}

TEST_CASE("set_union - subcursor, custom compare")
{
    using Value = long;

    saga_test::property_checker
    << [](std::vector<Value> in1_src, std::vector<Value> in2_src,
          std::vector<Value> const & out_src_old)
    {
        auto const cmp = std::greater<>{};

        std::sort(in1_src.begin(), in1_src.end(), cmp);
        std::sort(in2_src.begin(), in2_src.end(), cmp);

        // Исходные данные
        auto const in1 = saga_test::random_subcursor_of(saga::cursor::all(in1_src));
        auto const in2 = saga_test::random_subcursor_of(saga::cursor::all(in2_src));

        // saga
        std::vector<Value> out_saga(out_src_old);
        auto const out = saga_test::random_subcursor_of(saga::cursor::all(out_saga));

        auto const result = saga::set_union(in1, in2, out, cmp);

        // std
        auto out_std = out_src_old;
        std::set_union(in1.begin(), result.in1.begin(), in2.begin(), result.in2.begin()
                       , saga::rebase_cursor(out, out_std).begin(), cmp);

        // Проверка
        REQUIRE(out_std == out_saga);
    };
}

TEST_CASE("is_heap_until - default compare")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const result_std = std::is_heap_until(input.begin(), input.end());
        auto const result_saga = saga::is_heap_until(input);

        REQUIRE(result_saga.begin() == result_std);
        REQUIRE(result_saga.end() == input.end());

        REQUIRE(result_saga.dropped_front().begin() == src.begin());
        REQUIRE(result_saga.dropped_front().end() == result_saga.begin());
    };
}

TEST_CASE("is_heap_until - custom compare")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto const cmp = std::greater<>{};

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const result_std = std::is_heap_until(input.begin(), input.end(), cmp);
        auto const result_saga = saga::is_heap_until(input, cmp);

        REQUIRE(result_saga.begin() == result_std);
        REQUIRE(result_saga.end() == input.end());

        REQUIRE(result_saga.dropped_front().begin() == src.begin());
        REQUIRE(result_saga.dropped_front().end() == result_saga.begin());
    };
}

TEST_CASE("is_heap_until - default compare, guaranty")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> src)
    {
        std::make_heap(src.begin(), src.end());

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const result_std = std::is_heap_until(input.begin(), input.end());
        auto const result_saga = saga::is_heap_until(input);

        REQUIRE(result_saga.begin() == result_std);
        REQUIRE(result_saga.end() == input.end());

        REQUIRE(result_saga.dropped_front().begin() == src.begin());
        REQUIRE(result_saga.dropped_front().end() == result_saga.begin());
    };
}

TEST_CASE("is_heap_until - custom compare, guaranty")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> src)
    {
        auto const cmp = std::greater<>{};

        std::make_heap(src.begin(), src.end(), cmp);

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const result_std = std::is_heap_until(input.begin(), input.end(), cmp);
        auto const result_saga = saga::is_heap_until(input, cmp);

        REQUIRE(result_saga.begin() == result_std);
        REQUIRE(result_saga.end() == input.end());

        REQUIRE(result_saga.dropped_front().begin() == src.begin());
        REQUIRE(result_saga.dropped_front().end() == result_saga.begin());
    };
}

TEST_CASE("is_heap - default compare")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const result_std = std::is_heap(input.begin(), input.end());
        auto const result_saga = saga::is_heap(input);

        REQUIRE(result_saga == result_std);
    };
}

TEST_CASE("is_heap - custom compare")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src)
    {
        auto const cmp = std::greater<>{};

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const result_std = std::is_heap(input.begin(), input.end(), cmp);
        auto const result_saga = saga::is_heap(input, cmp);

        REQUIRE(result_saga == result_std);
    };
}

TEST_CASE("is_heap - default compare, guaranty")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> src)
    {
        std::make_heap(src.begin(), src.end());

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const result_std = std::is_heap(input.begin(), input.end());
        auto const result_saga = saga::is_heap(input);

        REQUIRE(result_saga == result_std);
    };
}

TEST_CASE("is_heap - custom compare, guaranty")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> src)
    {
        auto const cmp = std::greater<>{};

        std::make_heap(src.begin(), src.end(), cmp);

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const result_std = std::is_heap(input.begin(), input.end(), cmp);
        auto const result_saga = saga::is_heap(input, cmp);

        REQUIRE(result_saga == result_std);
    };
}

TEST_CASE("push_heap - default compare, subcursor")
{
    using Value = long;

    saga_test::property_checker << [](std::vector<Value> src, Value const & value)
    {
        src.push_back(value);

        auto const input = saga::cursor::all(src);

        for(auto cur = saga::cursor::all(input); !!cur;)
        {
            auto const src_old = src;

            REQUIRE(saga::is_heap(cur.dropped_front()));

            ++ cur;

            saga::push_heap(cur.dropped_front());

            auto const input_src_old = saga::rebase_cursor(input, src_old);
            auto const cur_src_old = saga::rebase_cursor(cur, src_old);

            CHECK(saga::equal(input.dropped_front(), input_src_old.dropped_front()));
            CHECK(saga::equal(cur, cur_src_old));
            CHECK(saga::equal(cur.dropped_back(), cur_src_old.dropped_back()));

            REQUIRE(saga::is_heap(cur.dropped_front()));

            CHECK(saga::is_permutation(cur.dropped_front(), cur_src_old.dropped_front()));
        }
    };
}

TEST_CASE("push_heap - custom compare, subcursor")
{
    using Value = long;

    saga_test::property_checker << [](std::vector<Value> src, Value const & value)
    {
        src.push_back(value);

        auto const cmp = std::greater<>{};

        auto const input = saga::cursor::all(src);

        for(auto cur = saga::cursor::all(input); !!cur;)
        {
            auto const src_old = src;

            REQUIRE(saga::is_heap(cur.dropped_front(), cmp));

            ++ cur;

            saga::push_heap(cur.dropped_front(), cmp);

            auto const input_src_old = saga::rebase_cursor(input, src_old);
            auto const cur_src_old = saga::rebase_cursor(cur, src_old);

            CHECK(saga::equal(input.dropped_front(), input_src_old.dropped_front()));
            CHECK(saga::equal(cur, cur_src_old));
            CHECK(saga::equal(cur.dropped_back(), cur_src_old.dropped_back()));

            REQUIRE(saga::is_heap(cur.dropped_front(), cmp));

            CHECK(saga::is_permutation(cur.dropped_front(), cur_src_old.dropped_front()));
        }
    };
}

TEST_CASE("make_heap - default compare")
{
    using Value = long;

    saga_test::property_checker << [](std::vector<Value> const & src_old)
    {
        auto src = src_old;
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        saga::make_heap(input);

        CAPTURE(src_old, src);

        auto const input_src_old = saga::rebase_cursor(input, src_old);

        CHECK(saga::equal(input.dropped_front(), input_src_old.dropped_front()));
        CHECK(saga::is_permutation(input, input_src_old));
        CHECK(saga::equal(input.dropped_back(), input_src_old.dropped_back()));

        CHECK(saga::is_heap(input));
    };
}

TEST_CASE("make_heap - custom compare")
{
    using Value = long;

    saga_test::property_checker << [](std::vector<Value> const & src_old)
    {
        auto const cmp = std::greater<>{};

        auto src = src_old;
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        saga::make_heap(input, cmp);

        CAPTURE(src_old, src);

        auto const input_src_old = saga::rebase_cursor(input, src_old);

        CHECK(saga::equal(input.dropped_front(), input_src_old.dropped_front()));
        CHECK(saga::is_permutation(input, input_src_old));
        CHECK(saga::equal(input.dropped_back(), input_src_old.dropped_back()));

        CHECK(saga::is_heap(input, cmp));
    };
}

TEST_CASE("pop_heap - default compare")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> src, Value value)
    {
        src.push_back(std::move(value));

        auto input = saga_test::random_subcursor_of(saga::cursor::all(src));

        saga::make_heap(input);
        REQUIRE(saga::is_heap(input));

        for(;!!input;)
        {
            auto const src_old = src;

            saga::pop_heap(input);

            CAPTURE(src_old, src);

            auto const input_src_old = saga::rebase_cursor(input, src_old);

            CHECK(saga::equal(input.dropped_front(), input_src_old.dropped_front()));
            CHECK(saga::is_permutation(input, input_src_old));
            CHECK(saga::equal(input.dropped_back(), input_src_old.dropped_back()));

            REQUIRE(input.back() == input_src_old.front());

            input.drop_back();

            REQUIRE(saga::is_heap(input));
        }
    };
}

TEST_CASE("pop_heap - custom compare")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> src, Value value)
    {
        src.push_back(std::move(value));

        auto const cmp = std::greater<>{};

        auto input = saga_test::random_subcursor_of(saga::cursor::all(src));

        saga::make_heap(input, cmp);
        REQUIRE(saga::is_heap(input, cmp));

        for(;!!input;)
        {
            auto const src_old = src;

            saga::pop_heap(input, cmp);

            CAPTURE(src_old, src);

            auto const input_src_old = saga::rebase_cursor(input, src_old);

            CHECK(saga::equal(input.dropped_front(), input_src_old.dropped_front()));
            CHECK(saga::is_permutation(input, input_src_old));
            CHECK(saga::equal(input.dropped_back(), input_src_old.dropped_back()));

            REQUIRE(input.back() == input_src_old.front());

            input.drop_back();

            REQUIRE(saga::is_heap(input, cmp));
        }
    };
}

TEST_CASE("sort_heap - default compare")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src_old)
    {
        // Подготовка
        auto src = src_old;
        auto input = saga_test::random_subcursor_of(saga::cursor::all(src));

        saga::make_heap(input);

        // Алгоритм
        saga::sort_heap(input);

        // Проверки
        REQUIRE(saga::is_sorted(input));

        auto const cur_src_old = saga::rebase_cursor(input, src_old);

        REQUIRE(std::is_permutation(input.begin(), input.end(), cur_src_old.begin()));

        REQUIRE(saga::equal(input.dropped_front(), cur_src_old.dropped_front()));
        REQUIRE(saga::equal(input.dropped_back(), cur_src_old.dropped_back()));
    };
}

TEST_CASE("sort_heap - custom compare")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src_old)
    {
        auto const cmp = std::greater<>{};

        // Подготовка
        auto src = src_old;
        auto input = saga_test::random_subcursor_of(saga::cursor::all(src));

        saga::make_heap(input, cmp);

        // Алгоритм
        saga::sort_heap(input, cmp);

        // Проверки
        REQUIRE(saga::is_sorted(input, cmp));

        auto const cur_src_old = saga::rebase_cursor(input, src_old);

        REQUIRE(std::is_permutation(input.begin(), input.end(), cur_src_old.begin()));

        REQUIRE(saga::equal(input.dropped_front(), cur_src_old.dropped_front()));
        REQUIRE(saga::equal(input.dropped_back(), cur_src_old.dropped_back()));
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

TEST_CASE("partial_sort_copy : minimalistic, default compare")
{
    using Value = int;

    saga_test::property_checker
    << [](std::vector<Value> const & src, std::vector<Value> const & dest_old)
    {
        // saga
        auto dest_saga = dest_old;

        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto src_saga = saga_test::make_istringstream_from_range(src);

        auto const r_saga = saga::partial_sort_copy(saga::make_istream_cursor<Value>(src_saga)
                                                    , out_saga);

        // std
        auto dest_std = dest_old;

        auto const out_std = saga::rebase_cursor(out_saga, dest_std);

        auto const r_std = std::partial_sort_copy(src.begin(), src.end()
                                                  , out_std.begin(), out_std.end());

        // Проверка
        CAPTURE(src, dest_old, out_saga);

        REQUIRE(dest_saga == dest_std);

        if(!!out_saga)
        {
            REQUIRE(!r_saga.in);
        }

        REQUIRE(r_saga.out.size() == (out_std.end() - r_std));
        REQUIRE(r_saga.out.end() == out_saga.end());
        REQUIRE(r_saga.out.dropped_front().begin() == dest_saga.begin());
        REQUIRE(r_saga.out.dropped_back().end() == dest_saga.end());
    };
}

TEST_CASE("partial_sort_copy : minimalistic, custom compare")
{
    using Value = int;

    saga_test::property_checker
    << [](std::vector<Value> const & src, std::vector<Value> const & dest_old)
    {
        auto const cmp = std::greater<>{};

        // saga
        auto dest_saga = dest_old;

        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto src_saga = saga_test::make_istringstream_from_range(src);

        auto const r_saga = saga::partial_sort_copy(saga::make_istream_cursor<Value>(src_saga)
                                                    , out_saga, cmp);

        // std
        auto dest_std = dest_old;

        auto const out_std = saga::rebase_cursor(out_saga, dest_std);

        auto const r_std = std::partial_sort_copy(src.begin(), src.end()
                                                  , out_std.begin(), out_std.end(), cmp);

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
    };
}

TEST_CASE("partial_sort_copy : input subrange, default compare")
{
    using Value = int;

    saga_test::property_checker
    << [](std::vector<Value> const & src, std::vector<Value> const & dest_old)
    {
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto dest_saga = dest_old;

        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto const r_saga = saga::partial_sort_copy(input, out_saga);

        // std
        auto dest_std = dest_old;

        auto const out_std = saga::rebase_cursor(out_saga, dest_std);

        auto const r_std = std::partial_sort_copy(input.begin(), input.end()
                                                  , out_std.begin(), out_std.end());

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
    };
}

TEST_CASE("partial_sort_copy : input subrage, custom compare")
{
    using Value = int;

    saga_test::property_checker
    << [](std::vector<Value> const & src, std::vector<Value> const & dest_old)
    {
        auto const cmp = std::greater<>{};

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // saga
        auto dest_saga = dest_old;

        auto const out_saga = saga_test::random_subcursor_of(saga::cursor::all(dest_saga));

        auto const r_saga = saga::partial_sort_copy(input, out_saga, cmp);

        // std
        auto dest_std = dest_old;

        auto const out_std = saga::rebase_cursor(out_saga, dest_std);

        auto const r_std = std::partial_sort_copy(input.begin(), input.end()
                                                  , out_std.begin(), out_std.end(), cmp);

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
    };
}

TEST_CASE("lower_bound: default compare")
{
    using Value = int;

    saga_test::property_checker << [](std::forward_list<Value> const & src_old, Value const & value)
    {
        // Подготовка
        auto const src = [&]()
        {
            auto src = src_old;
            src.sort();
            return src;
        }();

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // Выполнение
        auto const r_std = std::lower_bound(input.begin(), input.end(), value);

        auto const r_saga = saga::lower_bound(input, value);

        // Проверка
        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(r_saga.end() == input.end());

        REQUIRE(r_saga.dropped_front().begin() == src.begin());
        REQUIRE(r_saga.dropped_back().end() == src.end());
    };
}

TEST_CASE("lower_bound: custom compare")
{
    using Value = int;

    saga_test::property_checker << [](std::forward_list<Value> const & src_old, Value const & value)
    {
        // Подготовка
        auto cmp = std::greater<>{};

        auto const src = [&]()
        {
            auto src = src_old;
            src.sort(cmp);
            return src;
        }();

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // Выполнение
        auto const r_std = std::lower_bound(input.begin(), input.end(), value, cmp);

        auto const r_saga = saga::lower_bound(input, value, cmp);

        // Проверка
        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(r_saga.end() == input.end());

        REQUIRE(r_saga.dropped_front().begin() == src.begin());
        REQUIRE(r_saga.dropped_back().end() == src.end());
    };
}

TEST_CASE("upper_bound: default compare")
{
    using Value = int;

    saga_test::property_checker << [](std::forward_list<Value> const & src_old, Value const & value)
    {
        // Подготовка
        auto const src = [&]()
        {
            auto src = src_old;
            src.sort();
            return src;
        }();

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // Выполнение
        auto const r_std = std::upper_bound(input.begin(), input.end(), value);

        auto const r_saga = saga::upper_bound(input, value);

        // Проверка
        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(r_saga.end() == input.end());

        REQUIRE(r_saga.dropped_front().begin() == src.begin());
        REQUIRE(r_saga.dropped_back().end() == src.end());
    };
}

TEST_CASE("upper_bound: custom compare")
{
    using Value = int;

    saga_test::property_checker << [](std::forward_list<Value> const & src_old, Value const & value)
    {
        // Подготовка
        auto cmp = std::greater<>{};

        auto const src = [&]()
        {
            auto src = src_old;
            src.sort(cmp);
            return src;
        }();

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // Выполнение
        auto const r_std = std::upper_bound(input.begin(), input.end(), value, cmp);

        auto const r_saga = saga::upper_bound(input, value, cmp);

        // Проверка
        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(r_saga.end() == input.end());

        REQUIRE(r_saga.dropped_front().begin() == src.begin());
        REQUIRE(r_saga.dropped_back().end() == src.end());
    };
}

// equal_range
TEST_CASE("equal_range: defaul compare")
{
    using Value = long;

    saga_test::property_checker << [](std::forward_list<Value> const & src_old, Value const & value)
    {
        // Подготовка
        auto const src = [&]()
        {
            auto src = src_old;
            src.sort();
            return src;
        }();

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // std
        auto const r_std = std::equal_range(input.begin(), input.end(), value);

        // saga
        auto const r_saga = saga::equal_range(input, value);

        // Проверки
        CAPTURE(src, input);

        CHECK(r_saga.begin() == r_std.first);
        CHECK(r_saga.end() == r_std.second);

        CHECK(r_saga.dropped_front().begin() == input.begin());
        CHECK(r_saga.dropped_back().end() == input.end());
    };
}

TEST_CASE("equal_range: custom compare")
{
    using Value = long;

    saga_test::property_checker << [](std::forward_list<Value> const & src_old, Value const & value)
    {
        // Подготовка
        auto const cmp = std::greater<>{};

        auto const src = [&]()
        {
            auto src = src_old;
            src.sort(cmp);
            return src;
        }();

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // std
        auto const r_std = std::equal_range(input.begin(), input.end(), value, cmp);

        // saga
        auto const r_saga = saga::equal_range(input, value, cmp);

        // Проверки
        REQUIRE(r_saga.begin() == r_std.first);
        REQUIRE(r_saga.end() == r_std.second);

        REQUIRE(r_saga.dropped_front().begin() == input.begin());
        REQUIRE(r_saga.dropped_back().end() == input.end());
    };
}

TEST_CASE("binary_search: default compare")
{
    using Value = int;

    saga_test::property_checker << [](std::forward_list<Value> const & src_old, Value const & value)
    {
        // Подготовка
        auto const src = [&]()
        {
            auto src = src_old;
            src.sort();
            return src;
        }();

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // Выполнение
        auto const r_std = std::binary_search(input.begin(), input.end(), value);

        auto const r_saga = saga::binary_search(input, value);

        // Проверка
        REQUIRE(r_saga == r_std);
    };
}

TEST_CASE("binary_search: custom compare")
{
    using Value = int;

    saga_test::property_checker << [](std::forward_list<Value> const & src_old, Value const & value)
    {
        // Подготовка
        auto cmp = std::greater<>{};

        auto const src = [&]()
        {
            auto src = src_old;
            src.sort(cmp);
            return src;
        }();

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        // Выполнение
        auto const r_std = std::binary_search(input.begin(), input.end(), value, cmp);

        auto const r_saga = saga::binary_search(input, value, cmp);

        // Проверка
        REQUIRE(r_saga == r_std);
    };
}

TEST_CASE("min_element: default compare")
{
    using Value = int;

    saga_test::property_checker << [](std::forward_list<Value> const & src)
    {
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const r_std = std::min_element(input.begin(), input.end());
        auto const r_saga = saga::min_element(input);

        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(r_saga.end() == input.end());

        REQUIRE(r_saga.dropped_front().begin() == src.begin());
        REQUIRE(r_saga.dropped_front().end() == r_std);

        REQUIRE(r_saga.dropped_back().begin() == input.end());
        REQUIRE(r_saga.dropped_back().end() == src.end());
    };
}

TEST_CASE("min_element: custom compare")
{
    using Value = int;

    saga_test::property_checker << [](std::forward_list<Value> const & src)
    {
        auto const cmp = std::greater<>{};

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const r_std = std::min_element(input.begin(), input.end(), cmp);
        auto const r_saga = saga::min_element(input, cmp);

        REQUIRE(r_saga.begin() == r_std);
        REQUIRE(r_saga.end() == input.end());

        REQUIRE(r_saga.dropped_front().begin() == src.begin());
        REQUIRE(r_saga.dropped_front().end() == r_std);

        REQUIRE(r_saga.dropped_back().begin() == input.end());
        REQUIRE(r_saga.dropped_back().end() == src.end());
    };
}

TEST_CASE("max_element: default compare")
{
    using Value = int;

    saga_test::property_checker << [](std::forward_list<Value> const & src)
    {
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const r_saga = saga::max_element(input);
        auto const r_std = std::minmax_element(input.begin(), input.end());

        // Проверка
        REQUIRE(r_saga.begin() == r_std.second);
        REQUIRE(r_saga.end() == input.end());
        REQUIRE(r_saga.dropped_front().begin() == src.begin());
        REQUIRE(r_saga.dropped_back().end() == src.end());
    };
}

TEST_CASE("max_element: custom compare")
{
    using Value = int;

    saga_test::property_checker << [](std::forward_list<Value> const & src)
    {
        auto const cmp = [](Value const & lhs, Value const & rhs)
            { return lhs % 5 < rhs % 5; };

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const r_saga = saga::max_element(input, cmp);
        auto const r_std = std::minmax_element(input.begin(), input.end(), cmp);

        // Проверка
        REQUIRE(r_saga.begin() == r_std.second);
        REQUIRE(r_saga.end() == input.end());
        REQUIRE(r_saga.dropped_front().begin() == src.begin());
        REQUIRE(r_saga.dropped_back().end() == src.end());
    };
}

TEST_CASE("minmax_element: default compare")
{
    using Value = int;

    saga_test::property_checker << [](std::forward_list<Value> const & src)
    {
        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const r_saga = saga::minmax_element(input);
        auto const r_std = std::minmax_element(input.begin(), input.end());

        // Проверка
        REQUIRE(r_saga.min.begin() == r_std.first);
        REQUIRE(r_saga.min.end() == input.end());
        REQUIRE(r_saga.min.dropped_front().begin() == src.begin());
        REQUIRE(r_saga.min.dropped_back().end() == src.end());

        REQUIRE(r_saga.max.begin() == r_std.second);
        REQUIRE(r_saga.max.end() == input.end());
        REQUIRE(r_saga.max.dropped_front().begin() == src.begin());
        REQUIRE(r_saga.max.dropped_back().end() == src.end());
    };
}

TEST_CASE("minmax_element: custom compare")
{
    using Value = int;

    saga_test::property_checker << [](std::forward_list<Value> const & src)
    {
        auto const cmp = [](Value const & lhs, Value const & rhs)
            { return lhs % 5 < rhs % 5; };

        auto const input = saga_test::random_subcursor_of(saga::cursor::all(src));

        auto const r_saga = saga::minmax_element(input, cmp);
        auto const r_std = std::minmax_element(input.begin(), input.end(), cmp);

        // Проверка
        REQUIRE(r_saga.min.begin() == r_std.first);
        REQUIRE(r_saga.min.end() == input.end());
        REQUIRE(r_saga.min.dropped_front().begin() == src.begin());
        REQUIRE(r_saga.min.dropped_back().end() == src.end());

        REQUIRE(r_saga.max.begin() == r_std.second);
        REQUIRE(r_saga.max.end() == input.end());
        REQUIRE(r_saga.max.dropped_front().begin() == src.begin());
        REQUIRE(r_saga.max.dropped_back().end() == src.end());
    };
}

TEST_CASE("is_permutation: default predicate")
{
    using Value1 = int;
    using Value2 = long;

    using Container1 = std::forward_list<Value1>;
    using Container2 = std::forward_list<Value2>;

    saga_test::property_checker << [](Container1 const & src1, Container2 const & src2)
    {
        auto const cur1 = saga_test::random_subcursor_of(saga::cursor::all(src1));
        auto const cur2 = saga_test::random_subcursor_of(saga::cursor::all(src2));

        CAPTURE(src1, src2, Container1(cur1.begin(), cur1.end())
                , Container2(cur2.begin(), cur2.end()));

        REQUIRE(saga::is_permutation(cur1, cur2)
                == std::is_permutation(cur1.begin(), cur1.end(), cur2.begin(), cur2.end()));
    };
}

TEST_CASE("is_permutation: default predicate, always true")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src1)
    {
        auto const cur1 = saga_test::random_subcursor_of(saga::cursor::all(src1));

        auto src2 = src1;
        auto const cur2 = saga::rebase_cursor(cur1, src2);

        std::shuffle(cur2.begin(), cur2.end(), saga_test::random_engine());

        REQUIRE(saga::is_permutation(cur1, cur2));
    };
}

TEST_CASE("is_permutation: custom predicate")
{
    using Value1 = int;
    using Value2 = long;

    using Container1 = std::forward_list<Value1>;
    using Container2 = std::forward_list<Value2>;

    saga_test::property_checker << [](Container1 const & src1, Container2 const & src2)
    {
        auto const bin_pred = [](Value2 const & lhs, Value2 const & rhs)
        {
            return lhs % 10 == rhs % 10;
        };

        auto const cur1 = saga_test::random_subcursor_of(saga::cursor::all(src1));
        auto const cur2 = saga_test::random_subcursor_of(saga::cursor::all(src2));

        CAPTURE(src1, src2, Container1(cur1.begin(), cur1.end())
                , Container2(cur2.begin(), cur2.end()));

        REQUIRE(saga::is_permutation(cur1, cur2, bin_pred)
                == std::is_permutation(cur1.begin(), cur1.end()
                                       , cur2.begin(), cur2.end(), bin_pred));
    };
}

TEST_CASE("is_permutation: custom predicate, always true")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> const & src1)
    {
        auto const bin_pred = [](Value const & lhs, Value const & rhs)
        {
            return lhs % 10 == rhs % 10;
        };

        auto const cur1 = saga_test::random_subcursor_of(saga::cursor::all(src1));

        auto src2 = src1;
        auto const cur2 = saga::rebase_cursor(cur1, src2);

        std::shuffle(cur2.begin(), cur2.end(), saga_test::random_engine());

        REQUIRE(saga::is_permutation(cur1, cur2, bin_pred));
    };
}

TEST_CASE("lexicographical_compare - minimal, default compare")
{
    using Value1 = int;
    using Value2 = long;

    saga_test::property_checker
    << [](std::vector<Value1> const & src1, std::vector<Value2> const & src2)
    {
        auto src1_in = saga_test::make_istringstream_from_range(src1);
        auto src2_in = saga_test::make_istringstream_from_range(src2);

        REQUIRE(saga::lexicographical_compare(saga::make_istream_cursor<Value1>(src1_in)
                                              , saga::make_istream_cursor<Value2>(src2_in))
                == std::lexicographical_compare(src1.begin(), src1.end()
                                                , src2.begin(), src2.end()));

        REQUIRE(!saga::lexicographical_compare(saga::cursor::all(src1), saga::cursor::all(src1)));
        REQUIRE(!saga::lexicographical_compare(saga::cursor::all(src2), saga::cursor::all(src2)));
    };
}

TEST_CASE("lexicographical_compare - minimal, custom predicate")
{
    using Value1 = int;
    using Value2 = long;

    saga_test::property_checker
    << [](std::vector<Value1> const & src1, std::vector<Value2> const & src2)
    {
        CAPTURE(src1, src2);

        auto const cmp = std::greater<>{};

        auto src1_in = saga_test::make_istringstream_from_range(src1);
        auto src2_in = saga_test::make_istringstream_from_range(src2);

        REQUIRE(saga::lexicographical_compare(saga::make_istream_cursor<Value1>(src1_in)
                                              , saga::make_istream_cursor<Value2>(src2_in)
                                              , cmp)
                == std::lexicographical_compare(src1.begin(), src1.end()
                                                , src2.begin(), src2.end(), cmp));

        REQUIRE(!saga::lexicographical_compare(saga::cursor::all(src1)
                                               , saga::cursor::all(src1), cmp));
        REQUIRE(!saga::lexicographical_compare(saga::cursor::all(src2)
                                               , saga::cursor::all(src2), cmp));
    };
}

TEST_CASE("lexicographical_compare - subcursor, default compare")
{
    using Value1 = int;
    using Value2 = long;

    saga_test::property_checker
    << [](std::vector<Value1> const & src1, std::vector<Value2> const & src2)
    {
        auto in1 = saga_test::random_subcursor_of(saga::cursor::all(src1));
        auto in2 = saga_test::random_subcursor_of(saga::cursor::all(src2));

        REQUIRE(saga::lexicographical_compare(in1, in2)
                == std::lexicographical_compare(in1.begin(), in1.end(), in2.begin(), in2.end()));

        REQUIRE(!saga::lexicographical_compare(in1, in1));
        REQUIRE(!saga::lexicographical_compare(in2, in2));
    };
}

TEST_CASE("lexicographical_compare - subcursor, custom predicate")
{
    using Value1 = int;
    using Value2 = long;

    saga_test::property_checker
    << [](std::vector<Value1> const & src1, std::vector<Value2> const & src2)
    {
        CAPTURE(src1, src2);

        auto const cmp = std::greater<>{};

        auto in1 = saga_test::random_subcursor_of(saga::cursor::all(src1));
        auto in2 = saga_test::random_subcursor_of(saga::cursor::all(src2));

        REQUIRE(saga::lexicographical_compare(in1, in2, cmp)
                == std::lexicographical_compare(in1.begin(), in1.end()
                                                , in2.begin(), in2.end(), cmp));

        REQUIRE(!saga::lexicographical_compare(in1, in1));
        REQUIRE(!saga::lexicographical_compare(in2, in2));
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
        auto sub = std::string(str.begin(), saga_test::random_iterator_of(str));
        for(auto & c : sub)
        {
            c = std::tolower(c);
        }

        auto const cmp = [](char x, char y) { return std::tolower(x) < std::tolower(y); };

        REQUIRE(saga::lexicographical_compare(saga::cursor::all(sub), saga::cursor::all(str), cmp)
                == (sub.size() < str.size()));
    };
}

TEST_CASE("starts_with : prefix")
{
    saga_test::property_checker << [](std::string const & str)
    {
        auto const prefix = str.substr(0, str.size() / 2);

        REQUIRE(::saga::starts_with(str, prefix));
    };
}

TEST_CASE("starts_with : common")
{
    saga_test::property_checker << [](std::string const & str, std::string const & test)
    {
        auto const expected = test.size() <= str.size() && test == str.substr(0, test.size());

        CAPTURE(str, test);

        REQUIRE(::saga::starts_with(str, test) == expected);
    };
}

TEST_CASE("ends_with : suffix")
{
    saga_test::property_checker << [](std::string const & str)
    {
        auto const suffix = str.substr(str.size() / 2);

        REQUIRE(::saga::ends_with(str, suffix));
    };
}

TEST_CASE("ends_with : common")
{
    saga_test::property_checker << [](std::string const & str, std::string const & test)
    {
        auto const expected = test.size() <= str.size()
                            && test == str.substr(str.size() - test.size());

        CAPTURE(str, test);

        REQUIRE(::saga::ends_with(str, test) == expected);
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
