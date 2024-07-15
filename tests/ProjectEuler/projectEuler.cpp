/* (c) 2022-2024 Галушин Павел Викторович, galushin@gmail.com

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

// Инфраструктура тестирования
#include "../saga_test.hpp"
#include <catch2/catch_amalgamated.hpp>

// Используемые возможности
#include <saga/actions/reverse.hpp>
#include <saga/actions/sort.hpp>
#include <saga/container/make.hpp>
#include <saga/cursor/by_line.hpp>
#include <saga/cursor/cartesian_product.hpp>
#include <saga/cursor/enumerate.hpp>
#include <saga/cursor/indices.hpp>
#include <saga/cursor/iota.hpp>
#include <saga/cursor/filter.hpp>
#include <saga/cursor/set_union.hpp>
#include <saga/cursor/stride.hpp>
#include <saga/cursor/to.hpp>
#include <saga/cursor/transform.hpp>
#include <saga/flat_set.hpp>
#include <saga/math.hpp>
#include <saga/numeric.hpp>
#include <saga/numeric/is_prime.hpp>
#include <saga/numeric/primes_cursor.hpp>
#include <saga/pipes/filter.hpp>
#include <saga/pipes/for_each.hpp>
#include <saga/pipes/transform.hpp>
#include <saga/utility/exchange.hpp>
#include <saga/utility/functional_macro.hpp>

#include <optional>
#include <vector>

// Тесты
// PE 001 : сумма кратных 3 или 5
namespace
{
    template <class IntType>
    constexpr IntType projectEuler_001_range_for_loop_indices(IntType n_max)
    {
        auto result = IntType(0);

        for(auto num : saga::cursor::indices(n_max))
        {
            if(saga::is_divisible_by(num, 3) || saga::is_divisible_by(num, 5))
            {
                result += num;
            }
        }

        return result;
    }

    template <class IntType>
    constexpr IntType sum_of_multiples_below(IntType d, IntType n_max)
    {
        return saga::triangular_number((n_max - 1) / d) * d;
    }

    template <class IntType>
    constexpr IntType projectEuler_001_closed_form(IntType n_max)
    {
        return ::sum_of_multiples_below(3, n_max)
               + ::sum_of_multiples_below(5, n_max)
               - ::sum_of_multiples_below(15, n_max);
    }

    template <class IntType>
    IntType projectEuler_001_cursor_algorithms(IntType n_max)
    {
        std::vector<IntType> tmp;
        saga::copy_if(saga::cursor::indices(n_max)
                      , saga::back_inserter(tmp)
                      , [](IntType const & num) { return num % 3 == 0 || num % 5 == 0; });

        return saga::reduce(saga::cursor::all(tmp));
    }

    template <class IntType>
    constexpr IntType projectEuler_001_cursor_algorithms_filter(IntType n_max)
    {
        auto pred = [](IntType const & num) { return num % 3 == 0 || num % 5 == 0; };

        return saga::reduce(saga::cursor::indices(n_max) | saga::cursor::filter(pred));
    }

    template <class IntType>
    constexpr IntType projectEuler_001_smart_output_cursor(IntType n_max)
    {
        auto pred = [](IntType const & num) { return num % 3 == 0 || num % 5 == 0; };

        auto result = IntType{0};
        auto accumulator = [&result](IntType const & arg) { result += arg; };

        saga::copy(saga::cursor::indices(n_max)
                   , saga::pipes::filter(saga::pipes::for_each(accumulator), pred));

        return result;
    }

    template <class IntType>
    constexpr IntType projectEuler_001_cursor_algorithms_set_union(IntType n_max)
    {
        auto in_3 = saga::cursor::iota(IntType(3)) | saga::cursor::stride(IntType(3));
        auto in_5 = saga::cursor::iota(IntType(5)) | saga::cursor::stride(IntType(5));

        auto input = saga::cursor::set_union(std::move(in_3), std::move(in_5))
                   | saga::cursor::take_while([=](IntType num) { return num < n_max; });

        return saga::accumulate(std::move(input), IntType(0));
    }
}

static_assert(projectEuler_001_range_for_loop_indices(10) == 23, "");
static_assert(projectEuler_001_range_for_loop_indices(1000) == 233168, "");

static_assert(projectEuler_001_closed_form(10) == 23, "");
static_assert(projectEuler_001_closed_form(1000) == 233168, "");

static_assert(projectEuler_001_cursor_algorithms_filter(10) == 23, "");
static_assert(projectEuler_001_cursor_algorithms_filter(1000) == 233168, "");

static_assert(projectEuler_001_smart_output_cursor(10) == 23, "");
static_assert(projectEuler_001_smart_output_cursor(1000) == 233168, "");

static_assert(projectEuler_001_cursor_algorithms_set_union(10) == 23, "");
static_assert(projectEuler_001_cursor_algorithms_set_union(1000) == 233168, "");

TEST_CASE("ProjectEuler 001")
{
    CHECK(projectEuler_001_cursor_algorithms(10) == 23);
    CHECK(projectEuler_001_cursor_algorithms(1000) == 233168);

    for(auto const & num : saga::cursor::indices(1, 1000))
    {
        CHECK(projectEuler_001_cursor_algorithms(num)
              == projectEuler_001_closed_form(num));
    }
}

// PE 002 : Чётные числа Фибоначчи
namespace
{
    template <class IntType>
    class fibonacci_sequence
     : saga::cursor_facade<fibonacci_sequence<IntType>, IntType const &>
    {
    public:
        // Типы
        using value_type = IntType;
        using reference = value_type const &;
        using difference_type = std::ptrdiff_t;
        using cursor_category = std::input_iterator_tag;

        // Создание, копирование, уничтожение
        constexpr fibonacci_sequence(IntType num1, IntType num2)
         : cur_(std::move(num1))
         , next_(std::move(num2))
        {}

        // Однопроходная последовательность
        constexpr bool operator!() const
        {
            return false;
        }

        constexpr reference front() const
        {
            return this->cur_;
        }

        constexpr void drop_front()
        {
            this->cur_ = saga::exchange(this->next_
                                       , std::move(this->cur_) + std::move(this->next_));
        }

    private:
        IntType cur_;
        IntType next_;
    };

    template <class IntType>
    fibonacci_sequence<IntType>
    make_fibonacci_sequence(IntType num1, IntType num2)
    {
        return fibonacci_sequence<IntType>(std::move(num1), std::move(num2));
    }

    template <class IntType>
    constexpr IntType projectEuler_002_take_while_after_filter_input(IntType n_max)
    {
        auto input = ::fibonacci_sequence<IntType>(1, 2)
                   | saga::cursor::filter(saga::is_even)
                   | saga::cursor::take_while([&](IntType const & arg) { return arg <= n_max; });

        return saga::reduce(std::move(input), IntType{0});
    }

    template <class IntType>
    constexpr IntType projectEuler_002_filter_after_take_while_input(IntType n_max)
    {
        auto input = ::fibonacci_sequence<IntType>(1, 2)
                   | saga::cursor::take_while([=](IntType const & arg) { return arg <= n_max; })
                   | saga::cursor::filter(saga::is_even);

        return saga::reduce(std::move(input), IntType{0});
    }

    template <class IntType>
    constexpr IntType projectEuler_002_filter_output(IntType n_max)
    {
        auto input = ::fibonacci_sequence<IntType>(1, 2)
                   | saga::cursor::take_while([&](IntType const & arg) { return arg <= n_max; });

        auto result = IntType{0};
        auto accumulator = [&result](IntType const & arg) { result += arg; };
        auto output = saga::pipes::filter(saga::pipes::for_each(accumulator), saga::is_even);

        saga::copy(std::move(input), std::move(output));

        return result;
    }
}

static_assert(projectEuler_002_take_while_after_filter_input(4'000'000) == 4'613'732, "");
static_assert(projectEuler_002_filter_after_take_while_input(4'000'000) == 4'613'732, "");
static_assert(projectEuler_002_filter_output(4'000'000) == 4'613'732, "");

// PE 003: Наибольший простой делитель
namespace
{
    template <class IntType>
    constexpr void PE_003_step(IntType & num, IntType divisor, IntType & max_divisor)
    {
        auto result = saga::remove_factor(num, divisor);

        num = result.value;

        if(result.multiplicity > 0)
        {
            max_divisor = divisor;
        }
    }

    template <class IntType>
    constexpr
    IntType projectEuler_003(IntType num)
    {
        assert(num > 0);

        auto max_divisor = IntType{1};

        PE_003_step(num, IntType(2), max_divisor);
        PE_003_step(num, IntType(3), max_divisor);

        auto divisors = saga::cursor::iota(IntType(5))
                      | saga::cursor::stride(6)
                      | saga::cursor::take_while([&](auto const & arg) { return arg*arg < num; });

        saga::for_each(std::move(divisors), [&](auto const & divisor)
        {
            PE_003_step(num, divisor, max_divisor);
            PE_003_step(num, divisor + 2, max_divisor);
        });

        return num == 1 ? max_divisor : num;
    }

    static_assert(projectEuler_003(13195) == 29, "");
    static_assert(projectEuler_003(600851475143) == 6857, "");
}

// PE 004: Наибольшее палиндромное произведение
namespace
{
    template <class IntType>
    std::optional<IntType>
    projectEuler_004(IntType first, IntType last)
    {
        std::optional<IntType> result(std::nullopt);

        for(auto lhs = last; lhs != first; -- lhs)
        {
            for(auto rhs = lhs; rhs != first; -- rhs)
            {
                auto const prod = lhs * rhs;

                if(result.has_value() && prod < result.value())
                {
                    break;
                }

                auto const str = std::to_string(prod);

                if(saga::is_palindrome(saga::cursor::all(str))
                   && (!result.has_value() || result.value() < prod))
                {
                    result = prod;
                }
            }
        }

        return result;
    }
}

TEST_CASE("ProjectEuler: 004")
{
    CHECK(::projectEuler_004(10, 100).value() == 9009);
    CHECK(::projectEuler_004(100, 1000).value() == 906609);
}

// PE 005: Наименьшее кратное
namespace
{
    template <class IntType>
    constexpr
    IntType projectEuler_005(IntType num)
    {
        return saga::accumulate(saga::cursor::indices(2, num), IntType{1}, saga::lcm);
    }

    static_assert(projectEuler_005(10) == 2520, "");
    static_assert(projectEuler_005(20) == 232792560, "");
}

// PE 006: Разность сумм квадратов
#include <saga/math.hpp>

namespace
{
    template <class IntType>
    constexpr IntType projectEuler_006_sum_cursor(IntType num)
    {
        return saga::reduce(saga::cursor::indices(1, num+1), IntType{0});
    }

    template <class IntType>
    constexpr IntType projectEuler_006_sum_pipes(IntType num)
    {
        auto result = IntType{0};
        auto accumulator = [&result](IntType const & arg) { result += arg; };

        saga::copy(saga::cursor::indices(1, num+1), saga::pipes::for_each(accumulator));

        return result;
    }

    template <class IntType>
    constexpr IntType projectEuler_006_sum_squares_cursor(IntType num)
    {
        return saga::reduce(saga::cursor::indices(1, num+1)
                            | saga::cursor::transform(saga::square));
    }

    template <class IntType>
    constexpr IntType projectEuler_006_sum_squares_pipes(IntType num)
    {
        auto result = IntType{0};
        auto accumulator = [&result](IntType const & arg) { result += arg; };

        saga::copy(saga::cursor::indices(1, num+1)
                   , saga::pipes::transform(saga::pipes::for_each(accumulator), saga::square));

        return result;
    }

    template <class IntType>
    constexpr IntType projectEuler_006_cursor(IntType num)
    {
        return saga::square(::projectEuler_006_sum_cursor(num))
                - ::projectEuler_006_sum_squares_cursor(num);
    }

    template <class IntType>
    constexpr IntType projectEuler_006_pipes(IntType num)
    {
        return saga::square(::projectEuler_006_sum_pipes(num))
                - ::projectEuler_006_sum_squares_pipes(num);
    }

    template <class IntType>
    constexpr IntType projectEuler_006_formula(IntType num)
    {
        return saga::square(saga::triangular_number(num))
                - saga::square_pyramidal_number(num);
    }
}

static_assert(::projectEuler_006_sum_cursor(10) == 55, "");
static_assert(::projectEuler_006_sum_pipes(10) == 55, "");
static_assert(saga::triangular_number(10) == 55, "");

static_assert(saga::square(::projectEuler_006_sum_cursor(10)) == 3025, "");
static_assert(saga::square(::projectEuler_006_sum_pipes(10)) == 3025, "");
static_assert(saga::square(saga::triangular_number(10)) == 3025, "");

static_assert(::projectEuler_006_sum_squares_cursor(10) == 385, "");
static_assert(::projectEuler_006_sum_squares_pipes(10) == 385, "");
static_assert(saga::square_pyramidal_number(10) == 385, "");
static_assert(saga::square_pyramidal_number(10) == 385, "");

static_assert(::projectEuler_006_cursor(10) == 2640, "");
static_assert(::projectEuler_006_cursor(100) == 25164150, "");

static_assert(::projectEuler_006_formula(10) == 2640, "");
static_assert(::projectEuler_006_formula(100) == 25164150, "");

static_assert(::projectEuler_006_pipes(10) == 2640, "");
static_assert(::projectEuler_006_pipes(100) == 25164150, "");

// PE 007: 10001 Простое число
namespace
{
    template <class IntType>
    IntType projectEuler_007(IntType const count)
    {
        assert(count > 0);

        return saga::cursor::drop_front_n(saga::primes_cursor<IntType>(), count - 1)->back();
    }
}

TEST_CASE("ProjectEuler: 007")
{
    REQUIRE(projectEuler_007(6) == 13);
    REQUIRE(projectEuler_007(10001) == 104743);
}

// PE 008: Наибольшее произведение в последовательности
#include <saga/cursor/slide.hpp>

namespace
{
    static constexpr char pe008_data[] =
        "73167176531330624919225119674426574742355349194934"
        "96983520312774506326239578318016984801869478851843"
        "85861560789112949495459501737958331952853208805511"
        "12540698747158523863050715693290963295227443043557"
        "66896648950445244523161731856403098711121722383113"
        "62229893423380308135336276614282806444486645238749"
        "30358907296290491560440772390713810515859307960866"
        "70172427121883998797908792274921901699720888093776"
        "65727333001053367881220235421809751254540594752243"
        "52584907711670556013604839586446706324415722155397"
        "53697817977846174064955149290862569321978468622482"
        "83972241375657056057490261407972968652414535100474"
        "82166370484403199890008895243450658541227588666881"
        "16427171479924442928230863465674813919123162824586"
        "17866458359124566529476545682848912883142607690042"
        "24219022671055626321111109370544217506941658960408"
        "07198403850962455444362981230987879927244284909188"
        "84580156166097919133875499200524063689912560717606"
        "05886116467109405077541002256983155200055935729725"
        "71636269561882670428252483600823257530420752963450";

    template <std::size_t Size>
    constexpr auto projectEuler_008(char const (&str)[Size], std::size_t window_width)
    {
        assert(window_width <= std::size(str));

        auto prod = [](auto const & window)
        {
            return saga::accumulate(window, std::int64_t{1}, std::multiplies<>{});
        };

        auto input = saga::cursor::all(str)
                   | saga::cursor::transform([](char c) { return c - '0'; })
                   | saga::cursor::slide(window_width)
                   | saga::cursor::transform(prod);

        return saga::accumulate(std::move(input), std::int64_t(0), SAGA_OVERLOAD_SET(std::max));
    }

    static_assert(::projectEuler_008(pe008_data, 4) == 5832, "");
}

TEST_CASE("ProjectEuler: 008")
{
    CHECK(::projectEuler_008(pe008_data, 4) == 5832);
    CHECK(::projectEuler_008(pe008_data, 13) == 23514624000);
}

// PE 009: Особая Пифагорова тройка
namespace
{
    template <class IntType>
    constexpr IntType projectEuler_009_simple(IntType sum)
    {
        for(auto a : saga::cursor::indices(3, std::max(3, (sum - 3)/3)))
        for(auto b : saga::cursor::indices(a+1, std::max(a+1, (sum - 2)/2)))
        {
            auto const c = sum - a - b;

            if(a*a + b*b == c*c)
            {
                return a*b*c;
            }
        }

        return 0;
    }

    template <class IntType>
    constexpr IntType projectEuler_009_fast(IntType sum)
    {
        auto const sum_2 = sum / 2;

        auto const mlimit = std::max(2, static_cast<IntType>(std::sqrt(sum)+1));

        for(auto m : saga::cursor::indices(2, mlimit))
        {
            if(saga::is_divisible_by(sum_2, m))
            {
                auto const sum_m = saga::remove_factor(sum_2, 2).value;

                for(auto k = m + 1 + (m % 2); k < 2*m && k <= sum_m; k += 2)
                {
                    if(saga::is_divisible_by(sum_m, k) && std::gcd(k, m) == 1)
                    {
                        auto const d = sum_2  / (k*m);
                        auto const n = k - m;

                        auto const a = d*(m*m - n*n);
                        auto const b = 2*d*m*n;
                        auto const c = d*(m*m + n*n);

                        return a*b*c;
                    }
                }
            }
        }

        return 0;
    }

    // @note требуется constexpr sqrt
    // static_assert(::projectEuler_009_fast(1000) == 31875000, "");
}

TEST_CASE("ProjectEuler: 009")
{
    CHECK(::projectEuler_009_simple(1) == 0);
    CHECK(::projectEuler_009_simple(1000) == 31875000);

    CHECK(::projectEuler_009_fast(1) == 0);
    CHECK(::projectEuler_009_fast(1000) == 31875000);
}

// PE 010: Суммирование простых чисел
namespace
{
    template <class IntType>
    IntType projectEuler_010(IntType n_max)
    {
        auto primes = saga::primes_below(n_max);

        return saga::reduce(saga::cursor::all(primes));
    }
}

TEST_CASE("ProjectEuler: 010")
{
    CHECK(saga::primes_below(1).empty());
    CHECK(saga::primes_below(2).empty());

    CHECK(saga::primes_below(3).size() == 1);
    CHECK(saga::primes_below(3).front() == 2);

    REQUIRE(::projectEuler_010(10) == 17);
    REQUIRE(::projectEuler_010(2'000'000LL) == 142913828922);
}

// PE 011: Наибольшее произведение в решётке
#include <saga/cursor/istream_cursor.hpp>

#include <sstream>

namespace
{
    static constexpr char pe011_data[] =
        "08 02 22 97 38 15 00 40 00 75 04 05 07 78 52 12 50 77 91 08\n"
        "49 49 99 40 17 81 18 57 60 87 17 40 98 43 69 48 04 56 62 00\n"
        "81 49 31 73 55 79 14 29 93 71 40 67 53 88 30 03 49 13 36 65\n"
        "52 70 95 23 04 60 11 42 69 24 68 56 01 32 56 71 37 02 36 91\n"
        "22 31 16 71 51 67 63 89 41 92 36 54 22 40 40 28 66 33 13 80\n"
        "24 47 32 60 99 03 45 02 44 75 33 53 78 36 84 20 35 17 12 50\n"
        "32 98 81 28 64 23 67 10 26 38 40 67 59 54 70 66 18 38 64 70\n"
        "67 26 20 68 02 62 12 20 95 63 94 39 63 08 40 91 66 49 94 21\n"
        "24 55 58 05 66 73 99 26 97 17 78 78 96 83 14 88 34 89 63 72\n"
        "21 36 23 09 75 00 76 44 20 45 35 14 00 61 33 97 34 31 33 95\n"
        "78 17 53 28 22 75 31 67 15 94 03 80 04 62 16 14 09 53 56 92\n"
        "16 39 05 42 96 35 31 47 55 58 88 24 00 17 54 24 36 29 85 57\n"
        "86 56 00 48 35 71 89 07 05 44 44 37 44 60 21 58 51 54 17 58\n"
        "19 80 81 68 05 94 47 69 28 73 92 13 86 52 17 77 04 89 55 40\n"
        "04 52 08 83 97 35 99 16 07 97 57 32 16 26 26 79 33 27 98 66\n"
        "88 36 68 87 57 62 20 72 03 46 33 67 46 55 12 32 63 93 53 69\n"
        "04 42 16 73 38 25 39 11 24 94 72 18 08 46 29 32 40 62 76 36\n"
        "20 69 36 41 72 30 23 88 34 62 99 69 82 67 59 85 74 04 36 16\n"
        "20 73 35 29 78 31 90 01 74 31 49 71 48 86 81 16 23 57 05 54\n"
        "01 70 54 71 83 51 54 69 16 92 33 48 61 43 52 01 89 19 67 48";

    template <class T>
    std::vector<T> string_to_vector(std::string const & line)
    {
        std::istringstream line_in(line);

        return saga::make_istream_cursor<T>(line_in)
               | saga::cursor::to<std::vector>();
    }

    template <class IntType>
    std::vector<std::vector<IntType>>
    projectEuler_011_parse(std::istream & src_in)
    {
        std::vector<std::vector<IntType>> result;
        saga::transform(saga::cursor::by_line(src_in)
                       ,saga::back_inserter(result)
                       ,string_to_vector<IntType>);
        return result;
    }

    template <class IntType>
    std::vector<std::vector<IntType>>
    projectEuler_011_parse(std::string const & src)
    {
        std::istringstream src_in(src);

        return ::projectEuler_011_parse<IntType>(src_in);
    }

    template <class IntType>
    IntType
    projectEuler_011_horizontal(std::vector<std::vector<IntType>> const & data
                                , std::size_t row, std::size_t col, std::size_t num)
    {
        IntType result(1);

        for(auto pos : saga::cursor::indices(num))
        {
            result *= data.at(row).at(col + pos);
        }

        return result;
    }

    template <class IntType>
    IntType
    projectEuler_011_vertical(std::vector<std::vector<IntType>> const & data
                              , std::size_t row, std::size_t col, std::size_t num)
    {
        IntType result(1);

        for(auto pos : saga::cursor::indices(num))
        {
            result *= data.at(row + pos).at(col);
        }

        return result;
    }

    template <class IntType>
    IntType
    projectEuler_011_diagonal_plus(std::vector<std::vector<IntType>> const & data
                                   , std::size_t row, std::size_t col, std::size_t num)
    {
        IntType result(1);

        for(auto pos : saga::cursor::indices(num))
        {
            result *= data.at(row + pos).at(col + pos);
        }

        return result;
    }

    template <class IntType>
    IntType
    projectEuler_011_diagonal_minus(std::vector<std::vector<IntType>> const & data
                                    , std::size_t row, std::size_t col, std::size_t num)
    {
        IntType result(1);

        for(auto pos : saga::cursor::indices(num))
        {
            result *= data.at(row + pos).at(col - pos);
        }

        return result;
    }

    template <class IntType>
    IntType projectEuler_011(std::vector<std::vector<IntType>> const & data, std::size_t num)
    {
        assert(num > 0);

        auto const rows = data.size();
        auto const cols = data.front().size();

        assert(rows >= num && cols >= num);

        assert(saga::all_of(saga::cursor::all(data)
                            , [&cols](auto const & row) { return row.size() == cols; }));

        auto result = IntType(1);

        for(auto row : saga::cursor::indices(rows))
        for(auto col : saga::cursor::indices(cols))
        {
            if(col + num <= cols)
            {
                result = std::max(result, ::projectEuler_011_horizontal(data, row, col, num));
            }

            if(row + num <= rows)
            {
                result = std::max(result, ::projectEuler_011_vertical(data, row, col, num));
            }

            if(row + num <= rows && col + num <= cols)
            {
                result = std::max(result, ::projectEuler_011_diagonal_plus(data, row, col, num));
            }

            if(row + num <= rows && col + 1 >= num)
            {
                result = std::max(result, ::projectEuler_011_diagonal_minus(data, row, col, num));
            }
        }

        return result;
    }
}

TEST_CASE("ProjectEuler: 011")
{
    auto const data = ::projectEuler_011_parse<long long>(pe011_data);

    CHECK(projectEuler_011(data, 4) == 70600674);
}

// PE 012 - Сильно делимое треугольное число
namespace
{
    template <class IntType>
    IntType divisors_count(IntType value)
    {
        auto result = IntType(0);

        for(auto d = IntType(1); d*d <= value; ++ d)
        {
            if(saga::is_divisible_by(value, d))
            {
                result += 1;
                result += (value / d != d);
            }
        }

        return result;
    }

    template <class IntType>
    IntType projectEuler_012(IntType num_divisors_limit)
    {
        for(auto num = IntType(1);; ++ num)
        {
            auto m1 = saga::is_even(num) ? num/2 : num;
            auto m2 = saga::is_even(num) ? num + 1 : (num + 1)/2;

            auto div_count = ::divisors_count(m1) * ::divisors_count(m2);

            if(div_count > num_divisors_limit)
            {
                return m1 * m2;
            }
        }
    }
}

TEST_CASE("ProjectEuler: 012")
{
    REQUIRE(::projectEuler_012(5) == 28);
    REQUIRE(::projectEuler_012<long long>(500) == 76576500);
}

// PE 013 - Большая сумма
namespace
{
    static std::string PE_013_data[] = {
        "37107287533902102798797998220837590246510135740250",
        "46376937677490009712648124896970078050417018260538",
        "74324986199524741059474233309513058123726617309629",
        "91942213363574161572522430563301811072406154908250",
        "23067588207539346171171980310421047513778063246676",
        "89261670696623633820136378418383684178734361726757",
        "28112879812849979408065481931592621691275889832738",
        "44274228917432520321923589422876796487670272189318",
        "47451445736001306439091167216856844588711603153276",
        "70386486105843025439939619828917593665686757934951",
        "62176457141856560629502157223196586755079324193331",
        "64906352462741904929101432445813822663347944758178",
        "92575867718337217661963751590579239728245598838407",
        "58203565325359399008402633568948830189458628227828",
        "80181199384826282014278194139940567587151170094390",
        "35398664372827112653829987240784473053190104293586",
        "86515506006295864861532075273371959191420517255829",
        "71693888707715466499115593487603532921714970056938",
        "54370070576826684624621495650076471787294438377604",
        "53282654108756828443191190634694037855217779295145",
        "36123272525000296071075082563815656710885258350721",
        "45876576172410976447339110607218265236877223636045",
        "17423706905851860660448207621209813287860733969412",
        "81142660418086830619328460811191061556940512689692",
        "51934325451728388641918047049293215058642563049483",
        "62467221648435076201727918039944693004732956340691",
        "15732444386908125794514089057706229429197107928209",
        "55037687525678773091862540744969844508330393682126",
        "18336384825330154686196124348767681297534375946515",
        "80386287592878490201521685554828717201219257766954",
        "78182833757993103614740356856449095527097864797581",
        "16726320100436897842553539920931837441497806860984",
        "48403098129077791799088218795327364475675590848030",
        "87086987551392711854517078544161852424320693150332",
        "59959406895756536782107074926966537676326235447210",
        "69793950679652694742597709739166693763042633987085",
        "41052684708299085211399427365734116182760315001271",
        "65378607361501080857009149939512557028198746004375",
        "35829035317434717326932123578154982629742552737307",
        "94953759765105305946966067683156574377167401875275",
        "88902802571733229619176668713819931811048770190271",
        "25267680276078003013678680992525463401061632866526",
        "36270218540497705585629946580636237993140746255962",
        "24074486908231174977792365466257246923322810917141",
        "91430288197103288597806669760892938638285025333403",
        "34413065578016127815921815005561868836468420090470",
        "23053081172816430487623791969842487255036638784583",
        "11487696932154902810424020138335124462181441773470",
        "63783299490636259666498587618221225225512486764533",
        "67720186971698544312419572409913959008952310058822",
        "95548255300263520781532296796249481641953868218774",
        "76085327132285723110424803456124867697064507995236",
        "37774242535411291684276865538926205024910326572967",
        "23701913275725675285653248258265463092207058596522",
        "29798860272258331913126375147341994889534765745501",
        "18495701454879288984856827726077713721403798879715",
        "38298203783031473527721580348144513491373226651381",
        "34829543829199918180278916522431027392251122869539",
        "40957953066405232632538044100059654939159879593635",
        "29746152185502371307642255121183693803580388584903",
        "41698116222072977186158236678424689157993532961922",
        "62467957194401269043877107275048102390895523597457",
        "23189706772547915061505504953922979530901129967519",
        "86188088225875314529584099251203829009407770775672",
        "11306739708304724483816533873502340845647058077308",
        "82959174767140363198008187129011875491310547126581",
        "97623331044818386269515456334926366572897563400500",
        "42846280183517070527831839425882145521227251250327",
        "55121603546981200581762165212827652751691296897789",
        "32238195734329339946437501907836945765883352399886",
        "75506164965184775180738168837861091527357929701337",
        "62177842752192623401942399639168044983993173312731",
        "32924185707147349566916674687634660915035914677504",
        "99518671430235219628894890102423325116913619626622",
        "73267460800591547471830798392868535206946944540724",
        "76841822524674417161514036427982273348055556214818",
        "97142617910342598647204516893989422179826088076852",
        "87783646182799346313767754307809363333018982642090",
        "10848802521674670883215120185883543223812876952786",
        "71329612474782464538636993009049310363619763878039",
        "62184073572399794223406235393808339651327408011116",
        "66627891981488087797941876876144230030984490851411",
        "60661826293682836764744779239180335110989069790714",
        "85786944089552990653640447425576083659976645795096",
        "66024396409905389607120198219976047599490197230297",
        "64913982680032973156037120041377903785566085089252",
        "16730939319872750275468906903707539413042652315011",
        "94809377245048795150954100921645863754710598436791",
        "78639167021187492431995700641917969777599028300699",
        "15368713711936614952811305876380278410754449733078",
        "40789923115535562561142322423255033685442488917353",
        "44889911501440648020369068063960672322193204149535",
        "41503128880339536053299340368006977710650566631954",
        "81234880673210146739058568557934581403627822703280",
        "82616570773948327592232845941706525094512325230608",
        "22918802058777319719839450180888072429661980811197",
        "77158542502016545090413245809786882778948721859617",
        "72107838435069186155435662884062257473692284509516",
        "20849603980134001723930671666823555245252804609722",
        "53503534226472524250874054075591789781264330331690"};
}

#include <saga/numeric/digits_of.hpp>

#include <charconv>
#include <iomanip>

namespace
{
    template <class IntType>
    IntType from_chars_whole(char const * first, char const * last)
    {
        assert(first != last);

        auto reader = IntType(0);
        auto result = std::from_chars(first, last, reader);

        assert(result.ptr == last);
        assert(result.ec == std::errc{});

        return reader;
    }

    template <class IntType>
    IntType from_string_whole(std::string_view str)
    {
        return ::from_chars_whole<IntType>(str.data(), str.data() + str.size());
    }

    class integer10
    {
        // Равенство
        friend bool operator==(integer10 const & lhs, integer10 const & rhs)
        {
            return lhs.units_ == rhs.units_;
        }

        template <class IntType>
        friend auto operator==(integer10 const & lhs, IntType rhs)
        -> std::enable_if_t<std::is_integral<IntType>{}, bool>
        {
            return saga::equal(saga::cursor::all(lhs.data())
                              , saga::cursor::digits_of(rhs, lhs.unit_base));
        }

        // Вывод
        template <class CharT, class Traits>
        friend std::basic_ostream<CharT, Traits> &
        operator<<(std::basic_ostream<CharT, Traits> & out, integer10 const & value)
        {
            auto num = value.units_.size();

            if(num == 0)
            {
                out << '0';
                return out;
            }

            out << value.units_.back();
            -- num;

            for(; num > 0; -- num)
            {
                out << std::setw(value.digits_per_unit)
                    << std::setfill('0')
                    << value.units_[num - 1];
            }

            return out;
        }

        // Сложение
        friend integer10 operator+(integer10 lhs, integer10 const & rhs)
        {
            lhs += rhs;

            return lhs;
        }

        // Умножение
        friend integer10 operator*(integer10 const & lhs, integer10 const & rhs)
        {
            if(lhs.units_.empty() || rhs.units_.empty())
            {
                return integer10{};
            }

            auto result = integer10{};

            auto carry = Unit(0);

            auto const lhs_size = lhs.units_.size();
            auto const rhs_size = rhs.units_.size();

            for(auto pos : saga::cursor::indices(0*lhs_size, lhs_size + rhs_size))
            {
                auto new_unit = carry % unit_base;
                carry /= unit_base;

                auto const first = rhs_size <= pos ? pos - rhs_size + 1 : 0 * pos;
                auto const last = std::min(lhs_size, pos + 1);

                for(auto const & index : saga::cursor::indices(first, last))
                {
                    auto prod = lhs.units_[index] * rhs.units_[pos - index];

                    assert(prod < std::numeric_limits<Unit>::max() - new_unit);

                    new_unit += prod;
                }

                result.units_.push_back(new_unit % unit_base);
                carry += new_unit / unit_base;
            }

            while(!result.units_.empty() && result.units_.back() == 0)
            {
                assert(!result.units_.empty());

                result.units_.pop_back();
            }

            assert(carry == 0);

            return result;
        }

        template <class IntType>
        friend auto operator*(integer10 const & lhs, IntType rhs)
        -> std::enable_if_t<std::is_integral<IntType>{}, integer10>
        {
            return lhs * integer10(std::move(rhs));
        }


    private:
        using Unit = std::uint64_t;

        static_assert(std::is_unsigned<Unit>{});

    public:
        // @todo Сделать настраиваемым через шаблонный параметр,
        // Добавить проверку, что бит достаточно для хранения квадрата
        static constexpr auto digits_per_unit = 9;

        // Создание, копирование, уничтожение
        integer10() = default;

        template <class IntType, class = std::enable_if_t<std::is_integral<IntType>{}>>
        explicit integer10(IntType value)
        {
            assert(value >= 0);

            saga::copy(saga::cursor::digits_of(value, unit_base)
                      , saga::back_inserter(this->units_));
        }

        explicit integer10(std::string_view str)
        {
            auto const tail_size = str.size() % this->digits_per_unit;
            auto const units_count = str.size() / this->digits_per_unit + (tail_size != 0);

            this->units_.reserve(units_count);

            auto first = str.data();
            auto last = str.data() + (tail_size == 0 ? digits_per_unit : tail_size);
            auto const stop = str.data() + str.size();

            for(; first != stop; first = last, last += digits_per_unit)
            {
                this->units_.push_back(::from_chars_whole<Unit>(first, last));
            }

            this->units_ |= saga::actions::reverse;
        }

        // Арифметические операции
        integer10 & operator+=(integer10 const & rhs)
        {
            auto const num = std::max(this->units_.size(), rhs.units_.size());

            this->units_.resize(num, 0);

            auto carry = Unit(0);

            for(auto index : saga::cursor::indices(rhs.units_.size()))
            {
                carry += this->units_[index] + rhs.units_[index];

                this->units_[index] = carry % unit_base;

                carry /= unit_base;
            }

            for(auto index : saga::cursor::indices(rhs.units_.size(), this->units_.size()))
            {
                carry += this->units_[index];

                this->units_[index] = carry % unit_base;

                carry /= unit_base;
            }

            assert(carry < unit_base);

            if(carry > 0)
            {
                this->units_.push_back(carry);
            }

            return *this;
        }

        integer10 & operator*=(integer10 const & rhs)
        {
            *this = *this * rhs;

            return *this;
        }

        // Остаток
        void mod10(std::size_t power)
        {
            auto const tail_size = power % this->digits_per_unit;
            auto const units_to_keep = power / this->digits_per_unit + (tail_size != 0);

            if(this->units_.size() >= units_to_keep)
            {
                this->units_.resize(units_to_keep);

                if(tail_size > 0)
                {
                    this->units_.back() %= saga::power_natural(10, tail_size);
                }
            }
        }

        // Доступ к представлению
        std::vector<Unit> const & data() const
        {
            return this->units_;
        }

    private:
        static constexpr auto unit_base = saga::power_natural(Unit(10), digits_per_unit);

        std::vector<Unit> units_;
    };
}

TEST_CASE("integer10: default ctor")
{
    ::integer10 const zero{};

    std::ostringstream os;
    os << zero;

    std::string const str = os.str();

    REQUIRE(str == "0");
}

TEST_CASE("integer10: zero multiplication")
{
    saga_test::property_checker << [](unsigned const value)
    {
        ::integer10 const zero{};

        auto const prod = zero * ::integer10(value);

        REQUIRE(prod == zero);
    };
}

TEST_CASE("integer10: string ctor")
{
    REQUIRE(::integer10("1234567") == 1234567);
}

TEST_CASE("PE 013 - range for loop")
{
    ::integer10 result{};

    for(auto const & str : PE_013_data)
    {
        result += ::integer10(str);
    }

    result += ::integer10{};

    std::ostringstream os;
    os << result;

    REQUIRE(os.str().substr(0, 10) == "5537376230");
}

TEST_CASE("PE 013 - algorithms")
{
    auto const result
        = saga::transform_reduce(saga::cursor::all(PE_013_data), ::integer10{}
                                 , std::plus<>{}
                                 , [](std::string const & str) { return ::integer10(str); });

    std::ostringstream os;
    os << result;

    REQUIRE(os.str().substr(0, 10) == "5537376230");
}

// PE 014 Длиннейшая последовательность Коллатца
namespace
{
    template <class IntType>
    auto collatz_transform(IntType value)
    {
        return saga::is_even(value) ? value / 2 : 3*value+1;
    }

    template <class IntType>
    IntType collatz_length_no_memoization(IntType value)
    {
        assert(value > 0);

        auto result = IntType(1);

        for(; value != 1;)
        {
            value = ::collatz_transform(std::move(value));
            ++ result;
        }

        return result;
    }

    template <class IntType>
    IntType PE_014_no_memoization(IntType num)
    {
        assert(num > 1);

        auto result_value = IntType(1);
        auto result_length = IntType(1);

        for(auto const & value : saga::cursor::indices(2, num))
        {
            auto new_length = ::collatz_length_no_memoization(value);

            if(new_length > result_length)
            {
                result_length = new_length;
                result_value = value;
            }
        }

        return result_value;
    }
}

TEST_CASE("PE 014")
{
    using Value = long long;

    REQUIRE(::PE_014_no_memoization(Value(1'000'000)) == 837799);
}

// PE 015 : Пути в решётке
namespace
{
    template <class IntType>
    IntType projectEuler_015_dynamic(IntType const & row_count, IntType const & col_count)
    {
        assert(row_count > 0);

        std::vector<IntType> row(col_count + 1, 1);

        saga::for_n(row_count,
                    [&]
                    {
                        std::vector<IntType> new_row = row;

                        saga::partial_sum(saga::cursor::all(new_row), saga::cursor::all(new_row));

                        row.swap(new_row);
                    });

        return row.back();
    }

    template <class IntType>
    IntType projectEuler_015_formula(IntType const & row_count, IntType const & col_count);
}

TEST_CASE("PE 015")
{
    REQUIRE(::projectEuler_015_dynamic(2, 2) == 6);
    REQUIRE(::projectEuler_015_dynamic<long long>(20, 20) == 137846528820);
}

// PE 016: Сумма цифр степени
#include <saga/numeric/digits_of.hpp>

namespace
{
    template <class IntType, class Power>
    constexpr IntType
    projectEuler_016_buildin(IntType const & base, Power const & power)
    {
        return saga::reduce(saga::cursor::digits_of(saga::power_natural(base, power)));
    }

    long digits_sum(::integer10 const & num)
    {
        long digits_sum = 0;

        for(auto const & unit : saga::cursor::all(num.data()))
        {
            digits_sum += saga::reduce(saga::cursor::digits_of(unit));
        }

        return digits_sum;
    }

    int
    projectEuler_016_arbitrary(int base, int power)
    {
        assert(power > 0);

        auto num = saga::power_natural(::integer10(base), power);

        return ::digits_sum(num);
    }
}

static_assert(::projectEuler_016_buildin(2, 15) == 26, "");

TEST_CASE("PE 016")
{
    REQUIRE(::projectEuler_016_arbitrary(2, 15) == 26);
    REQUIRE(::projectEuler_016_arbitrary(2, 1000) == 1366);
}

// PE 017: Подсчёт количества букв
namespace
{
    std::string number_as_words(int value)
    {
        assert(value >= 0);
        assert(value <= 1000);

        static std::vector<std::string> const named_small
            {"", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten"
            , "eleven", "twelwe", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen"
            , "eighteen", "nineteen"};

        static std::vector<std::string> const dozens
            = {"twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety"};

        if(value == 1000)
        {
            return "one thousand";
        }

        std::string result;

        if(value >= 100)
        {
            result += named_small.at(value / 100);
            result += " hundred";

            value %= 100;

            if(value > 0)
            {
                result += " and ";
            }
        }

        if(value >= 20)
        {
            result += dozens.at(value / 10 - 2);
            value %= 10;

            if(value > 0)
            {
                result += "-";
            }
        }

        result += named_small.at(value);

        return result;
    }

    std::size_t projectEuler_017(int max_value)
    {
        auto result = std::size_t{0};

        for(auto value : saga::cursor::indices(1, max_value+1))
        {
            auto words = ::number_as_words(value);

            result += saga::count_if(saga::cursor::all(words)
                                     , [](char val) { return val != ' ' && val != '-'; });
        }

        return result;
    }
}

TEST_CASE("PE 017")
{
    CHECK(::number_as_words(1) == "one");
    CHECK(::number_as_words(2) == "two");
    CHECK(::number_as_words(3) == "three");
    CHECK(::number_as_words(4) == "four");
    CHECK(::number_as_words(5) == "five");

    CHECK(::number_as_words(342) == "three hundred and forty-two");
    CHECK(::number_as_words(115) == "one hundred and fifteen");

    CHECK(::projectEuler_017(5) == 19);
    CHECK(::projectEuler_017(1000) == 21124);
}

// PE 018: Путь наибольшей суммы (часть I)
namespace
{
    static std::string pe018_sample_data{
        "3\n"
        "7 4\n"
        "2 4 6\n"
        "8 5 9 3"};

    static std::string pe018_data{
        "75\n"
        "95 64\n"
        "17 47 82\n"
        "18 35 87 10\n"
        "20 04 82 47 65\n"
        "19 01 23 75 03 34\n"
        "88 02 77 73 07 63 67\n"
        "99 65 04 28 06 16 70 92\n"
        "41 41 26 56 83 40 80 70 33\n"
        "41 48 72 33 47 32 37 16 94 29\n"
        "53 71 44 65 25 43 91 52 97 51 14\n"
        "70 11 33 28 77 73 17 78 39 68 17 57\n"
        "91 71 52 38 17 14 91 43 58 50 27 29 48\n"
        "63 66 04 68 89 53 67 30 73 16 69 87 40 31\n"
        "04 62 98 27 23 09 70 98 73 93 38 53 60 04 23"};

    template <class Vector>
    Vector projectEuler_018_update(Vector & result, Vector && row)
    {
        assert(row.size() == result.size() + 1);

        if(row.size() > 1)
        {
            row.front() += result.front();
            row.back() += result.back();

            for(auto index : saga::cursor::indices(1u, result.size()))
            {
                row[index] += std::max(result[index-1], result[index]);
            }
        }

        result.swap(row);

        return result;
    }

    template <class IntType>
    IntType projectEuler_018(std::istream & src_in)
    {
        std::vector<IntType> result;
        auto update = [&result](std::string const & line)
        {
            ::projectEuler_018_update(result, ::string_to_vector<IntType>(line));
        };

        saga::for_each(saga::cursor::by_line(src_in), update);

        assert(!result.empty());

        return saga::max_element(saga::cursor::all(result)).front();
    }


    template <class IntType>
    IntType projectEuler_018_string(std::string const & text)
    {
        std::istringstream istr(text);

        return ::projectEuler_018<IntType>(istr);
    }
}

TEST_CASE("PE 018")
{
    REQUIRE(projectEuler_018_string<int>(pe018_sample_data) == 23);
    REQUIRE(projectEuler_018_string<int>(pe018_data) == 1074);
}

// PE 019 Подсчёт числа воскресений
namespace
{
    bool is_leap(int year)
    {
        return saga::is_divisible_by(year, 4)
               - (saga::is_divisible_by(year, 100) && !saga::is_divisible_by(year, 400));
    }

    int days_in_month(int month, int year)
    {
        switch(month)
        {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            return 31;

        case 2:
            return 28 + ::is_leap(year);

        default:
            return 30;
        }
    }
}

TEST_CASE("PE 019")
{
    REQUIRE(::days_in_month(2, 1900) == 28);
    REQUIRE(::days_in_month(2, 2000) == 29);

    auto result = 0;

    auto weekday = 2;

    for(auto year : saga::cursor::indices(1901, 2001))
    {
        for(auto month : saga::cursor::indices(1, 13))
        {
            result += (weekday == 0);

            weekday += ::days_in_month(month, year);
            weekday %= 7;
        }
    }

    REQUIRE(result == 171);
}

// PE 020: Сумма цифр факториала
namespace
{
    template <class IntType>
    IntType projectEuler_020(IntType num)
    {
        auto const factorial = saga::accumulate(saga::cursor::indices(1, num)
                                                , ::integer10(1), std::multiplies<>{});

        return ::digits_sum(factorial);
    }
}

TEST_CASE("PE 020")
{
    REQUIRE(::projectEuler_020(10) == 27);
    REQUIRE(::projectEuler_020(100) == 648);
}

// PE 021: Дружественные числа
namespace
{
    template <class IntType>
    std::vector<IntType>
    proper_divisors_sums_below(IntType n_max)
    {
        std::vector<IntType> d_sum(n_max, 1);

        for(auto d : saga::cursor::indices(2, n_max))
        {
            for(auto num = 2*d; num < n_max; num += d)
            {
                d_sum[num] += d;
            }
        }

        return d_sum;
    }

    template <class IntType>
    IntType projectEuler_021(IntType n_max)
    {
        auto const d_sum = ::proper_divisors_sums_below(n_max);

        auto result = IntType(0);
        for(auto a : saga::cursor::indices(2, n_max))
        {
            auto const b = d_sum[a];

            if(b < n_max && a != b && d_sum[b] == a)
            {
                result += a;
            }
        }

        return result;
    }
}

TEST_CASE("PE 021")
{
    REQUIRE(::projectEuler_021(10000) == 31626);
}

namespace
{
    long alphabetical_value(std::string_view str)
    {
        auto letter_pos = [](char letter)
        {
            assert(std::isalpha(letter));

            return std::toupper(letter) - 'A' + 1;
        };

        return saga::transform_reduce(saga::cursor::all(str), long{0}, std::plus<>{}, letter_pos);
    }
}

// PE 022: Баллы для имён
#include <fstream>

namespace
{
    std::vector<std::string>
    read_quoted_comma_separated(std::istream & src)
    {
        std::vector<std::string> names;

        std::string reader;
        for(; src; )
        {
            char c = src.get();
            if(std::isalpha(c))
            {
                reader.push_back(c);
            }
            else if(c == ',')
            {
                names.push_back(std::move(reader));
                reader.clear();
            }
        }

        if(!reader.empty())
        {
            names.push_back(std::move(reader));
        }

        return names;
    }

    std::vector<std::string>
    read_quoted_comma_separated_from_file(std::string const & path)
    {
        std::ifstream file(path);

        assert(!!file);

        return ::read_quoted_comma_separated(file);
    }
}

TEST_CASE("PE 022")
{
    REQUIRE(::alphabetical_value("COLIN") == 53);

    auto names = ::read_quoted_comma_separated_from_file("ProjectEuler/p022_names.txt");

    // Отсортировать
    names |= saga::actions::sort;

    REQUIRE(names[938 - 1] == "COLIN");

    // Вычислить алфавитные значения имён и получить ответ
    auto const result
        = saga::transform_reduce(saga::cursor::indices_of(names)
                                 , saga::cursor::all(names), long{0}, std::plus<>{}
                                 , [](long index, std::string const & name)
                                   { return (index + 1) * ::alphabetical_value(name); });

    REQUIRE(result == 871'198'282);
}

// PE 023 Не-избыточные суммы
TEST_CASE("PE 023")
{
    using IntType = long;

    // Установлено с помощью математического анализа
    auto const n_max = IntType(28123 + 1);

    std::vector<IntType> abundants;

    saga::copy_if(saga::cursor::indices(1, n_max), saga::back_inserter(abundants)
                  , [d_sum = ::proper_divisors_sums_below(n_max)](IntType num)
                    { return d_sum[num] > num ; });

    REQUIRE(abundants.front() == 12);

    std::vector<IntType> marks(n_max, true);

    for(auto cur1 = saga::cursor::all(abundants); !!cur1; ++ cur1)
    for(auto cur2 = cur1; !!cur2; ++ cur2)
    {
        auto const sum = *cur1 + *cur2;

        if(sum < n_max)
        {
            marks[sum] = false;
        }
        else
        {
            break;
        }
    }

    auto result = IntType(0);

    for(auto const & num : saga::cursor::indices(1, n_max))
    {
        result += num * marks[num];
    }

    REQUIRE(result == 4'179'871);
}

// PE 024: Лексикографические перестановки
TEST_CASE("PE 024")
{
    std::string digits("0123456789");

    saga::nth_permutation(saga::cursor::all(digits), 1'000'000 - 1);

    REQUIRE(digits == "2783915460");
}

// PE 025: 1000-значное число Фибоначчи
namespace
{
    std::size_t projectEuler_025(std::size_t digits)
    {
        auto to_str = [](integer10 const & num)
        {
            std::ostringstream os;
            os << num;
            return os.str();
        };

        auto fib_str = ::make_fibonacci_sequence(::integer10(1), ::integer10(1))
                     | saga::cursor::transform(to_str);

        auto cur = saga::find_if(std::move(saga::cursor::enumerate(fib_str))
                                ,[&](auto const & elem) {return elem.value.size() >= digits;});
        assert(!!cur);

        return cur.front().index + 1;
    }
}

TEST_CASE("PE 025")
{
    CHECK(projectEuler_025(3) == 12);
    CHECK(projectEuler_025(1000) == 4782);
}

// PE 026: Циклы в обратных числах
#include <saga/cursor/cached1.hpp>

namespace
{
    class pe_026_cursor
     : saga::cursor_facade<pe_026_cursor, int const &>
    {
    public:
        // Типы
        using reference = int const &;
        using difference_type = std::ptrdiff_t;

        // Конструктор
        explicit pe_026_cursor(int denom)
         : denom_(denom)
        {}

        // Курсор ввода
        bool operator!() const
        {
            return this->state_ == 0;
        }

        reference front() const
        {
            return this->state_;
        }

        void drop_front()
        {
            this->state_ *= 10;
            this->state_ %= this->denom_;
        }

    private:
        int state_ = 1;
        int denom_ = 1;
    };

    template <class ForwardCursor>
    ForwardCursor collision_point(ForwardCursor slow)
    {
        if(!slow)
        {
            return slow;
        }

        auto fast = slow;
        ++ fast;

        for(;;)
        {
            if(!fast || *slow == *fast)
            {
                return fast;
            }

            ++ slow;
            ++ fast;

            if(!fast)
            {
                return fast;
            }

            ++ fast;
        }

        return fast;
    }

    template <class ForwardCursor>
    saga::cursor_difference_t<ForwardCursor>
    cycle_length(ForwardCursor slow)
    {
        slow = ::collision_point(slow);

        if(!slow)
        {
            return saga::cursor_difference_t<ForwardCursor>(0);
        }

        auto fast = slow;
        ++ fast;

        auto result = saga::cursor_difference_t<ForwardCursor>(1);

        for(; !(*fast == *slow); ++ fast, ++ result)
        {}

        return result;
    }

    int PE_026_reciprocal_cycle_length(int num)
    {
        assert(num > 0);

        return ::cycle_length(pe_026_cursor(num));
    }

    int PE_026(int num)
    {
        auto cur = saga::cursor::indices(2, num)
                 | saga::cursor::transform(::PE_026_reciprocal_cycle_length)
                 | saga::cursor::cached1;

        return saga::max_element(std::move(cur)).base().base().front();
    }
}

TEST_CASE("cycle_length of empty")
{
    REQUIRE(::cycle_length(saga::cursor::indices(0)) == 0);
}

TEST_CASE("PE 026")
{
    REQUIRE(PE_026_reciprocal_cycle_length(2)  == 0);
    REQUIRE(PE_026_reciprocal_cycle_length(3)  == 1);
    REQUIRE(PE_026_reciprocal_cycle_length(4)  == 0);
    REQUIRE(PE_026_reciprocal_cycle_length(5)  == 0);
    REQUIRE(PE_026_reciprocal_cycle_length(6)  == 1);
    REQUIRE(PE_026_reciprocal_cycle_length(7)  == 6);
    REQUIRE(PE_026_reciprocal_cycle_length(8)  == 0);
    REQUIRE(PE_026_reciprocal_cycle_length(9)  == 1);
    REQUIRE(PE_026_reciprocal_cycle_length(10) == 0);

    REQUIRE(PE_026(11) == 7);
    REQUIRE(PE_026(1000) == 983);
}

// PE 027: Квадратичные простые числа
namespace
{
    template <class IntType, class Primes>
    IntType PE_027_count(IntType a, IntType b, Primes const & primes)
    {
        auto value = b;

        for(auto const & n : saga::cursor::indices(std::abs(b) + 1))
        {
            assert(saga::square(primes.back()) >= value);

            if(!is_prime_sorted(std::abs(value), primes, saga::unsafe_tag_t{}))
            {
                return n;
            }

            value += 2 * n + 1 + a;
        }

        assert(true);

        return std::abs(b);
    }
}

/* Мы рассматриваем числа вида n^2 + a*n + b
 |a|<1000 и |b|<=1000
 При n=|b| получаем гарантировано простое число, так что n достаточно перебирать до |b|
 Самое большое число b*b+a*b+b <= 2'001'000.
 Так что для проверки на простоту нужны только числа до sqrt(2'001'000)
*/
TEST_CASE("PE 027")
{
    auto const primes = saga::primes_below(static_cast<int>(std::sqrt(2'001'000)));

    REQUIRE(PE_027_count(1, 41, primes) == 40);
    REQUIRE(PE_027_count(-79, 1601, primes) == 80);
    REQUIRE(PE_027_count(-999, -1000, primes) == 0);

    auto fun = [&](auto const & value) { return PE_027_count(value.first, value.second, primes); };

    auto cur = saga::cursor::cartesian_product(saga::cursor::indices(-1000 + 1, 1000)
                                               , saga::cursor::indices(-1000, 1001))
             | saga::cursor::transform(std::ref(fun))
             | saga::cursor::cached1;

    auto const pos = saga::max_element(std::move(cur)).base().base();

    REQUIRE(std::get<0>(*pos) * std::get<1>(*pos) == -59'231);
}

// PE 028: Диагонали числовой суммы
namespace
{
    template <class IntType>
    constexpr IntType PE_028_algorithm(IntType max_width)
    {
        auto corners_sum = [](IntType num) { return 16*saga::square(num) + 4*num + 4; };

        namespace cursor = saga::cursor;

        return saga::transform_reduce(cursor::indices(1, (max_width+1)/2)
                                      , IntType(1), std::plus<>{}, corners_sum);
    }

    template <class IntType>
    constexpr IntType PE_028_closed_form(IntType max_width)
    {
        auto const num = (max_width - 1)/2;

        return 16*saga::square_pyramidal_number(num) + 4*saga::triangular_number(num) + 4*num + 1;
    }

    static_assert(PE_028_algorithm(5) == 101, "");
    static_assert(PE_028_algorithm(1001) == 669'171'001, "");

    static_assert(PE_028_closed_form(5) == 101, "");
    static_assert(PE_028_closed_form(1001) == 669'171'001, "");
}

TEST_CASE("PE 028")
{
    REQUIRE(PE_028_algorithm(5) == 101);
    REQUIRE(PE_028_algorithm(1001) == 669'171'001);

    REQUIRE(saga::square_pyramidal_number(1) == 1);

    REQUIRE(PE_028_closed_form(5) == 101);
    REQUIRE(PE_028_closed_form(1001) == 669'171'001);
}

// PE 029: Различные степени
namespace
{
    int PE_029(int n_max)
    {
        using Real = double;

        saga::flat_set<std::string> powers;

        auto param_range = saga::cursor::indices(2, n_max + 1);

        saga::transform(saga::cursor::cartesian_product(param_range, param_range)
                        , saga::inserter(powers, powers.end())
                        , [](auto args)
                          { return std::to_string(args.second * std::log(Real(args.first))); });

        return powers.size();
    }
}

TEST_CASE("PE 029")
{
    REQUIRE(PE_029(5) == 15);
    REQUIRE(PE_029(100) == 9183);
}

// PE 031: Суммы монет
namespace
{
    template <class IntType, class Range>
    IntType summations_count(IntType number, Range addends)
    {
        assert(number > 0);

        std::vector<IntType> counts(number + 1, 0);
        counts.front() = 1;

        for(auto max_addend : addends)
        {
            for(auto num : saga::cursor::indices(max_addend, number + 1))
            {
                counts[num] += counts[num - max_addend];
            }
        }

        return counts.back();
    }
}

TEST_CASE("PE 031")
{
    using IntType = std::size_t;

    auto const coins = saga::make_container<std::vector>(1, 2, 5, 10, 20, 50, 100, 200);

    REQUIRE(::summations_count(IntType(200), coins) == 73682);
}

// PE 032: Панцифирные произведения
TEST_CASE("PE 032")
{
    using IntType = long;
    auto const min_mult = saga::power_natural(IntType(10), 2);
    auto const max_mult = saga::power_natural(IntType(10), 4);

    saga::flat_set<IntType> obj;

    for(auto lhs : saga::cursor::indices(min_mult, max_mult))
    {
        auto const lhs_str = std::to_string(lhs);

        for(auto rhs : saga::cursor::indices(1, lhs))
        {
            auto rhs_str = std::to_string(rhs);

            auto prod = lhs * rhs;

            auto str = lhs_str + rhs_str + std::to_string(prod);

            if(str.size() > 9)
            {
                break;
            }

            str |= saga::actions::sort;

            if(str == "123456789")
            {
                // @todo Убрать первый аргумент?
                obj.insert(obj.end(), std::move(prod));
            }
        }
    }

    REQUIRE(saga::reduce(saga::cursor::all(obj)) == 45228);
}

// PE 033: Дроби с сокращающимися цифрами
namespace
{
    template <class IntType>
    constexpr IntType PE_033_is_canceling(IntType num, IntType denom, IntType a)
    {
        return (num*(10*a+denom) == denom*(10*num + a))
                || (num*(10*denom+a) == denom*(10*a + num));
    }

    template <class IntType>
    constexpr IntType PE_033()
    {
        auto numerator = IntType(1);
        auto denominator = IntType(1);

        for(auto denom : saga::cursor::indices(1, 10))
        for(auto num : saga::cursor::indices(1, denom))
        for(auto a : saga::cursor::indices(1, 10))
        {
            if(::PE_033_is_canceling(num, denom, a))
            {
                numerator *= num;
                denominator *= denom;
            }
        }

        return denominator / saga::gcd(numerator, denominator);
    }

    static_assert(PE_033<int>() == 100, "");
}

TEST_CASE("PE 033")
{
    REQUIRE(PE_033<int>() == 100);
}

// PE 34: Факториалы цифр
namespace
{
    template <class IntType>
    constexpr std::array<IntType, 10> digits_factorials_table()
    {
        std::array<IntType, 10> factorials{1};

        saga::iota(saga::cursor::all(factorials), 0);
        factorials.front() = 1;

        saga::partial_sum(saga::cursor::all(factorials), saga::cursor::all(factorials)
                          , std::multiplies<>{});

        return factorials;
    }

    template <class IntType>
    IntType digits_factorial_sum(IntType num)
    {
        static constexpr auto factorials = ::digits_factorials_table<IntType>();

        return saga::transform_reduce(saga::cursor::digits_of(num), IntType(0)
                                      , std::plus<>{}
                                      , [&](IntType arg) { return factorials[arg]; });
    }

    long PE_034()
    {
        using IntType = long;

        // Можно показать, что если количество цифр больше 7, то сумма факториалов цифр всегда
        // меньше самого этого числа
        auto numbers = saga::cursor::indices(IntType(10), saga::power_natural(IntType(10), 7));
        auto pred = [](IntType num) { return num == digits_factorial_sum(num); };

        return saga::reduce(saga::cursor::filter(std::move(numbers), pred));
    }
}

TEST_CASE("PE 034")
{
    REQUIRE(::digits_factorial_sum(1) == 1);
    REQUIRE(::digits_factorial_sum(2) == 2);
    REQUIRE(::digits_factorial_sum(145) == 145);
    REQUIRE(::digits_factorial_sum(40585) == 40585);

    REQUIRE(PE_034() == 40730);
}

// PE 35: Круговые простые числа
namespace
{
    template <class IntType>
    bool PE_035_is_circular_prime(IntType prime, std::vector<IntType> const & primes)
    {
        IntType num = std::log10(prime);
        auto const mask = (num == 0 ? IntType(1) : saga::power_natural(IntType(10), num));

        for(; num > 0; -- num)
        {
            auto last = prime % 10;

            prime /= 10;
            prime += mask * last;

            if(!saga::binary_search(saga::cursor::all(primes), prime))
            {
                return false;
            }
        }

        return true;
    }

    auto PE_035(std::uint64_t max_num)
    {
        auto const primes = saga::primes_below(max_num);

        return saga::count_if(saga::cursor::all(primes)
                              , [&](auto prime)
                                { return ::PE_035_is_circular_prime(std::move(prime), primes);});
    }
}

TEST_CASE("PE 035")
{
    REQUIRE(::PE_035(100) == 13);
    REQUIRE(::PE_035(1'000'000) == 55);
}

// PE 036 Палиндромы по двум основаниям
#include <saga/numeric/polynomial.hpp>

namespace
{
    template <class IntType, class Base>
    constexpr IntType reverse_number(IntType const num, Base base)
    {
        return saga::polynomial_horner(saga::cursor::digits_of(num, base), base, IntType(0));
    }

    template <class IntType, class Base>
    constexpr bool is_palindrome_in_base(IntType const num, Base base)
    {
        return !saga::is_divisible_by(num, base) && ::reverse_number(num, base) == num;
    }

    template <class IntType, class Base>
    IntType make_palindrome(IntType seed, Base base, bool odd)
    {
        return saga::polynomial_horner(saga::cursor::digits_of(odd ? seed / base : seed, base)
                                       , base, seed);
    }

    static_assert(::is_palindrome_in_base(585, 2));
    static_assert(::is_palindrome_in_base(585, 10));
}

TEST_CASE("PE 036")
{
    using IntType = long;

    REQUIRE(::is_palindrome_in_base(IntType(585), 2));
    REQUIRE(::is_palindrome_in_base(IntType(585), 10));
    REQUIRE(!::is_palindrome_in_base(IntType(580), 10));

    auto result = IntType(0);

    for(auto seed : saga::cursor::indices(1, 1'000))
    {
        if(auto const num = ::make_palindrome(seed, 2, true); ::is_palindrome_in_base(num, 10))
        {
            result += num;
        }

        if(auto const num = ::make_palindrome(seed, 2, false); ::is_palindrome_in_base(num, 10))
        {
            result += num;
        }
    }

    REQUIRE(result == 872187);
}

// PE 037 - Усекаемые простые числа
namespace
{
    template <class IntType>
    IntType trunc_left(IntType num)
    {
        auto M = saga::power_natural(10, IntType(std::log10(num)));

        return num % M;
    }

    template <class IntType>
    IntType trunc_right(IntType num)
    {
        return (num - (num % 10)) / 10;
    }
}

// Количество искомых чисел дано в условии задачи.
// Более естественное условие существенно увеличивает время выполнения,
// так как нужно исчерпать все 83 усекаемые справа простые числа
TEST_CASE("PE 037")
{
    using IntType = long;
    auto primes_cur = saga::primes_cursor<IntType>();

    // Левые подстроки-кандидаты
    std::vector<IntType> lefts;

    for(; primes_cur->back() < 10; ++primes_cur)
    {
        lefts.push_back(primes_cur->back());
    }

    // Правые подстроки кандидаты
    auto rights = lefts;

    // Ищем усекаемые простые числа
    std::vector<IntType> results;

    for(; results.size() < 11; ++primes_cur)
    {
        auto const num = primes_cur->back();

        if(saga::binary_search(saga::cursor::all(rights), ::trunc_right(num)))
        {
            rights.push_back(num);
        }
        if(saga::binary_search(saga::cursor::all(lefts), ::trunc_left(num)))
        {
            lefts.push_back(num);
        }
        if(lefts.back() == num && rights.back() == num)
        {
            results.push_back(num);
        }
    }

    REQUIRE(saga::reduce(saga::cursor::all(results)) == 748'317);
}

// PE 038: Панцифровые кратные
namespace
{
    template <class IntType>
    IntType PE_038_pandigital(IntType num)
    {
        assert(num > 0);

        static auto const all_digits = std::string("123456789");

        std::string str;

        for(auto mult = 1; str.size() < 9; mult += 1)
        {
            str += std::to_string(num * mult);
        }

        if(saga::is_permutation(saga::cursor::all(str), saga::cursor::all(all_digits)))
        {
            return ::from_string_whole<IntType>(str);
        }
        else
        {
            return IntType(0);
        }
    }
}

TEST_CASE("PE 038")
{
    using IntType = long;

    REQUIRE(::PE_038_pandigital(192) == 192'384'576);
    REQUIRE(::PE_038_pandigital(9) == 918'273'645);

    // Если в числе A пять цифр, то в конкатенации A и A*2 не менее 10 цифр!
    auto const result = saga::transform_reduce(saga::cursor::indices(1, IntType(10'000))
                                               , IntType(0), SAGA_OVERLOAD_SET(std::max)
                                               , SAGA_OVERLOAD_SET(::PE_038_pandigital));

    REQUIRE(result == 932'718'654);
}

// PE 039 Целые прямоугольные треугольники
// @todo Оптимизация перебора
TEST_CASE("PE 039")
{
    auto const p_max = 1000;

    std::vector<int> counts(p_max+1, 0);

    for(auto a : saga::cursor::indices(1, 500))
    for(auto b : saga::cursor::indices(1, a))
    {
        auto const c = std::hypot(a, b);

        if(!(int(c) < c) && a + b + c <= 1000)
        {
            counts.at(a + b + c) += 1;
        }
    }

    REQUIRE(counts.at(120) == 3);

    REQUIRE(std::max_element(counts.begin(), counts.end()) - counts.begin() == 840);
}

// PE 040 - Постоянная Чемперноуна
namespace
{
    int PE_040_champernowne(int const index)
    {
        if(index < 10)
        {
            return index;
        }

        int skipped_length = 10;
        int number_length = 2;
        int numbers_to_skip = 90;
        int base = 10;

        for(;;)
        {
            auto to_skip = number_length * numbers_to_skip;

            if(index < skipped_length + to_skip)
            {
                break;
            }

            numbers_to_skip *= 10;
            skipped_length += to_skip;
            number_length += 1;
            base *= 10;
        }

        auto const num = base + (index - skipped_length) / number_length;
        auto const rest = number_length * (num - base + 1) + skipped_length - index;
        assert(rest >= 0);

        auto const str = std::to_string(num);
        assert(str.size() == static_cast<std::size_t>(number_length));

        return str[str.size() - rest] - '0';
    }
}

TEST_CASE("PE 040")
{
    for(auto num : saga::cursor::indices(10))
    {
        REQUIRE(::PE_040_champernowne(num) == num);
    }

    REQUIRE(::PE_040_champernowne(12) == 1);

    REQUIRE(::PE_040_champernowne(1) == 1);
    REQUIRE(::PE_040_champernowne(10) == 1);

    int const result = ::PE_040_champernowne(1)
                     * ::PE_040_champernowne(10)
                     * ::PE_040_champernowne(100)
                     * ::PE_040_champernowne(1000)
                     * ::PE_040_champernowne(10000)
                     * ::PE_040_champernowne(100000)
                     * ::PE_040_champernowne(1000000);

    REQUIRE(result == 210);
}

// PE 041 -  Панцифровое простое число
// Так как нам нужно самое большое число, то искать начнём с самых больших чисел
namespace
{
    bool is_pandigital(std::string str)
    {
        static const auto all_digits = std::string("123456789");

        str |= saga::actions::sort;

        return all_digits.compare(0, str.size(), str) == 0;
    }

    template <class IntType>
    IntType PE_041(std::size_t digits)
    {
        assert(1 <= digits && digits <= 9);

        std::string str;

        for(; digits > 0; --digits)
        {
            str.push_back('0' + digits);
        }

        auto const primes = saga::primes_below(IntType(std::sqrt(std::stoi(str)))+1);

        for(auto cur = saga::cursor::all(str); !!cur; ++ cur)
        {
            assert(saga::is_sorted(saga::cursor::all(str), std::greater<>{}));

            do
            {
                auto num = ::from_chars_whole<IntType>(str.data() + str.size() - cur.size()
                                                      ,str.data() + str.size());

                if(saga::is_prime_sorted(num, primes, saga::unsafe_tag_t{}) && num > IntType(1))
                {
                    return num;
                }
            }
            while(saga::prev_permutation(cur));
        }

        return -1;
    }
}

TEST_CASE("PE 041")
{
    REQUIRE(::is_pandigital(std::to_string(2143)));

    REQUIRE(PE_041<int>(2) == -1);
    REQUIRE(PE_041<decltype(987654321)>(9) == 7'652'413);
}

// PE 042 - Кодированные треугольные числа
namespace
{
    struct is_triangular_number_fn
    {
        template <class IntType>
        bool operator()(IntType num) const
        {
            // Решаем уравнение x*(x-1)/2 == num и проверяем, что решение целое
            auto const x = (std::sqrt(8*num + 1) + 1)/2;

            return !(std::floor(x) < x);
        }
    };
}

TEST_CASE("PE 042")
{
    auto const words = ::read_quoted_comma_separated_from_file("ProjectEuler/p042_words.txt");

    REQUIRE(::alphabetical_value("SKY") == 55);
    REQUIRE(::is_triangular_number_fn{}(::alphabetical_value("SKY")));

    REQUIRE(saga::count_if(saga::cursor::all(words) | saga::cursor::transform(::alphabetical_value)
                          , is_triangular_number_fn{}) == 162);
}

// PE 043 - Делимость под-строк
namespace
{
    template <class IntType>
    bool PE_043_check(IntType num)
    {
        constexpr int primes[] = {17, 13, 11, 7, 5, 3, 2};

        for(auto prime : primes)
        {
            if(!saga::is_divisible_by(num % 1000, prime))
            {
                return false;
            }

            num /= 10;
        }

        return true;
    }

    // Рефакторинг
    template <class Container, class Difference = typename Container::difference_type>
    class permutations_cursor
     : saga::cursor_facade<permutations_cursor<Container, Difference>, Container const &>
    {
    public:
        // Типы
        using reference = Container const &;
        using difference_type = Difference;
        using cursor_category = std::input_iterator_tag;

        // Конструктор
        explicit permutations_cursor(Container container)
         : data_(std::move(container))
         , stop_(this->data_.empty())
        {}

        // Курсор ввода
        bool operator!() const
        {
            return this->stop_;
        }

        reference front() const
        {
            return this->data_;
        }

        void drop_front()
        {
            assert(!!*this);
            this->stop_ = !saga::next_permutation(saga::cursor::all(this->data_));
        }

    private:
        Container data_;
        bool stop_ = true;
    };

    template <class Container>
    permutations_cursor<Container>
    permutations(Container container)
    {
        return permutations_cursor<Container>(std::move(container));
    }

    template <class IntType>
    IntType PE_043_6th_digit(std::string arg, char digit)
    {
        assert(std::isdigit(digit));

        auto to_number = [&](std::string str)
        {
            assert(str.size() == 9);

            constexpr auto n_half = 5;

            str.insert(str.begin() + n_half, digit);

            return ::from_string_whole<IntType>(str);
        };

        auto cur = ::permutations(std::move(arg))
                 | saga::cursor::transform(to_number)
                 | saga::cursor::filter(::PE_043_check<IntType>);

        return saga::reduce(std::move(cur));
    }
}

TEST_CASE("PE 043")
{
    REQUIRE(::PE_043_check(1406357289));

    auto const expected = 16'695'334'890;

    using IntType = std::remove_const_t<decltype(expected)>;
    static_assert(std::is_integral<IntType>{});

    auto const total = ::PE_043_6th_digit<IntType>("123456789", '0')
                     + ::PE_043_6th_digit<IntType>("102346789", '5');

    REQUIRE(total == expected);
}

// PE 044 - Пятиугольные числа
namespace
{
    template <class IntType>
    bool is_pentagonal(IntType num)
    {
        assert(num >= 0);
        auto const x = (std::sqrt(1 + 24 * num) + 1)/6;

        return !(std::floor(x) < x);
    }

    struct pentagonal_number_fn
    {
        template <class IntType>
        constexpr IntType operator()(IntType num) const
        {
            assert(num >= 0);

            return 3 * saga::triangular_number(num) - 2 * num;
        }
    };

    inline constexpr auto pentagonal_number = ::pentagonal_number_fn{};

    template <class IntType>
    IntType PE_044()
    {
        std::vector<IntType> pentagonals;

        pentagonals.push_back(::pentagonal_number(1));

        for(;;)
        {
            // Генерируем новое число
            auto new_value = ::pentagonal_number(IntType(pentagonals.size() + 1));

            // Сравниваем со всеми предыдущими
            auto pred = [&new_value](IntType const & value)
            {
                assert(value <= new_value);

                return ::is_pentagonal(new_value + value) && ::is_pentagonal(new_value - value);
            };

            auto pos = saga::find_if(saga::cursor::all(pentagonals) | saga::cursor::reverse, pred);

            if(!!pos)
            {
                return new_value - *pos;
            }

            // Если не нашли нужное, то добавляем это число к списку
            pentagonals.push_back(std::move(new_value));
        }
    }
}
TEST_CASE("PE 044")
{
    REQUIRE(::pentagonal_number(0) == 0);
    REQUIRE(::pentagonal_number(1) == 1);
    REQUIRE(::pentagonal_number(4) == 22);
    REQUIRE(::pentagonal_number(7) == 70);
    REQUIRE(::pentagonal_number(8) == 92);
    REQUIRE(::pentagonal_number(9) == 117);
    REQUIRE(::pentagonal_number(10) == 145);

    REQUIRE(::is_pentagonal(22));
    REQUIRE(::is_pentagonal(70));
    REQUIRE(::is_pentagonal(92));
    REQUIRE(!::is_pentagonal(48));

    REQUIRE(::PE_044<long>() == 5482660);
}

// PE 045 - Треугольное, пятиугольное и шестиугольное
namespace
{
    struct hexagonal_number_fn
    {
        template <class IntType>
        constexpr IntType operator()(IntType num) const
        {
            return num * (2*num - 1);
        }
    };

    inline constexpr auto hexagonal_number = hexagonal_number_fn{};

    static_assert(saga::triangular_number(285) == 40755);
    static_assert(::pentagonal_number(165) == 40755);
    static_assert(::hexagonal_number_fn{}(143) == 40755);
}

TEST_CASE("PE 045")
{
    REQUIRE(::is_triangular_number_fn{}(40755));
    REQUIRE(::is_pentagonal(40755));

    auto pred = [](auto const & num)
        { return ::is_pentagonal(num) && ::is_triangular_number_fn{}(num); };

    auto cur = saga::cursor::iota(std::int64_t(1))
             | saga::cursor::transform(::hexagonal_number_fn{})
             | saga::cursor::filter(pred);

    REQUIRE(cur.front() == 1);
    cur.drop_front();

    REQUIRE(cur.front() == 40755);
    cur.drop_front();

    REQUIRE(cur.front() == 1'533'776'805);
}

// PE 046 - Ещё одна гипотеза Гольдбаха
namespace
{
    template <class IntType, class Container>
    bool is_prime_plus_twice_square(IntType num, Container primes)
    {
        auto pred = [&](auto const & prime)
        {
            assert(num > prime);

            auto const d = std::sqrt((num - prime) / 2);

            return !(std::floor(d) < d);
        };

        return saga::any_of(saga::cursor::all(primes), pred);
    }
}

TEST_CASE("PE 046")
{
    using IntType = long;

    std::vector<IntType> primes{2};

    auto num = IntType{3};

    for(;; num += 2)
    {
        if(saga::is_prime_sorted(num, primes, saga::unsafe_tag_t{}))
        {
            primes.push_back(num);
        }
        else if(!::is_prime_plus_twice_square(num, primes))
        {
            break;
        }
    }

    REQUIRE(num == 5777);
}

// PE 047 - Различные простые делители
namespace
{
    template <class IntType, class Cursor>
    saga::cursor_difference_t<Cursor>
    count_prime_divisors_sorted(IntType num, Cursor cur)
    {
        SAGA_ASSERT_AUDIT(saga::is_sorted(cur));

        auto result = saga::cursor_difference_t<Cursor>(0);

        for(; !!cur && saga::square(*cur) <= num; ++cur)
        {
            auto remove_result = saga::remove_factor(num, *cur);

            num = remove_result.value;
            result += (remove_result.multiplicity > 0);
        }

        // Может быть не более одного простого делителя больше sqrt(num)
        return result + (num != 1);
    }

    template <class IntType>
    IntType PE_047(IntType const required)
    {
        assert(required > 0);

        std::vector<IntType> primes;

        auto hits = IntType(0);

        auto cur = saga::cursor::iota(2);

        for(; hits < required; ++ cur)
        {
            auto const d_count = ::count_prime_divisors_sorted(*cur, saga::cursor::all(primes));

            if(d_count == 1)
            {
                primes.push_back(*cur);
            }

            hits = (d_count == required) ? hits + 1 : IntType(0);
        }

        return *cur - required;
    }
}

TEST_CASE("PE 047")
{
    REQUIRE(::PE_047(2) == 14);
    REQUIRE(::PE_047(3) == 644);
    REQUIRE(::PE_047(4) == 134043);
}

// PE 048 - Собственные степени
namespace
{
    template <class Integer, class IntType
             , class BinaryOperation1 = std::multiplies<>
             , class BinaryOperation2 = std::plus<>>
    Integer PE_048_self_powers_sum(IntType max_num, BinaryOperation1 prod = {}
                                  , BinaryOperation2 add = {})
    {
        auto self_power = [&](IntType const & num)
        {
            assert(num >= 1);

            return saga::power_natural(Integer(num), num, prod);
        };

        auto cur = saga::cursor::indices(1, max_num + 1) | saga::cursor::transform(self_power);

        return saga::reduce(std::move(cur), {}, add);
    }
}

TEST_CASE("integer10 : mod10")
{
    ::integer10 num("987654321");
    num.mod10(3);

    REQUIRE(num == 321);
}

TEST_CASE("PE 048")
{
    // Простой пример
    REQUIRE(::PE_048_self_powers_sum<long long>(10) == 10'405'071'317);

    // Пример, для которого недостаточно 64 бита
    using Integer = ::integer10;

    auto const digits_needed = 10;

    auto const mult_mod = [=](Integer lhs, Integer const & rhs)
    {
        lhs *= rhs;
        lhs.mod10(digits_needed);

        return lhs;
    };

    auto const add_mod = [=](Integer lhs, Integer const & rhs)
    {
        lhs += rhs;
        lhs.mod10(digits_needed);

        return lhs;
    };

    REQUIRE(::PE_048_self_powers_sum<Integer>(1000, mult_mod, add_mod) == 9'110'846'700);
}

// PE 049 - Простые перестановки
namespace
{
    std::vector<std::string> PE_049_prime_permutations(long last)
    {
        std::vector<std::string> result;

        auto const primes = saga::primes_below(last);

        for(auto second : saga::cursor::indices_of(primes))
        for(auto first  : saga::cursor::indices(second))
        {
            auto const str1 = std::to_string(primes[first]);
            auto const str2 = std::to_string(primes[second]);

            if(!saga::is_permutation(saga::cursor::all(str1), saga::cursor::all(str2)))
            {
                continue;
            }

            auto dif = primes[second] - primes[first];

            auto const prime3 = primes[second] + dif;
            auto const str3 = std::to_string(prime3);

            if(saga::is_permutation(saga::cursor::all(str3), saga::cursor::all(str1))
               && saga::binary_search(saga::cursor::all(primes), prime3))
            {
                result.push_back(str1 + str2 + str3);
            }
        }

        return result;
    }
}

TEST_CASE("PE 049")
{
    auto const answer = PE_049_prime_permutations(10'000);

    REQUIRE(answer.size() == 2);
    REQUIRE(answer[0] == "148748178147");
    REQUIRE(answer[1] == "296962999629");
}

// PE 050 - Сумма последовательных простых чисел
namespace
{
    template <class IntType>
    IntType PE_050(IntType last)
    {
        // Простые числа
        auto primes = saga::primes_below(last);
        assert(!primes.empty());

        // Накопленные суммы простых чисел
        std::vector<IntType> acc{0};
        saga::partial_sum(saga::cursor::all(primes), saga::back_inserter(acc));

        // Находим ограничение на количество слагаемых
        // Если первые n чисел дают в сумме более last, то и любые n идущих подряд простых
        // чисел дают в сумме более last
        auto const pos = saga::lower_bound(saga::cursor::all(acc), last);

        // Ищем простое число
        for(auto num = acc.size() - pos.size(); num > 1; -- num)
        {
            for(auto index = acc.size(); index > num; -- index)
            {
                auto const value = acc[index - 1] - acc[index - 1 - num];

                if(saga::binary_search(saga::cursor::all(primes), value))
                {
                    return value;
                }
            }
        }

        return primes.back();
    }
}

TEST_CASE("PE 050")
{
    REQUIRE(::PE_050(3) == 2);
    REQUIRE(::PE_050(100) == 41);
    REQUIRE(::PE_050(1'000) == 953);
    REQUIRE(::PE_050<long>(1'000'000) == 997651);
}

// PE 051 - Замена цифр в простых числах
namespace
{
    template <class Cursor>
    saga::cursor_difference_t<Cursor>
    PE_051_count(Cursor cur, std::string const & str, char old_digit, char first, char last)
    {
        saga::cursor_difference_t<Cursor> counter(0);

        for(auto new_digit : saga::cursor::indices(first, last))
        {
            auto new_str = str;
            saga::replace(saga::cursor::all(new_str), old_digit, new_digit);

            counter += (new_str != str) && saga::binary_search(cur, new_str);
        }

        return counter;
    }

    template <class Cursor>
    bool PE_051_check(Cursor cur, std::string const & str, std::ptrdiff_t family_size)
    {
        for(auto old_digit : saga::cursor::indices('0', '9' - family_size + 1))
        {
            if(::PE_051_count(cur.dropped_front(), str, old_digit, '0', old_digit) > 0)
            {
                continue;
            }

            if(::PE_051_count(cur, str, old_digit, old_digit + 1, '9' + 1) + 1 >= family_size)
            {
                return true;
            }
        }

        return false;
    }

    /**
    @pre first <= last
    @pre <tt> old_primes.empty() || old_primes.back() < first </tt>
    @pre Между primes.back() и first нет простых чисел
    @todo Можно ли унифицировать эту функцию с saga::copy_primes_below?
    */
    template <class IntType>
    std::vector<IntType>
    PE_051_primes(std::vector<IntType> const & old_primes, IntType first, IntType last)
    {
        assert(first <= last);
        assert(old_primes.empty() || old_primes.back() < first);
        assert(old_primes.empty() || (old_primes.front() == IntType(2)));
        assert(!old_primes.empty() || (first <= IntType(2)));

        if(first == last)
        {
            return {};
        }

        first += saga::is_even(first);

        // num = first + index * 2
        // index = (num - first) / 2
        std::vector<unsigned short> seive((last - first) / 2, true);

        auto const cur = saga::cursor::all(seive);

        // Просеиваем уже найденные простые
        for(auto prime : old_primes)
        {
            // Двойки мы пропускаем и так!
            if(prime == 2)
            {
                continue;
            }

            auto num = first + prime - (first % prime);

            num += prime * saga::is_even(num);

            saga::mark_eratosthenes_seive(saga::cursor::drop_front_n(cur, (num - first)/2)
                                          , prime);
        }

        // Просеиваем остальные простые
        for(auto index : saga::cursor::indices(cur.size()))
        {
            if(cur[index])
            {
                auto new_prime = first + 2*index;
                auto const square_index = (saga::square(new_prime) - first)/2;

                if(square_index >= cur.size())
                {
                    break;
                }

                saga::mark_eratosthenes_seive(saga::cursor::drop_front_n(cur, square_index)
                                              , new_prime);
            }
        }

        // Выделяем простые числа
        std::vector<IntType> primes;

        if(old_primes.empty())
        {
            primes.emplace_back(2);
        }

        for(auto index : saga::cursor::indices_of(seive))
        {
            if(seive[index])
            {
                primes.push_back(first + 2 * index);
            }
        }

        return primes;
    }

    template <class Cursor>
    Cursor PE_051_find(Cursor cur, std::ptrdiff_t const family_size)
    {
        for(; !!cur;)
        {
            auto cur_next = cur;
            ++ cur_next;

            if(::PE_051_check(cur_next, *cur, family_size))
            {
                break;
            }

            cur = cur_next;
        }

        return cur;
    }

    template <class IntType>
    std::string PE_051(std::ptrdiff_t const family_size)
    {
        std::vector<IntType> old_primes{};
        std::vector<IntType> primes = saga::primes_below(IntType(10));

        for(auto limit = IntType(10);; limit *= 10)
        {
            std::vector<std::string> primes_str;
            saga::transform(saga::cursor::all(primes), saga::back_inserter(primes_str)
                            , SAGA_OVERLOAD_SET(std::to_string));

            auto const cur = ::PE_051_find(saga::cursor::all(primes_str), family_size);

            if(!!cur)
            {
                return *cur;
            }

            old_primes.insert(old_primes.end(), primes.begin(), primes.end());
            primes = ::PE_051_primes(old_primes, limit, limit * 10);
        }
    }
}

TEST_CASE("PE_051_primes")
{
    REQUIRE(::PE_051_primes({}, 2, 2).empty());

    auto const primes1 = saga::primes_below(10);
    auto const primes2 = ::PE_051_primes({}, 2, 10);

    REQUIRE(primes1 == primes2);
}

TEST_CASE("primes multistep")
{
    auto const limit1 = 10;
    auto const limit2 = 1000;

    REQUIRE(limit1 <= limit2);

    // Все сразу
    auto const primes_all = saga::primes_below(limit2);

    // В два этапа
    auto const primes1 = saga::primes_below(limit1);

    auto const primes2 = ::PE_051_primes(primes1, limit1, limit2);

    REQUIRE(!primes2.empty());
    REQUIRE(saga::is_sorted(saga::cursor::all(primes2)));
    REQUIRE(primes2.front() >= limit1);
    REQUIRE(primes2.back() < limit2);

    // Сравниваем
    auto result = primes1;
    result.insert(result.end(), primes2.begin(), primes2.end());

    REQUIRE(result == primes_all);
}

TEST_CASE("PE 051")
{
    REQUIRE(::PE_051<long>(6) == "13");
    REQUIRE(::PE_051<long>(7) == "56003");
    REQUIRE(::PE_051<long>(8) == "121313");
}

// PE 052 - Перестановочные кратные
namespace
{
    template <class IntType>
    bool PE_052_check(IntType num, IntType max_mult)
    {
        auto const str_1 = std::to_string(num) | saga::actions::sort;

        for(auto mult : saga::cursor::indices(2, max_mult + 1))
        {
            auto const str_k = std::to_string(num * mult) | saga::actions::sort;

            if(str_1 != str_k)
            {
                return false;
            }
        }

        return true;
    }

    template <class IntType>
    IntType PE_052(IntType max_mult)
    {
        return saga::find_if(saga::cursor::iota(IntType(1))
                            , [&](IntType num){ return ::PE_052_check(num, max_mult); }).front();
    }
}

TEST_CASE("PE 052")
{
    REQUIRE(::PE_052<long>(6) == 142857);
}

// PE 053 - Комбинаторные сочетания
namespace
{
    template <class IntType>
    std::size_t PE_053(std::size_t n_max, IntType limit_value)
    {
        std::vector<IntType> row{1};

        auto result = std::size_t(0);

        saga::for_n(n_max, [&]
        {
            row.push_back(1);

            auto prev_value = row.front();

            for(auto index : saga::cursor::indices(1u, row.size()-1))
            {
                prev_value = saga::exchange(row[index]
                                            , std::move(row[index]) + std::move(prev_value));

                if(row[index] > limit_value)
                {
                    result += row.size() - 2*index;
                    row[index] = limit_value + 1;
                }
            }
        });

        return result;
    }
}

TEST_CASE("PE 053")
{
    using IntType = long;

    REQUIRE(::PE_053(100, IntType(1'000'000)) == 4075);
}

// PE 054 - Покерные комбинации
namespace
{
    struct poker_card
    {
        int value = 0;
        char suit = 0;
    };

    struct poker_hand
    {
        std::array<poker_card, 5> cards;
    };

    bool is_poker_flush(poker_hand const & hand)
    {
        auto pred = [suit = hand.cards.front().suit](poker_card const & card)
            { return card.suit == suit; };

        return saga::all_of(saga::cursor::all(hand.cards), pred);
    }

    bool is_poker_straight(poker_hand const & hand)
    {
        for(auto index : saga::cursor::indices(1, hand.cards.size()))
        {
            if(hand.cards[index-1].value != hand.cards[index].value + 1)
            {
                return false;
            }
        }

        return true;
    }

    bool less_by_highest(poker_hand const & lhs, poker_hand const & rhs)
    {
        auto const cmp = saga::compare_by(&poker_card::value, std::less<>{});

        return saga::lexicographical_compare(saga::cursor::all(lhs.cards)
                                            ,saga::cursor::all(rhs.cards), cmp);
    }

    struct poker_hand_summary
    {
        int four = 0;
        int triple = 0;
        int high_pair = 0;
        int low_pair = 0;
    };

    poker_hand_summary make_poker_hand_summary(poker_hand const & hand)
    {
        std::vector<int> freqs(13, 0);

        for(auto const & card : hand.cards)
        {
            freqs.at(card.value - 2) += 1;
        }

        poker_hand_summary result;

        for(auto index : saga::cursor::indices_of(freqs))
        {
            auto const value = index + 2;

            if(freqs[index] == 4)
            {
                result.four = value;
            }
            else if(freqs[index] == 3)
            {
                result.triple = value;
            }
            else if(freqs[index] == 2)
            {
                if(result.high_pair == 0)
                {
                    result.high_pair = value;
                }
                else
                {
                    result.low_pair = value;
                    if(result.low_pair > result.high_pair)
                    {
                        using std::swap;
                        swap(result.low_pair, result.high_pair);
                    }
                }
            }
        }

        return result;
    }

    bool operator<(poker_hand const & lhs, poker_hand const & rhs)
    {
        auto const lhs_flush = ::is_poker_flush(lhs);
        auto const rhs_flush = ::is_poker_flush(rhs);

        auto const lhs_straight = ::is_poker_straight(lhs);
        auto const rhs_straight = ::is_poker_straight(rhs);

        // Роял-флэш и Стрит-флэш
        if(lhs_flush && lhs_straight && rhs_flush && rhs_straight)
        {
            return less_by_highest(lhs, rhs);
        }
        if((lhs_flush && lhs_straight) || (rhs_flush && rhs_straight))
        {
            return (rhs_flush && rhs_straight);
        }

        auto const lhs_summary = make_poker_hand_summary(lhs);
        auto const rhs_summary = make_poker_hand_summary(rhs);

        // Четыре карты
        if(lhs_summary.four > 0 && rhs_summary.four > 0)
        {
            // Не может быть 8 карт одного номинала
            assert(lhs_summary.four != rhs_summary.four);

            return lhs_summary.four < rhs_summary.four;
        }
        else if(lhs_summary.four > 0 || rhs_summary.four > 0)
        {
            return rhs_summary.four > 0;
        }

        // Тройка+пара
        if(lhs_summary.triple > 0 && lhs_summary.high_pair > 0
           && rhs_summary.triple > 0 && rhs_summary.high_pair > 0)
        {
            // Не может быть 6 карт одного номинала
            assert(lhs_summary.triple != rhs_summary.triple);

            return lhs_summary.triple < rhs_summary.triple;
        }
        else if((lhs_summary.triple > 0 && lhs_summary.high_pair > 0)
                || (rhs_summary.triple > 0 && rhs_summary.high_pair > 0))
        {
            return (rhs_summary.triple > 0 && rhs_summary.high_pair > 0);
        }

        // Флэш
        if(lhs_flush && rhs_flush)
        {
            return less_by_highest(lhs, rhs);
        }
        if(lhs_flush || rhs_flush)
        {
            return rhs_flush;
        }

        // Стрит
        if(lhs_straight && rhs_straight)
        {
            return less_by_highest(lhs, rhs);
        }
        if(lhs_straight || rhs_straight)
        {
            return rhs_straight;
        }

        // Тройка
        if(lhs_summary.triple > 0 && rhs_summary.triple > 0)
        {
            // Не может быть 6 карт одного номинала
            assert(lhs_summary.triple != rhs_summary.triple);

            return lhs_summary.triple < rhs_summary.triple;
        }
        else if(lhs_summary.triple > 0 || rhs_summary.triple > 0)
        {
            return rhs_summary.triple > 0;
        }

        // Две пары
        if(lhs_summary.low_pair > 0 && rhs_summary.low_pair > 0)
        {
            assert(lhs_summary.high_pair > 0);
            assert(rhs_summary.high_pair > 0);

            if(lhs_summary.high_pair != rhs_summary.high_pair)
            {
                return lhs_summary.high_pair < rhs_summary.high_pair;
            }
            else if(lhs_summary.low_pair != rhs_summary.low_pair)
            {
                return lhs_summary.low_pair < rhs_summary.low_pair;
            }
            else
            {
                return less_by_highest(lhs, rhs);
            }
        }
        if(lhs_summary.low_pair > 0 || rhs_summary.low_pair > 0)
        {
            return rhs_summary.low_pair > 0;
        }

        // Пара
        if(lhs_summary.high_pair != rhs_summary.high_pair)
        {
            return lhs_summary.high_pair < rhs_summary.high_pair;
        }

        // Старшая карта
        return less_by_highest(lhs, rhs);
    }

    int poker_card_value(char arg)
    {
        if(arg == 'T')
        {
            return 10;
        }
        else if(arg == 'J')
        {
            return 11;
        }
        else if(arg == 'Q')
        {
            return 12;
        }
        else if(arg == 'K')
        {
            return 13;
        }
        else if(arg == 'A')
        {
            return 14;
        }
        else
        {
            assert(std::isdigit(arg));

            return int(arg - '0');
        }
    }

    poker_hand parse_poker_hand(std::string_view str)
    {
        assert(str.size() == 14);

        poker_hand hand;

        for(auto index : saga::cursor::indices_of(hand.cards))
        {
            hand.cards[index].value = poker_card_value(str[3*index]);
            hand.cards[index].suit  = str[3*index + 1];
        }

        hand.cards |= saga::actions::sort(saga::compare_by(&poker_card::value, std::greater<>{}));

        return hand;
    }

    bool PE_054_line(std::string const & two_hands)
    {
        assert(two_hands.size() >= 29);
        assert(two_hands[14] == ' ');

        auto const lhs = parse_poker_hand(std::string_view(two_hands).substr(0, 14));
        auto const rhs = parse_poker_hand(std::string_view(two_hands).substr(15, 14));

        return rhs < lhs;
    }

    std::size_t PE_054_file(std::string const & path)
    {
        std::ifstream file(path);

        assert(!!file);

        return saga::transform_reduce(saga::cursor::by_line(file), std::size_t(0)
                                     , std::plus<>{}, PE_054_line);
    }
}

TEST_CASE("PE 054")
{
    CHECK(::PE_054_line("5H 5C 6S 7S KD 2C 3S 8S 8D TD") == false);
    CHECK(::PE_054_line("5D 8C 9S JS AC 2C 5C 7D 8S QH") == true);
    CHECK(::PE_054_line("2D 9C AS AH AC 3D 6D 7D TD QD") == false);
    CHECK(::PE_054_line("4D 6S 9H QH QC 3D 6D 7H QD QS") == true);
    CHECK(::PE_054_line("2H 2D 4C 4D 4S 3C 3D 3S 9S 9D") == true);

    CHECK(::PE_054_line("3S 4S 5S 6S 7S 2D 3D 4D 5D 6D") == true);
    CHECK(::PE_054_line("3S 4S 5S 6S 7H 2D 3D 4D 5D 6D") == false);
    CHECK(::PE_054_line("2H 2C 2S 2D AH 3H 3C 3S 3D KH") == false);
    CHECK(::PE_054_line("2H 2C 2S 2D AH 3H 3C 3S KD KH") == true);
    CHECK(::PE_054_line("3S 4S 5S 6S 8S 2D 3D 4D 6D 8D") == true);
    CHECK(::PE_054_line("3S 4S 5D 6S 7S 2D 3D 4S 5D 6D") == true);
    CHECK(::PE_054_line("2H KD 4C 4D 4S 3C 3D 3S 9S QD") == true);
    CHECK(::PE_054_line("2H 2D 4C 4D JS 3C 3D TS 9S 9D") == false);
    CHECK(::PE_054_line("2H 2D 4C 4D JS 3C 3D TS 4S 4H") == false);
    CHECK(::PE_054_line("2H 2D 4C 4D JS 2C 2H TS 3S 3H") == true);
    CHECK(::PE_054_line("3H 3D 4C 4D QS 3S 3D TS 4S 4H") == true);

    CHECK(::PE_054_file("ProjectEuler/p054_poker.txt") == 376);
}

// PE 055 - Числа Лишрел
namespace
{
    template <class Container>
    void plus_assign_long_same_size(Container & lhs, Container const & rhs
                                    , typename Container::value_type base)
    {
        assert(lhs.size() == rhs.size());

        using Digit = typename Container::value_type;

        auto carry = Digit(0);
        for(auto index : saga::cursor::indices_of(lhs))
        {
            lhs[index] += (rhs[index] + carry);

            carry = lhs[index] / base;
            lhs[index] %= base;

            assert(0 <= carry && carry < base);
        }

        if(carry > 0)
        {
            lhs.push_back(carry);
        }
    }

    struct PE_055_is_Lychrel_fn
    {
        template <class IntType>
        bool operator()(IntType value) const
        {
            assert(value >= 0);

            using Digit = int;
            auto const base = Digit(10);

            auto num = saga::cursor::digits_of(value, base)
                     | saga::cursor::to<std::vector>();

            for(auto counter = 50; counter > 0; -- counter)
            {
                auto reversed = num;
                reversed |= saga::actions::reverse;

                ::plus_assign_long_same_size(num, reversed, base);

                if(saga::is_palindrome(saga::cursor::all(num)))
                {
                    return false;
                }
            }

            return true;
        }
    };
}
TEST_CASE("PE 055")
{
    using IntType = long;

    auto const n_max = IntType(10'000);

    REQUIRE(saga::count_if(saga::cursor::iota(IntType(1), n_max), ::PE_055_is_Lychrel_fn{}) == 249);
}

// PE 056 - Сумма цифр степени
TEST_CASE("PE 056")
{
    auto fun = [](auto const & arg)
    {
        return ::digits_sum(saga::power_natural(::integer10(std::get<0>(arg)), std::get<1>(arg)));
    };

    auto cur = saga::cursor::cartesian_product(saga::cursor::indices(1, 100)
                                               , saga::cursor::indices(1, 100))
             | saga::cursor::transform(fun)
             | saga::cursor::cached1;

    REQUIRE(saga::reduce(std::move(cur), 0L, SAGA_OVERLOAD_SET(std::max)) == 972);
}

// PE 057 - Подходящие цепные дроби квадратного корня
namespace
{
    template <class IntType>
    class convergent
    {
    public:
        explicit convergent(IntType num)
         : p_cur_(std::move(num))
         , q_cur_(1)
        {}

        IntType const & numerator() const
        {
            return this->p_cur_;
        }

        IntType const & denominator() const
        {
            return this->q_cur_;
        }

        void add(IntType const & arg)
        {
            this->update(this->p_cur_, std::move(this->p_prev_), arg);
            this->update(this->q_cur_, std::move(this->q_prev_), arg);
        }

    private:
        static void update(IntType & cur, IntType && prev, IntType const & arg)
        {
            prev = saga::exchange(cur, arg * std::move(cur) + std::move(prev));
        }

        IntType p_cur_ = IntType(0);
        IntType q_cur_ = IntType(0);

        IntType p_prev_ = IntType(1);
        IntType q_prev_ = IntType(0);
    };

    std::size_t digits_count(::integer10 const & num)
    {
        if(num.data().empty())
        {
            return 0;
        }

        return (num.data().size() - 1) * num.digits_per_unit
                + saga::cursor::size(saga::cursor::digits_of(num.data().back(), 10));
    }
}

TEST_CASE("integer10(0) digits_count")
{
    REQUIRE(::digits_count(::integer10()) == 0);
}

TEMPLATE_TEST_CASE("sqrt(2) convergents", "convergent", int, ::integer10)
{
    ::convergent<TestType> conv(TestType(1));

    REQUIRE(conv.numerator() == 1);
    REQUIRE(conv.denominator() == 1);

    conv.add(TestType(2));

    REQUIRE(conv.numerator() == 3);
    REQUIRE(conv.denominator() == 2);

    conv.add(TestType(2));

    REQUIRE(conv.numerator() == 7);
    REQUIRE(conv.denominator() == 5);

    conv.add(TestType(2));

    REQUIRE(conv.numerator() == 17);
    REQUIRE(conv.denominator() == 12);

    conv.add(TestType(2));

    REQUIRE(conv.numerator() == 41);
    REQUIRE(conv.denominator() == 29);

    conv.add(TestType(2));

    REQUIRE(conv.numerator() == 99);
    REQUIRE(conv.denominator() == 70);

    conv.add(TestType(2));

    REQUIRE(conv.numerator() == 239);
    REQUIRE(conv.denominator() == 169);

    conv.add(TestType(2));

    REQUIRE(conv.numerator() == 577);
    REQUIRE(conv.denominator() == 408);

    conv.add(TestType(2));

    REQUIRE(conv.numerator() == 1393);
    REQUIRE(conv.denominator() == 985);
}

TEST_CASE("PE 057")
{
    ::convergent<::integer10> conv(::integer10(1));

    std::size_t result = 0;

    for(auto steps = 1000; steps > 0; -- steps)
    {
        conv.add(::integer10(2));

        result += (::digits_count(conv.numerator()) > ::digits_count(conv.denominator()));
    }

    REQUIRE(result == 153);
}

// PE 058 - Спиральные простые числа
/* Пусть n - номер слоя
Его длина (если считать номер слоя с нуля): 2n+1
Угловые числа:
(2n+1)^2
(2n+1)^2 - 2n
(2n+1)^2 - 4n
(2n+1)^2 - 6n

Квадрат не может быть простым числом, поэтому первое из этих чисел можно не проверять

Длина диагоналей: 4 * n + 1 (на нулевом слое одно число, на остальных по 4)
*/

namespace
{
    template <class IntType>
    IntType PE_058(IntType numerator, IntType denominator)
    {
        std::vector<IntType> primes{2, 3};
        auto primes_limit = primes.back() + 1;

        auto primes_count = IntType(0);

        auto layer = IntType(1);
        for(;; ++ layer)
        {
            auto num = saga::square(2 * layer + 1);

            if(primes_limit < 2 * layer + 1)
            {
                auto new_primes = ::PE_051_primes(primes, primes_limit, saga::square(primes_limit));
                primes.insert(primes.end(), new_primes.begin(), new_primes.end());

                primes_limit = saga::square(primes_limit);
            }

            for(auto rep = 3; rep > 0; -- rep)
            {
                num -= 2 * layer;

                primes_count += saga::is_prime_sorted(num, primes, saga::unsafe_tag_t{});
            }

            auto const diagonal_length = 4 * layer + 1;

            if(primes_count * denominator < diagonal_length * numerator)
            {
                break;
            }
        }

        return 2 * layer + 1;
    }
}

TEST_CASE("PE 058")
{
    REQUIRE(::PE_058<long long>(1, 10) == 26241);
}

// PE 061 - Цикличные фигурные числа
namespace
{
    template <class UnaryFunction>
    std::vector<int>
    PE_061_prepare(UnaryFunction fun)
    {
        auto cur = saga::cursor::iota(1)
                 | saga::cursor::transform(std::move(fun));

        return saga::find_if(std::move(cur), [](int const & num) {return num >= 1000;})
               | saga::cursor::take_while([](int const & num) { return num < 10000; })
               | saga::cursor::to<std::vector>();
    }

    bool is_chained(int lhs, int rhs)
    {
        assert(1000 <= lhs && lhs < 10'000);
        assert(1000 <= rhs && rhs < 10'000);

        return lhs % 100 == rhs / 100;
    }

    int PE_061_recursion(std::vector<std::vector<int>> points
                         , std::vector<int> const & indices
                         , std::vector<int> & path)
    {
        assert(!path.empty());

        if(path.size() == indices.size())
        {
            if(::is_chained(path.back(), path.front()))
            {
                return saga::reduce(saga::cursor::all(path));
            }
            else
            {
                return 0;
            }
        }

        for(auto const & point : points[indices[path.size()]])
        {
            if(::is_chained(path.back(), point))
            {
                path.push_back(point);

                auto result = ::PE_061_recursion(points, indices, path);

                path.pop_back();

                if(result != 0)
                {
                    return result;
                }
            }
        }

        return 0;
    }

    int PE_061_start(std::vector<std::vector<int>> points
                     , std::vector<int> const & indices)
    {
        assert(!points.empty());

        std::vector<int> path;

        for(auto const & start : points[0])
        {
            path.push_back(start);

            auto result = PE_061_recursion(points, indices, path);

            if(result != 0)
            {
                return result;
            }

            path.pop_back();
        }

        return 0;
    }

    template <class... IntVector>
    int PE_061(IntVector const &... args)
    {
        std::vector<std::vector<int>> points{args...};

        auto indices = saga::cursor::indices(0, static_cast<int>(points.size()))
                     | saga::cursor::to<std::vector<int>>();

        auto tail = saga::cursor::all(indices);
        tail.drop_front();

        do
        {
            auto result = PE_061_start(points, indices);

            if(result != 0)
            {
                return result;
            }
        }
        while(saga::next_permutation(tail));

        return 0;
    }

    struct heptagonal_number_fn
    {
        template <class IntType>
        IntType operator()(IntType const & num) const
        {
            return 5 * saga::triangular_number(num) - 4 * num;
        }
    };

    inline constexpr auto heptagonal_number = heptagonal_number_fn{};

    struct octagonal_number_fn
    {
        template <class IntType>
        IntType operator()(IntType const & num) const
        {
            return num * (3*num - 2);
        }
    };

    inline constexpr auto octagonal_number = octagonal_number_fn{};
}

TEST_CASE("PE 061")
{
    auto const nums_3 = ::PE_061_prepare(saga::triangular_number);
    auto const nums_4 = ::PE_061_prepare(saga::square);
    auto const nums_5 = ::PE_061_prepare(::pentagonal_number);
    auto const nums_6 = ::PE_061_prepare(::hexagonal_number);
    auto const nums_7 = ::PE_061_prepare(::heptagonal_number);
    auto const nums_8 = ::PE_061_prepare(::octagonal_number);

    CAPTURE(nums_3, nums_4, nums_5, nums_6, nums_7, nums_8);

    REQUIRE(PE_061(nums_4) == 0);
    REQUIRE(PE_061(nums_3, nums_4, nums_5) == 8128 + 2882 + 8281);
    REQUIRE(PE_061(nums_3, nums_4, nums_5, nums_6, nums_7, nums_8) == 28684);
}

// PE 062 - Кубические перестановки
namespace
{
    template <class IntType>
    IntType PE_062(std::size_t const repeats)
    {
        assert(repeats > 0);

        std::map<std::string, std::pair<IntType, std::size_t>> data;

        auto current_length = std::size_t(1);

        for(auto const & number : saga::cursor::iota(IntType(1)))
        {
            if(number >= std::numeric_limits<IntType>::max() / number / number)
            {
                break;
            }

            auto const cube = saga::power_natural(number, 3);
            auto const str = std::to_string(cube) | saga::actions::sort;

            if(str.size() > current_length)
            {
                auto pred = [&](auto const & item) { return item.second.second == repeats; };
                auto pos = saga::find_if(saga::cursor::all(data), pred);

                if(!!pos)
                {
                    return pos.front().second.first;
                }

                data.clear();
                ++ current_length;
            }

            auto & ref = data[str];

            if(ref.second == 0)
            {
                ref.first = cube;
            }

            ref.second += 1;
        }

        throw std::range_error("PE 062 : answer is not found");
    }
}

TEST_CASE("PE 062")
{
    using IntType = std::uint64_t;

    REQUIRE(::PE_062<IntType>(3) == 41063625);
    REQUIRE(::PE_062<IntType>(5) == 127035954683);

    REQUIRE(::PE_062<IntType>(6) == 1000600120008);

    REQUIRE_THROWS(::PE_062<int>(5));
}

// PE 063 - Количество цифр степеней
/* Число 10^n содержит n+1 цифр, поэтому основания больше 9 проверять смысла нет.
 Степени n числа b, для которых уже нет смысла проверять, определяется неравенством b^n < 10^(n-1).
 (n-1) > n * log10(b)
 n(1-log10(b)) > 1
 n > 1/(1-log10(b))

 Так как b^1 == b, то при n от 1 до 1/(1-log10(b)) числа b^n содержат n цифр
*/
TEST_CASE("PE 063")
{
    auto max_power = [](auto num){ return static_cast<std::size_t>(1/(1 - std::log10(num)));};

    auto const result = saga::transform_reduce(saga::cursor::indices(1, 10), std::size_t(0)
                                               , std::plus<>{}, max_power);
    REQUIRE(result == 49);
}

// PE 064 - Квадратные корни с нечётным периодом
namespace
{
    template <class IntType>
    struct PE_64_state
    {
        IntType shift = IntType(0);
        IntType value = IntType(0);
        IntType denominator = IntType(1);

        friend bool operator==(PE_64_state const & lhs, PE_64_state const & rhs)
        {
            return std::tie(lhs.shift, lhs.value, lhs.denominator)
                   == std::tie(rhs.shift, rhs.value, rhs.denominator);
        }
    };

    template <class IntType>
    class PE_64_cursor
     : saga::cursor_facade<PE_64_cursor<IntType>, PE_64_state<IntType> const &>
    {
    public:
        // Типы
        using reference = PE_64_state<IntType> const &;
        using difference_type = std::ptrdiff_t;
        using cursor_category = std::input_iterator_tag;

        // Конструктор
        /**
        @pre number >= 0
        */
        explicit PE_64_cursor(IntType number)
         : number_(std::move(number))
         , sqrt_floor_(std::sqrt(this->number_))
         , state_{this->sqrt_floor_, this->sqrt_floor_, 1}
        {}

        // Курсор ввода
        bool operator!() const
        {
            return this->state_.denominator == 0;
        }

        reference front() const
        {
            return this->state_;
        }

        void drop_front()
        {
            auto denom = this->number_ - saga::square(this->state_.shift);

            assert(this->state_.denominator != 0);
            assert(saga::is_divisible_by(denom, this->state_.denominator));

            this->state_.denominator = std::move(denom) / this->state_.denominator;

            if(this->state_.denominator == 0)
            {
                return;
            }

            this->state_.value = (this->sqrt_floor_ + this->state_.shift)
                               / this->state_.denominator;

            this->state_.shift = this->state_.value * this->state_.denominator
                               - std::move(this->state_.shift);

            assert(this->state_.shift > 0);
        }

    private:
        IntType number_;
        IntType sqrt_floor_;
        PE_64_state<IntType> state_;
    };

    template <class IntType>
    std::ptrdiff_t square_root_cycle_length(IntType num)
    {
        return ::cycle_length(::PE_64_cursor<IntType>(std::move(num)));
    }

    template <class IntType>
    std::size_t PE_064(IntType num)
    {
        auto pred = [](auto const & arg) { return saga::is_odd(::square_root_cycle_length(arg)); };

        return saga::count_if(saga::cursor::indices(IntType(2), num + 1), pred);
    }
}

TEST_CASE("PE 064")
{
    REQUIRE(::square_root_cycle_length(2) == 1);
    REQUIRE(::square_root_cycle_length(3) == 2);
    REQUIRE(::square_root_cycle_length(4) == 0);
    REQUIRE(::square_root_cycle_length(5) == 1);
    REQUIRE(::square_root_cycle_length(6) == 2);
    REQUIRE(::square_root_cycle_length(7) == 4);
    REQUIRE(::square_root_cycle_length(8) == 2);
    REQUIRE(::square_root_cycle_length(10) == 1);
    REQUIRE(::square_root_cycle_length(11) == 2);
    REQUIRE(::square_root_cycle_length(12) == 2);
    REQUIRE(::square_root_cycle_length(13) == 5);

    REQUIRE(::PE_064(13) == 4);
    REQUIRE(::PE_064(10'000) == 1322);
}

// PE 065 - Подходящие цепные дроби числа e
namespace
{
    template <class IntType>
    class cursor_for_continued_fraction_for_e
     : saga::cursor_facade<cursor_for_continued_fraction_for_e<IntType>, IntType const &>
    {
    public:
        // Типы
        using cursor_category = std::input_iterator_tag;
        using value_type = IntType;
        using reference = IntType const &;

        // Создание, копирование, уничтожение
        cursor_for_continued_fraction_for_e() = default;

        // Курсор ввода
        bool operator!() const
        {
            return false;
        }

        reference front() const
        {
            return this->value_;
        }

        void drop_front()
        {
            ++ index;

            this->value_ = (index % 3 == 2) ? IntType(2 * ((index+1) / 3)) : IntType(1);
        }

    private:
        IntType value_ = IntType(2);
        std::size_t index = 0;
    };

    std::size_t PE_065(std::size_t num)
    {
        auto cur = ::cursor_for_continued_fraction_for_e<::integer10>{}
                 | saga::cursor::take(num);

        ::convergent<::integer10> conv(*cur);
        ++ cur;

        for(; !!cur; ++ cur)
        {
            conv.add(*cur);
        }

        return ::digits_sum(conv.numerator());
    }
}

TEST_CASE("continued fraction of e")
{
    using IntType = int;

    std::vector<IntType> const expected{2, 1, 2, 1, 1, 4, 1, 1, 6, 1, 1, 8, 1, 1, 10, 1};

    auto const actual = ::cursor_for_continued_fraction_for_e<IntType>()
                      | saga::cursor::take(expected.size())
                      | saga::cursor::to<std::vector>();

    REQUIRE(actual == expected);
}

TEST_CASE("PE 065")
{
    REQUIRE(::PE_065(10) == 17);
    REQUIRE(::PE_065(100) == 272);
}

// PE 066 - Диофантово уравнение
namespace
{
    template <class IntType>
    std::pair<IntType, std::vector<IntType>>
    sqrt_continued_fraction_period(IntType number)
    {
        auto cur = ::PE_64_cursor<IntType>(number)
                 | saga::cursor::transform(&PE_64_state<IntType>::value);

        assert(!!cur);

        auto const first = *cur;
        ++ cur;

        auto const pred = [stop = 2 * first](IntType const & arg){ return arg != stop; };

        auto other = cur
                   | saga::cursor::take_while(pred)
                   | saga::cursor::to<std::vector>();

        return {first, other};
    }

    template <class Result, class IntType>
    Result pells_equation_minimal_solution_x(IntType number)
    {
        auto const sqrt_cf = ::sqrt_continued_fraction_period(number);

        ::convergent<Result> conv(sqrt_cf.first);

        for(auto const & each : saga::cursor::all(sqrt_cf.second))
        {
            conv.add(each);
        }

        if(saga::is_even(sqrt_cf.second.size()))
        {
            conv.add(2*sqrt_cf.first);

            for(auto const & each : saga::cursor::all(sqrt_cf.second))
            {
                conv.add(each);
            }
        }

        return conv.numerator();
    }

    template <class IntType>
    IntType pells_equation_minimal_solution_x(IntType number)
    {
        return pells_equation_minimal_solution_x<IntType, IntType>(number);
    }

    template <class IntType>
    IntType PE_066(IntType max_number)
    {
        assert(max_number >= 2);

        auto cur = saga::cursor::indices(IntType(2), max_number+1)
                 | saga::cursor::filter(saga::not_fn(saga::is_square))
                 | saga::cursor::transform(::pells_equation_minimal_solution_x<double, IntType>)
                 | saga::cursor::cached1;

        assert(!!cur);

        return *saga::max_element(cur).base().base();
    }
}

TEST_CASE("PE 066")
{
    CHECK(::pells_equation_minimal_solution_x(2) == 3);
    CHECK(::pells_equation_minimal_solution_x(3) == 2);
    CHECK(::pells_equation_minimal_solution_x(5) == 9);

    CHECK(::pells_equation_minimal_solution_x(6) == 5);
    CHECK(::pells_equation_minimal_solution_x(7) == 8);

    CHECK(::pells_equation_minimal_solution_x(13) == 649);

    CHECK(::PE_066(7) == 5);
    CHECK(::PE_066(1000) == 661);
}

// PE 067: Путь наибольшей суммы (часть II)
TEST_CASE("PE 067")
{
    std::ifstream file("ProjectEuler/p067_triangle.txt");

    REQUIRE(!!file);

    REQUIRE(::projectEuler_018<int>(file) == 7273);
}

// PE 068 - Магическое 5-угольное кольцо
namespace
{
    using PE_068_configuration = std::vector<std::tuple<int, int, int>>;

    auto PE_068_group(std::vector<int> const & numbers)
    {
        assert(!numbers.empty());
        assert(saga::is_even(numbers.size()));

        auto const length = numbers.size() / 2;

        PE_068_configuration groups;

        for(auto index : saga::cursor::indices(length))
        {
            groups.emplace_back(numbers[index + length], numbers[index]
                                , numbers[(index + 1) % length]);
        }

        saga::rotate(saga::min_element(saga::cursor::all(groups)));

        return groups;
    }

    bool is_magical(PE_068_configuration const & groups)
    {
        auto fun = [](auto const & group)
        {
            return std::get<0>(group) + std::get<1>(group) + std::get<2>(group);
        };

        auto value = fun(groups.front());

        return saga::all_of(saga::cursor::all(groups)
                            , [&](auto const & group) { return fun(group) == value; });
    }

    std::string PE_068(std::size_t const num, bool keep_last)
    {
        std::vector<int> digits(2*num);
        saga::iota(saga::cursor::all(digits), 1);

        std::string result;

        auto cur = saga::cursor::all(digits);

        if(keep_last)
        {
            cur.drop_back();
        }

        do
        {
            auto const groups = ::PE_068_group(digits);

            if(::is_magical(groups))
            {
                std::ostringstream str;

                for(auto const & group : groups)
                {
                    str << std::get<0>(group)
                        << std::get<1>(group)
                        << std::get<2>(group);
                }

                result = std::max(result, str.str());
            }
        }
        while(saga::next_permutation(cur));

        return result;
    }
}

TEST_CASE("PE 068")
{
    REQUIRE(PE_068(3, false) == "432621513");
    // Последний можно не переставлять (для 5-угольника)!
    // Если 10 будет на втуреннем круге, то количество цифр будет 17, а не 16!
    REQUIRE(PE_068(5, true) == "6531031914842725");
}

// PE 069 - Максимум функции Эйлера
namespace
{
    template <class Quotient, class IntType>
    IntType PE_069_scan(IntType const n_max)
    {
        auto const phi = saga::euler_phi_below(n_max);
        assert(phi.size() == static_cast<size_t>(n_max));
        assert(phi[0] == 0);

        std::vector<Quotient> n_over_phi;

        saga::transform(saga::cursor::indices(IntType(1), n_max)
                       ,saga::cursor::drop_front_n(saga::cursor::all(phi), 1)
                       ,saga::back_inserter(n_over_phi)
                       ,std::divides<Quotient>{});

        return saga::max_element(saga::cursor::all(n_over_phi)).dropped_front().size() + 1;
    }

    template <class IntType>
    IntType PE_069_smart(IntType const n_max)
    {
        auto cur = saga::primes_cursor<IntType>();

        auto result = IntType(1);

        for(;; ++ cur)
        {
            auto new_result = result * cur->back();

            if(new_result > n_max)
            {
                break;
            }

            result = std::move(new_result);
        }

        return result;
    }
}

TEST_CASE("PE 069")
{
    REQUIRE(PE_069_scan<double>(10) == 6);
    REQUIRE(PE_069_scan<double>(1'000'000) == 510'510);

    REQUIRE(PE_069_smart(10) == 6);
    REQUIRE(PE_069_smart(1'000'000) == 510'510);
}

// PE 070 - Перестановка функции Эйлера
/* Чтобы n/phi(n) было как можно меньше, phi(p) должно быть как можно больше.
Чем больше простых множителей, тем меньше phi(p).
Для p простого phi(p)=p-1 не может быть перестановкой
Рассмотрим всевозможные пары
*/
TEST_CASE("PE 070")
{
    auto const n_max = 10'000'000;

    auto const primes = saga::primes_below(n_max / 2 + 1);

    auto best_num = 2;
    auto best_phi = 1;

    for(auto pos1 : saga::cursor::indices_of(primes))
    for(auto pos2 : saga::cursor::indices(0, pos1))
    {
        if(primes[pos2] > n_max / primes[pos1])
        {
            break;
        }

        auto const num = primes[pos1] * primes[pos2];
        auto const phi = (primes[pos1] - 1) * (primes[pos2] - 1);

        auto const num_s = std::to_string(num);
        auto const phi_s = std::to_string(phi);

        if(saga::is_permutation(saga::cursor::all(num_s), saga::cursor::all(phi_s))
           && (double(num) / phi < double(best_num) / best_phi))
        {
            best_num = num;
            best_phi = phi;
        }
    }

    REQUIRE(best_num == 8319823);
}

// PE 071 - Упорядоченные дроби
namespace
{
    /* Перебор осуществляется в обратном направлении, чтобы можно было сократить перебор.
    Пусть a/b > r/s. Тогда a/b - r/s = (as-rb)/(bs) >= 1/(bs).
    Пусть a/b - данная дробь и к текущему моменту мы нашли дробь p/q. Найдём ограничение на
    возможную дробь r/s: p/q < r/s < a/b
    a/b - p/q = (aq - bp)/(bq)
    С другой стороны:
    a/b - p/q = a/b - r/s + r/s - p/q = (as - br)/(bs) + (rq - ps)/(sq) >= 1/(bs) + 1/(sq)
    Таким обарзом:
    (aq - bp)/(bq) = a/b - p/q >= (b+q)/(bsq)
    (aq - bp) >= (b+q)/s
    s >= (b+q)/(aq - bp)
    */
    template <class IntType>
    IntType PE_071(IntType const & max_denom, IntType const & num, IntType const & denom)
    {
        assert(std::gcd(num, denom) == 1);

        auto result_num = IntType(0);
        auto result_denom = IntType(1);

        auto min_denom = IntType(1);

        for(auto cur_denom = max_denom; cur_denom >= min_denom; -- cur_denom)
        {
            auto const cur_num = (num * cur_denom - 1)/ denom;

            auto const g = std::gcd(cur_num, cur_denom);

            if(cur_denom / g != denom)
            {
                if(cur_num / g * result_denom > cur_denom / g * result_num)
                {
                    result_num = cur_num / g;
                    result_denom = cur_denom / g;

                    min_denom = (denom + cur_denom) / (num * cur_denom - cur_num * denom);
                }
            }
        }

        return result_num;
    }
}

TEST_CASE("PE 071")
{
    REQUIRE(::PE_071(8, 3, 7) == 2);
    REQUIRE(::PE_071<std::int64_t>(1'000'000, 3, 7) == 428570);
}

// PE 072 - Подсчёт дробей
namespace
{
    /* Число правильных дробей для каждого знаменателя равно функции Эйлера от этого знаменателя.
    1/1 = 1 - не является правильно дробью
    */
    template <class IntType>
    IntType PE_072(IntType const max_denom)
    {
        auto const phi = saga::euler_phi_below(max_denom + 1);

        return saga::reduce(saga::cursor::all(phi)) - 1;
    }
}

TEST_CASE("PE 072")
{
    REQUIRE(::PE_072(8) == 21);
    REQUIRE(::PE_072(std::int64_t{1'000'000}) == 303'963'552'391);
}

// PE 073 - Подсчёт дробей в интервале
namespace
{
    template <class IntType>
    IntType PE_073(IntType max_denom, IntType num1, IntType denom1, IntType num2, IntType denom2)
    {
        assert(num2 * denom1 - num1 * denom2 == 1);

        auto result = IntType(0);

        std::vector<IntType> denoms;

        for(;;)
        {
            auto med = denom1 + denom2;

            if(med > max_denom)
            {
                if(denoms.empty())
                {
                    break;
                }
                else
                {
                    denom1 = denom2;
                    denom2 = denoms.back();
                    denoms.pop_back();
                }
            }
            else
            {
                result += 1;
                denoms.push_back(denom2);
                denom2 = med;
            }
        }

        return result;
    }
}

TEST_CASE("PE 073")
{
    REQUIRE(::PE_073(8, 1, 3, 1, 2) == 3);
    REQUIRE(::PE_073(12'000, 1, 3, 1, 2) == 7'295'372);
}

// PE 074 - Цепи факториалов цифр
namespace
{
    template <class IntType>
    std::vector<IntType>
    PE_074_lengths(IntType n_max)
    {
        std::vector<IntType> lengths(n_max, -1);

        for(auto const & cur_num : saga::cursor::indices(n_max))
        {
            if(lengths[cur_num] != -1)
            {
                continue;
            }

            // Строим орбиту
            std::vector<IntType> history;

            auto num = cur_num;
            auto repeat_index = history.size();

            for(;; num = ::digits_factorial_sum<IntType>(num))
            {
                if(num < n_max && lengths[num] != -1)
                {
                    break;
                }

                auto pos = saga::find(saga::cursor::all(history), num);
                if(!!pos)
                {
                    repeat_index = pos.dropped_front().size();
                    break;
                }

                history.push_back(num);
            }

            if(num < n_max && lengths[num] != -1)
            {
                auto cur_length = lengths[num] + 1;
                for(; !history.empty(); history.pop_back(), ++ cur_length)
                {
                    if(history.back() < n_max)
                    {
                        lengths[history.back()] = cur_length;
                    }
                }
            }
            else
            {
                for(auto index : saga::cursor::indices_of(history))
                {
                    if(history[index] < n_max)
                    {
                        lengths[history[index]] = (index < repeat_index)
                                                ? history.size() - index
                                                : history.size() - repeat_index;
                    }
                }
            }
        }

        return lengths;
    }
}

TEST_CASE("PE 074")
{
    using IntType = int;

    auto const n_max = IntType(1'000'000);

    auto const lengths = ::PE_074_lengths(n_max);

    REQUIRE(lengths[0] == 1);
    REQUIRE(lengths[145] == 1);
    REQUIRE(lengths[169] == 3);
    REQUIRE(lengths[871] == 2);
    REQUIRE(lengths[872] == 2);
    REQUIRE(lengths[69] == 5);

    REQUIRE(*saga::max_element(saga::cursor::all(lengths)) == 60);
    REQUIRE(saga::count(saga::cursor::all(lengths), 60) == 402);
}

// PE 075 - Особые целочисленные прямоугольные треугольники
TEST_CASE("PE 075")
{
    using IntType = std::int64_t;

    auto const L_max = IntType(1'500'000);

    std::vector<IntType> counter(L_max + 1, 0);

    auto const n_max = IntType(std::sqrt(L_max / 2));

    for(auto n_cur : saga::cursor::indices(IntType(2), n_max))
    {
        auto const m_max = std::min(n_cur, L_max / 2 / n_cur - n_cur + 1);

        for(auto m_cur = 1 + (n_cur % 2); m_cur < m_max; m_cur += 2)
        {
            if(std::gcd(n_cur, m_cur) == 1)
            {
                auto const L_cur = 2 * n_cur * (m_cur + n_cur);

                for(auto L = L_cur; L <= L_max; L += L_cur)
                {
                    counter[L] += 1;
                }
            }
        }
    }

    REQUIRE(counter.at(20) == 0);
    REQUIRE(counter.at(120) == 3);

    REQUIRE(saga::count(saga::cursor::all(counter), 1) == 161667);
}

// PE 076 - Подсчёт композиций
namespace
{
    template <class IntType>
    IntType summations_count(IntType number)
    {
        return summations_count(std::move(number), saga::cursor::indices(IntType(1), number));
    }
}

TEST_CASE("PE 076")
{
    REQUIRE(::summations_count(5) == 6);
    REQUIRE(::summations_count(std::int64_t(100)) == 190569291);
}

// PE 077 - Композиций простых чисел
namespace
{
    template <class IntType>
    IntType PE_077(IntType limit)
    {
        auto primes_cur = saga::primes_cursor<IntType>();

        // ways[i][j] количество способов представить число j в виде суммы первых i простых чисел
        // ways[i][j] = ways[i-1][j] + ways[i][j - prime[i-1]]
        std::vector<std::vector<IntType>> ways;

        // В виде нуля первых простых чисел можно представить только ноль
        ways.emplace_back(1, 1);

        for(auto num = IntType(1); ways.back().back() <= limit; ++ num)
        {
            ways.front().push_back(0);

            for(auto index : saga::cursor::indices(1u, ways.size()))
            {
                ways[index].push_back(ways[index-1].back());

                auto const prime = primes_cur.front()[index - 1];

                if(num >= prime)
                {
                    ways[index].back() += ways[index][num - prime];
                }
            }

            if(num == primes_cur->back())
            {
                ways.emplace_back(ways.back());
                ways.back().back() += 1;

                ++ primes_cur;
            }
        }

        return ways.back().size() - 1;
    }
}

TEST_CASE("PE 077")
{
    using IntType = std::int64_t;

    REQUIRE(::summations_count(IntType(10), saga::primes_below(IntType(10))) == 5);

    REQUIRE(PE_077(IntType(5'000)) == 71);
}

// PE 078 - Разбиение монет
namespace
{
    template <class IntType>
    IntType PE_078_euler()
    {
        std::vector<IntType> ways{1};

        for(auto num = IntType(1); ways.back() != 0; ++ num)
        {
            ways.emplace_back(0);

            for(auto k = IntType(1);; ++ k)
            {
                auto const m1 = num - k * (3 * k - 1) / 2;
                auto const m2 = num - k * (3 * k + 1) / 2;

                auto const sign = saga::is_even(k) ? -1 : 1;

                if(m1 >= 0)
                {
                    ways.back() += sign * ways[m1];
                }

                if(m2 >= 0)
                {
                    ways.back() += sign * ways[m2];
                }

                if(m1 < 0 && m2 < 0)
                {
                    break;
                }
            }

            ways.back() %= 1'000'000;
        }

        assert(!ways.empty());
        return ways.size() - 1;
    }
}

TEST_CASE("PE 078")
{
    using IntType = std::int64_t;

    REQUIRE(::PE_078_euler<IntType>() == 55374);
}

// PE 081 - Суммы по маршрутам: два варианта
namespace
{
    template <class IntType>
    IntType path_sum_two_ways(std::vector<std::vector<IntType>> table)
    {
        assert(!table.empty());
        auto const n_rows = table.size();
        auto const n_cols = table.front().size();

        saga::partial_sum(saga::cursor::all(table.front())
                         ,saga::cursor::all(table.front()));

        for(auto row : saga::cursor::indices(1, n_rows))
        {
            table[row].front() += table[row-1].front();

            assert(table[row].size() == n_cols);

            for(auto col : saga::cursor::indices(1, n_cols))
            {
                table[row][col] += std::min(table[row - 1][col], table[row][col - 1]);
            }
        }

        return table.back().back();
    }

    template <class IntType, class IStream>
    std::vector<IntType>
    read_csv_line(IStream & input)
    {
        std::vector<IntType> result;

        auto num = IntType();
        input >> num;

        for(; !!input;)
        {
            result.push_back(std::move(num));

            auto reader = input.get();

            if(!input || reader == '\n')
            {
                break;
            }

            assert(reader == ',');

            input >> num;
        }


        return result;
    }

    template <class IntType, class IStream>
    std::vector<std::vector<IntType>>
    read_csv(IStream & input)
    {
        std::vector<std::vector<IntType>> table;

        for(; !!input;)
        {
            auto row = ::read_csv_line<IntType>(input);

            if(!row.empty())
            {
                table.push_back(std::move(row));
            }
        }

        return table;
    }
}

TEST_CASE("PE 081")
{
    {
        std::vector<std::vector<int>> const table_1
        {
             {131, 673, 234, 103, 18}
            ,{201, 96,  342, 965, 150}
            ,{630, 803, 746, 422, 111}
            ,{537, 699, 497, 121, 956}
            ,{805, 732, 624, 37, 331}
        };

        REQUIRE(path_sum_two_ways(table_1) == 2427);
    }
    {
        std::ifstream file("ProjectEuler/p081_matrix.txt");

        REQUIRE(file);

        auto const table_2 = ::read_csv<int>(file);

        REQUIRE(table_2.size() == 80);
        REQUIRE(table_2.front().size() == 80);

        REQUIRE(path_sum_two_ways(table_2) == 427337);
    }
}

// PE 087 - Тройки степеней простых чисел
namespace
{
    template <class IntType>
    auto PE_087_prime_power_triplets(IntType n_limit)
    {
        auto const primes = saga::primes_below(IntType(std::sqrt(n_limit) + 1));

        auto const pred = [=](IntType const & arg) { return arg < n_limit; };

        auto const squares = saga::cursor::all(primes)
                           | saga::cursor::transform(saga::square)
                           | saga::cursor::to<std::vector<IntType>>();

        auto const cubes = saga::cursor::all(primes)
                         | saga::cursor::transform([](IntType const & arg)
                                                   { return saga::power_natural(arg, 3); })
                         | saga::cursor::take_while(pred)
                         | saga::cursor::to<std::vector<IntType>>();

        auto const forths = saga::cursor::all(primes)
                          | saga::cursor::transform([](IntType const & arg)
                                                    { return saga::power_natural(arg, 4); })
                          | saga::cursor::take_while(pred)
                          | saga::cursor::to<std::vector<IntType>>();

        std::vector<IntType> result;

        for(auto const & arg3 : forths)
        for(auto const & arg2 : cubes)
        {
            auto sum_2 = arg2 + arg3;

            if(sum_2 >= n_limit)
            {
                break;
            }

            for(auto const & arg1 : squares)
            {
                auto sum_3 = arg1 + sum_2;

                if(sum_3 < n_limit)
                {
                    result.push_back(std::move(sum_3));
                }
                else
                {
                    break;
                }
            }
        }

        auto cur = saga::cursor::all(result);
        saga::sort(cur);
        cur = saga::unique(std::move(cur));
        result.erase(cur.begin(), cur.end());

        return result;
    }
}

TEST_CASE("PE 087")
{
    auto const pe_087_50 = ::PE_087_prime_power_triplets(50);

    std::vector<int> const pe_087_50_expected{28, 33, 47, 49};

    CHECK(pe_087_50 == pe_087_50_expected);

    auto const pe_087 = ::PE_087_prime_power_triplets(std::int64_t(50'000'000));
    REQUIRE(pe_087.size() == 1'097'343);
}

// PE 092 - Цепочки квадратов цифр
namespace
{
    template <class IntType>
    IntType sum_of_digits_squares(IntType num)
    {
        return saga::transform_reduce(saga::cursor::digits_of(num)
                                     ,IntType(0), std::plus<>{}, saga::square);
    }

    template <class IntType, class Container>
    void update_sum_of_digits_squares_chain(IntType start, Container & finishes)
    {
        std::vector<IntType> chain;

        for(; finishes.at(start) == 0;)
        {
            chain.push_back(start);

            start = ::sum_of_digits_squares(std::move(start));
        }

        auto finish_value = finishes[start];

        for(auto each : chain)
        {
            finishes[each] = finish_value;
        }
    }

    template <class IntType>
    std::vector<IntType> square_digit_chains_full(IntType num_max)
    {
        std::vector<IntType> finishes(num_max, 0);
        finishes[1] = 1;
        finishes[89] = 89;

        for(auto num : saga::cursor::indices(IntType{1}, num_max))
        {
            ::update_sum_of_digits_squares_chain(num, finishes);
        }

        return finishes;
    }

    template <class Container>
    void PE_092_increment(Container & num)
    {
        auto carry = typename Container::value_type(1);

        for(auto & item : num)
        {
            item += carry;

            carry = item / 10;
            item %= 10;
        }

        if(carry > 0)
        {
            num.emplace_back(1);
        }
    }

    template <class IntType>
    IntType PE_092_square_digit_chains(IntType num_max)
    {
        // Числа, у которых сумма квадратов цифр может привышать само число
        auto const n_digits = IntType(std::log10(num_max)) + 1;
        auto const num_full = saga::square(9) * n_digits + 1;

        auto const finishes = square_digit_chains_full(num_full);

        auto const finish_to_count = IntType(89);

        auto result = saga::count(saga::cursor::all(finishes), finish_to_count);

        // Остальные числа
        using Vector = std::vector<IntType>;

        auto const num_max_vector = saga::cursor::digits_of(num_max)
                                  | saga::cursor::to<Vector>();

        auto num_vector = saga::cursor::digits_of(num_full)
                        | saga::cursor::to<Vector>();

        for(; num_vector != num_max_vector; ::PE_092_increment(num_vector))
        {
            auto const value = saga::transform_reduce(saga::cursor::all(num_vector)
                                                     ,IntType(0), std::plus<>{}, saga::square);

            result += (finishes[value] == finish_to_count);
        }

        return result;
    }
}

TEST_CASE("PE 092")
{
    // Суммы квадратов цифр
    REQUIRE(::sum_of_digits_squares(1) == 1);

    REQUIRE(::sum_of_digits_squares(44) == 32);
    REQUIRE(::sum_of_digits_squares(32) == 13);
    REQUIRE(::sum_of_digits_squares(13) == 10);
    REQUIRE(::sum_of_digits_squares(10) == 1);

    REQUIRE(::sum_of_digits_squares(85) == 89);
    REQUIRE(::sum_of_digits_squares(89) == 145);
    REQUIRE(::sum_of_digits_squares(145) == 42);
    REQUIRE(::sum_of_digits_squares(42) == 20);
    REQUIRE(::sum_of_digits_squares(20) == 4);
    REQUIRE(::sum_of_digits_squares(4) == 16);
    REQUIRE(::sum_of_digits_squares(16) == 37);
    REQUIRE(::sum_of_digits_squares(37) == 58);
    REQUIRE(::sum_of_digits_squares(58) == 89);

    REQUIRE(PE_092_square_digit_chains(std::int32_t{10'000'000}) == 8581146);
}

// PE 097 - Большое не-Мерсеновское простое число
TEST_CASE("PE 097")
{
    using IntType = std::uint64_t;
    auto const Mod = 10'000'000'000;

    auto prod = [&Mod](IntType lhs, IntType rhs)
    {
        auto const HalfMod = 100'000;

        auto a_lhs = lhs / HalfMod;
        auto b_lhs = lhs % HalfMod;

        auto a_rhs = rhs / HalfMod;
        auto b_rhs = rhs % HalfMod;

        return (HalfMod * (a_lhs * b_rhs + b_lhs * a_rhs) + b_lhs * b_rhs) % Mod;
    };

    auto const result = 28433 * saga::power_natural(IntType(2), 7'830'457, prod) + 1;

    REQUIRE(result % Mod == 8'739'992'577);
}

// PE 099 - Наибольшая степенное выражение
namespace
{
    struct base_exponent_pair
    {
        long base = 0;
        long exponent = 0;
    };

    std::istream & operator>>(std::istream & input, base_exponent_pair & obj)
    {
        input >> obj.base;

        auto sep = input.get();

        input >> obj.exponent;

        if(input)
        {
            assert(sep == ',');
        }

        return input;
    }
}

TEST_CASE("PE 099")
{
    std::ifstream file("ProjectEuler/p099_base_exp.txt");

    auto bep_log = [](::base_exponent_pair const & obj) {return obj.exponent * std::log(obj.base);};

    auto input = saga::make_istream_cursor<::base_exponent_pair>(file)
               | saga::cursor::transform(bep_log);

    using ValueAndIndex = std::pair<double, int>;

    auto const result
        = saga::transform_reduce(std::move(input), saga::cursor::iota(1), ValueAndIndex{0, 0}
                                ,SAGA_OVERLOAD_SET(std::max), SAGA_OVERLOAD_SET(std::make_pair));

    REQUIRE(result.second == 709);
}
