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

TEST_CASE("copy")
{
    using Value = int;
    saga_test::property_checker << [](std::vector<Value> const & src,
                                      std::vector<Value> const & dest_old)
    {
        auto dest = dest_old;

        // Взять подинтервалы контейнеров
        auto const src_cur = saga_test::random_subcursor_of(saga::cursor::all(src));
        auto const dest_cur = saga_test::random_subcursor_of(saga::cursor::all(dest));

        auto const dest_prefix_size = (dest_cur.begin() - dest.begin());

        assert(0 <= dest_prefix_size && static_cast<size_t>(dest_prefix_size) <= dest.size());

        auto const result = saga::copy(src_cur, dest_cur);

        // Проверка содержимого dest
        auto const n = std::min(src_cur.size(), dest_cur.size());

        REQUIRE(std::equal(dest.begin(), dest.begin() + dest_prefix_size,
                           dest_old.begin(), dest_old.begin() + dest_prefix_size));

        REQUIRE(std::equal(dest.begin() + dest_prefix_size, dest.begin() + dest_prefix_size + n,
                           src_cur.begin(), src_cur.begin() + n));

        REQUIRE(std::equal(dest.begin() + dest_prefix_size + n, dest.end(),
                           dest_old.begin() + dest_prefix_size + n, dest_old.end()));

        // Проверяем возвращаемое значение
        REQUIRE((!result.in || !result.out));
        REQUIRE(!result.in == (src_cur.size() <= dest_cur.size()));

        REQUIRE(result.in.begin() == src_cur.begin() + n);
        REQUIRE(result.in.end() == src_cur.end());

        REQUIRE(result.out.begin() == dest_cur.begin() + n);
        REQUIRE(result.out.end() == dest_cur.end());

        // @todo Проверить begin_orig и end_orig (имена предварительные)
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
        std::vector<ValueOut> dest_std;
        std::copy_if(input.begin(), result.in.begin(), std::back_inserter(dest_std), pred);

        // Проверка
        auto const n_front = (out_saga.begin() - dest_saga.begin());
        auto const n_back = (dest_saga.end() - result.out.begin());

        REQUIRE(dest_saga.size() == dest_old.size());
        REQUIRE(dest_saga.size() == n_front + (result.out.begin() - out_saga.begin()) + n_back);
        REQUIRE((result.in.begin() - input.begin()) >= (result.out.begin() - out_saga.begin()));

        REQUIRE(std::equal(dest_saga.begin(), dest_saga.begin() + n_front
                           , dest_old.begin(), dest_old.begin() + n_front));

        REQUIRE(std::equal(out_saga.begin(), result.out.begin(),
                           dest_std.begin(), dest_std.end()));

        REQUIRE(std::equal(dest_saga.end() - n_back, dest_saga.end(),
                           dest_old.end() - n_back, dest_old.end()));
    };
}

// @todo минимальный тест fill, нужно придумать конечный курсор вывода (take_n от back_inserter)
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
        auto const n_front = std::distance(xs_saga.begin(), sub_saga.begin());
        auto const n_back = std::distance(sub_saga.end(), xs_saga.end());

        REQUIRE(std::equal(xs_saga.begin(), std::next(xs_saga.begin(), n_front),
                           src.begin(), src.begin() + n_front));

        for(auto const & x : sub_saga)
        {
            REQUIRE(x == value);
        }

        REQUIRE(std::equal(std::next(xs_saga.begin(), src.size() - n_back), xs_saga.end(),
                           src.end() - n_back, src.end()));
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

// @todo минимальный тест generate, нужно придумать конечный курсор вывода (take_n от back_inserter)
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
        auto const cmp_no_case = [](int x, int y) { return std::abs(x) < std::abs(y); };

        std::sort(in1.begin(), in1.end(), cmp_no_case);
        std::sort(in2.begin(), in2.end(), cmp_no_case);

        CAPTURE(in1, in2);

        auto in1_istream = saga_test::make_istringstream_from_range(in1);
        auto in2_istream = saga_test::make_istringstream_from_range(in2);

        REQUIRE(saga::includes(saga::make_istream_cursor<Value>(in1_istream),
                               saga::make_istream_cursor<Value>(in2_istream), cmp_no_case)
                == std::includes(in1.begin(), in1.end(), in2.begin(), in2.end(), cmp_no_case));
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
        auto const cmp_no_case = [](int x, int y) { return std::abs(x) < std::abs(y); };

        std::sort(in1.begin(), in1.end(), cmp_no_case);
        std::sort(in2.begin(), in2.end(), cmp_no_case);

        CAPTURE(in1, in2);

        REQUIRE(saga::includes(saga::cursor::all(in1), saga::cursor::all(in2), cmp_no_case)
                == std::includes(in1.begin(), in1.end(), in2.begin(), in2.end(), cmp_no_case));
    };
}

TEST_CASE("includes - always true, custom compare")
{
    using Value = int;

    saga_test::property_checker << [](std::vector<Value> in1, std::vector<Value> in2)
    {
        auto const cmp_no_case = [](int x, int y) { return std::abs(x) < std::abs(y); };

        in1.insert(in1.end(), in2.begin(), in2.end());

        std::sort(in1.begin(), in1.end(), cmp_no_case);
        std::sort(in2.begin(), in2.end(), cmp_no_case);

        CAPTURE(in1, in2);

        REQUIRE(std::includes(in1.begin(), in1.end(), in2.begin(), in2.end(), cmp_no_case));
        REQUIRE(saga::includes(saga::cursor::all(in1), saga::cursor::all(in2), cmp_no_case));
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
        std::vector<Value> out_src(out_src_old);
        auto const out = saga_test::random_subcursor_of(saga::cursor::all(out_src));

        auto const result = saga::set_difference(in1, in2, out, cmp);

        // std
        std::vector<Value> out_std;
        std::set_difference(in1.begin(), result.in.begin(), in2.begin(), in2.end()
                            , std::back_inserter(out_std), cmp);

        // Проверка
        auto const n_before = out.begin() - out_src.begin();
        auto const n_after = out_src.end() - out.end();

        REQUIRE(std::equal(out.begin(), result.out.begin(), out_std.begin(), out_std.end()));

        REQUIRE(std::equal(out_src.begin(), out_src.begin() + n_before,
                           out_src_old.begin(), out_src_old.begin() + n_before));

        REQUIRE(std::equal(out_src.end() - n_after, out_src.end(),
                           out_src_old.end() - n_after, out_src_old.end()));
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
        std::vector<Value> out_src(out_src_old);
        auto const out = saga_test::random_subcursor_of(saga::cursor::all(out_src));

        auto const result = saga::set_intersection(in1, in2, out, cmp);

        // std
        std::vector<Value> out_std;
        std::set_intersection(in1.begin(), result.in1.begin(), in2.begin(), result.in2.begin()
                              , std::back_inserter(out_std), cmp);

        // Проверка
        auto const n_before = out.begin() - out_src.begin();
        auto const n_after = out_src.end() - out.end();

        REQUIRE(std::equal(out.begin(), result.out.begin(), out_std.begin(), out_std.end()));

        REQUIRE(std::equal(out_src.begin(), out_src.begin() + n_before,
                           out_src_old.begin(), out_src_old.begin() + n_before));

        REQUIRE(std::equal(out_src.end() - n_after, out_src.end(),
                           out_src_old.end() - n_after, out_src_old.end()));
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
        std::vector<Value> out_src(out_src_old);
        auto const out = saga_test::random_subcursor_of(saga::cursor::all(out_src));

        auto const result = saga::set_symmetric_difference(in1, in2, out, cmp);

        // std
        std::vector<Value> out_std;
        std::set_symmetric_difference(in1.begin(), result.in1.begin()
                                      , in2.begin(), result.in2.begin()
                                      , std::back_inserter(out_std), cmp);

        // Проверка
        auto const n_before = out.begin() - out_src.begin();
        auto const n_after = out_src.end() - out.end();

        REQUIRE(std::equal(out.begin(), result.out.begin(), out_std.begin(), out_std.end()));

        REQUIRE(std::equal(out_src.begin(), out_src.begin() + n_before,
                           out_src_old.begin(), out_src_old.begin() + n_before));

        REQUIRE(std::equal(out_src.end() - n_after, out_src.end(),
                           out_src_old.end() - n_after, out_src_old.end()));
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
        std::vector<Value> out_src(out_src_old);
        auto const out = saga_test::random_subcursor_of(saga::cursor::all(out_src));

        auto const result = saga::set_union(in1, in2, out, cmp);

        // std
        std::vector<Value> out_std;
        std::set_union(in1.begin(), result.in1.begin(), in2.begin(), result.in2.begin()
                       , std::back_inserter(out_std), cmp);

        // Проверка
        auto const n_before = out.begin() - out_src.begin();
        auto const n_after = out_src.end() - out.end();

        REQUIRE(std::equal(out.begin(), result.out.begin(), out_std.begin(), out_std.end()));

        REQUIRE(std::equal(out_src.begin(), out_src.begin() + n_before,
                           out_src_old.begin(), out_src_old.begin() + n_before));

        REQUIRE(std::equal(out_src.end() - n_after, out_src.end(),
                           out_src_old.end() - n_after, out_src_old.end()));
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
