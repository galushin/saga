/* (c) 2022 Галушин Павел Викторович, galushin@gmail.com

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
#include <catch/catch.hpp>

// Используемые возможности
#include <saga/cursor/cartesian_product.hpp>
#include <saga/cursor/enumerate.hpp>
#include <saga/cursor/indices.hpp>
#include <saga/cursor/iota.hpp>
#include <saga/cursor/filter.hpp>
#include <saga/cursor/set_union.hpp>
#include <saga/cursor/stride.hpp>
#include <saga/cursor/take_while.hpp>
#include <saga/cursor/transform.hpp>
#include <saga/flat_set.hpp>
#include <saga/math.hpp>
#include <saga/numeric.hpp>
#include <saga/pipes/filter.hpp>
#include <saga/pipes/for_each.hpp>
#include <saga/pipes/transform.hpp>
#include <saga/utility/exchange.hpp>

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
            if(num % 3 == 0 || num % 5 == 0)
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

        auto input = saga::cursor::filter(saga::cursor::indices(n_max), pred);

        return saga::reduce(std::move(input));
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
        auto in_3 = saga::cursor::stride(saga::cursor::iota(IntType(3)), IntType(3));
        auto in_5 = saga::cursor::stride(saga::cursor::iota(IntType(5)), IntType(5));

        auto in_3_or_5 = saga::cursor::set_union(std::move(in_3), std::move(in_5));
        auto input = saga::cursor::take_while(in_3_or_5, [=](IntType num) { return num < n_max; });

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

        // Создание, копирование, уничтожение
        constexpr fibonacci_sequence(IntType num1, IntType num2)
         : prev_(num1)
         , cur_(num2)
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
            this->prev_ = saga::exchange(this->cur_, this->cur_ + this->prev_);
        }

    private:
        IntType prev_;
        IntType cur_;
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
        auto const fib_even
            = saga::cursor::filter(::fibonacci_sequence<IntType>(1, 2)
                                   , [](IntType const & arg) { return arg % 2 == 0; });

        auto input
            = saga::cursor::take_while(fib_even, [&](IntType const & arg) { return arg <= n_max; });

        return saga::reduce(std::move(input), IntType{0});
    }

    template <class IntType>
    constexpr IntType projectEuler_002_filter_after_take_while_input(IntType n_max)
    {
        auto fib_below_n_max
            = saga::cursor::take_while(::fibonacci_sequence<IntType>(1, 2)
                                       , [=](IntType const & arg) { return arg <= n_max; });

        auto input = saga::cursor::filter(std::move(fib_below_n_max)
                                          , [](IntType const & arg) { return arg % 2 == 0; });

        return saga::reduce(std::move(input), IntType{0});
    }

    template <class IntType>
    constexpr IntType projectEuler_002_filter_output(IntType n_max)
    {
        auto input = saga::cursor::take_while(::fibonacci_sequence<IntType>(1, 2)
                                              , [&](IntType const & arg) { return arg <= n_max; });

        auto result = IntType{0};
        auto accumulator = [&result](IntType const & arg) { result += arg; };
        auto output = saga::pipes::filter(saga::pipes::for_each(accumulator)
                                          , [](IntType const & arg) { return arg % 2 == 0; });

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
    constexpr
    IntType projectEuler_003(IntType num)
    {
        assert(num > 0);

        auto max_divisor = IntType{1};

        while(num % 2 == 0)
        {
            max_divisor = 2;
            num /= 2;
        }

        for(auto divisor = 3; divisor*divisor <= num; divisor += 2)
        {
            while(num % divisor == 0)
            {
                max_divisor = divisor;

                num /= divisor;
            }
        }

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
        auto values = saga::cursor::indices(1, num+1);

        return saga::reduce(saga::cursor::transform(values, saga::square), IntType{0});
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
    bool is_coprime_with_sorted(IntType const num, std::vector<IntType> const & values)
    {
        assert(num >= 0);

        for(auto const & value : values)
        {
            if(value * value > num)
            {
                break;
            }

            if(num % value == 0)
            {
                return false;
            }
        }

        return true;
    }

    template <class IntType>
    IntType projectEuler_007(IntType const count)
    {
        assert(count > 0);

        std::vector<IntType> primes{2};

        for(auto num = IntType{3}; primes.size() < static_cast<std::size_t>(count); num += 2)
        {
            if(::is_coprime_with_sorted(num, primes))
            {
                primes.push_back(num);
            }
        }

        return primes.back();
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
    constexpr std::size_t projectEuler_008(char const (&str)[Size], std::size_t window)
    {
        assert(window <= std::size(str));

        auto data = saga::cursor::transform(saga::cursor::all(str), [](char c) { return c - '0'; });

        auto data_slided = saga::cursor::slide(data, window);

        auto prod = [](auto const & window)
        {
            return saga::accumulate(window, std::size_t{1}, std::multiplies<>{});
        };

        auto data_slided_aggregated = saga::cursor::transform(data_slided, prod);

        auto max_fn = [](auto && lhs, auto && rhs)
        {
            return std::max(std::forward<decltype(lhs)>(lhs), std::forward<decltype(rhs)>(rhs));
        };

        return saga::accumulate(std::move(data_slided_aggregated), std::size_t(0), max_fn);
    }

    static_assert(::projectEuler_008(pe008_data, 4) == 5832, "");
    // @note Некоторые компиляторы упираются в лимиты реализации constexpr в более сложном тесте
    // static_assert(::projectEuler_008(pe008_data, 13) == 23514624000, "");
}

TEST_CASE("ProjectEuler: 008")
{
    CHECK(::projectEuler_008(pe008_data, 4) == 5832);
    CHECK(::projectEuler_008(pe008_data, 13) == 23514624000);
}

// PE 009: Особая Пифагорова тройка
namespace
{
    // @todo Обобщить, чтобы можно было использовать в PE 003
    template <class IntType>
    IntType remove_factor(IntType num, IntType factor)
    {
        for(; num % factor == 0;)
        {
            num /= factor;
        }

        return num;
    }

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
            if(sum_2 % m == 0)
            {
                auto const sum_m = ::remove_factor(sum_2, 2);

                for(auto k = m + 1 + (m % 2); k < 2*m && k <= sum_m; k += 2)
                {
                    if(sum_m % k == 0 && std::gcd(k, m) == 1)
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

    // @note Некоторые компиляторы упираются в лимиты реализации constexpr
    // static_assert(::projectEuler_009_simple(1000) == 31875000, "");

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

    template <class IntType>
    std::vector<std::vector<IntType>>
    projectEuler_011_parse(std::istream & src_in)
    {
        std::vector<std::vector<IntType>> data;

        std::string line;
        for(; src_in;)
        {
            std::getline(src_in, line);

            if(!src_in)
            {
                break;
            }

            std::istringstream line_in(line);

            std::vector<IntType> row;
            saga::copy(saga::make_istream_cursor<IntType>(line_in), saga::back_inserter(row));

            if(!row.empty())
            {
                data.push_back(std::move(row));
            }
        }

        return data;
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

        for(auto const & row : data)
        {
            assert(row.size() == cols);
        }

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

    CAPTURE(data);

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
            if(value % d == 0)
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
        std::vector<IntType> divisors;

        for(auto num = IntType(1);; ++ num)
        {
            auto m1 = num % 2 == 0 ? num/2 : num;
            auto m2 = num % 2 == 0 ? num + 1 : (num + 1)/2;

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

namespace saga
{
    class integer
    {
        template <class CharT, class Traits>
        friend std::basic_ostream<CharT, Traits> &
        operator<<(std::basic_ostream<CharT, Traits> & out, integer const & value)
        {
            auto num = value.digits_.size();

            if(num == 0)
            {
                out << '0';
                return out;
            }

            for(; num > 0; -- num)
            {
                out << char('0' + value.digits_[num - 1]);
            }

            return out;
        }

        friend integer operator+(integer lhs, integer const & rhs)
        {
            lhs += rhs;

            return lhs;
        }

        friend integer operator*(integer const & lhs, integer const & rhs)
        {
            integer result;

            auto const rhs_size = rhs.digits_.size();

            for(size_t index = 0; index != rhs_size; ++ index)
            {
                result += saga::integer::mult_impl(lhs, rhs.digits_[index], index);
            }

            return result;
        }

        template <class IntType>
        friend auto operator*(integer const & lhs, IntType rhs)
        -> std::enable_if_t<std::is_integral<IntType>{}, integer>
        {
            return lhs * integer(std::move(rhs));
        }

        friend bool operator==(integer const & lhs, integer const & rhs)
        {
            return lhs.digits_ == rhs.digits_;
        }

    private:
        using Digit = int;

        static integer mult_impl(integer const & lhs, Digit const & rhs, std::size_t initial_zeroes)
        {
            if(lhs.digits_.empty())
            {
                return lhs;
            }

            std::vector<Digit> result(initial_zeroes, Digit(0));

            auto carry = Digit(0);

            for(auto const & digit : lhs.digits_)
            {
                carry += digit * rhs;

                result.push_back(carry % 10);

                carry /= 10;
            }

            assert(0 <= carry && carry < 10);

            if(carry > 0)
            {
                result.push_back(carry);
            }

            integer tmp;
            tmp.digits_ = std::move(result);

            return tmp;
        }

    public:
        // Создание, копирование, уничтожение
        integer() = default;

        template <class IntType, class = std::enable_if_t<std::is_integral<IntType>{}>>
        explicit integer(IntType value)
        {
            assert(value >= 0);

            saga::copy(saga::cursor::digits_of(value), saga::back_inserter(this->digits_));
        }

        explicit integer(std::string const & str)
        {
            this->digits_.reserve(str.size());

            for(auto const & each : str)
            {
                assert(std::isdigit(each));

                digits_.push_back(each - '0');
            }

            saga::reverse(saga::cursor::all(this->digits_));
        }

        // Арифметические операции
        integer & operator+=(integer const & rhs)
        {
            auto const num = std::max(this->digits_.size(), rhs.digits_.size());

            this->digits_.resize(num, 0);

            auto carry = Digit(0);

            for(auto index : saga::cursor::indices(rhs.digits_.size()))
            {
                carry += this->digits_[index] + rhs.digits_[index];

                this->digits_[index] = carry % 10;

                carry /= 10;
            }

            for(auto index : saga::cursor::indices(rhs.digits_.size(), this->digits_.size()))
            {
                carry += this->digits_[index];

                this->digits_[index] = carry % 10;

                carry /= 10;
            }

            assert(0 <= carry && carry < 10);

            if(carry > 0)
            {
                this->digits_.push_back(carry);
            }

            return *this;
        }

        integer & operator*=(integer const & rhs)
        {
            *this = *this * rhs;

            return *this;
        }

        // Доступ к представлению
        std::vector<Digit> const & digits() const
        {
            return this->digits_;
        }

    private:
        std::vector<Digit> digits_;
    };
}
// namespace saga

TEST_CASE("saga::integer: default ctor")
{
    saga::integer const zero{};

    std::ostringstream os;
    os << zero;

    REQUIRE(os.str() == "0");
}

TEST_CASE("saga::integer: zero multiplication")
{
    saga_test::property_checker << [](unsigned const value)
    {
        saga::integer const zero{};

        auto const prod = zero * saga::integer(value);

        REQUIRE(prod == zero);
    };
}

TEST_CASE("PE 013 - range for loop")
{
    saga::integer result{};

    for(auto const & str : PE_013_data)
    {
        result += saga::integer(str);
    }

    std::ostringstream os;
    os << result;

    REQUIRE(os.str().substr(0, 10) == "5537376230");
}

TEST_CASE("PE 013 - algorithms")
{
    auto const result
        = saga::transform_reduce(saga::cursor::all(PE_013_data), saga::integer{}
                                 , std::plus<>{}
                                 , [](std::string const & str) { return saga::integer(str); });

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
        return (value % 2 == 0) ? value / 2 : 3*value+1;
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

    int
    projectEuler_016_arbitrary(int base, int power)
    {
        assert(power > 0);

        auto num = saga::power_natural(saga::integer(base), power);

        return saga::reduce(saga::cursor::all(num.digits()));
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

// PE 019 Подсчёт числа воскресений
namespace
{
    bool is_leap(int year)
    {
        return (year % 4 == 0) - (year % 100 == 0 && year % 400 != 0);
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
                                                , saga::integer(1), std::multiplies<>{});

        return saga::reduce(saga::cursor::all(factorial.digits()));
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
        return saga::transform_reduce(saga::cursor::all(str), long{0}, std::plus<>{}
                                      , [](char each) { return each - 'A' + 1; });
    }
}

// PE 022: Баллы для имён
#include <fstream>

TEST_CASE("PE 022")
{
    REQUIRE(::alphabetical_value("COLIN") == 53);

    std::vector<std::string> names;

    {
        std::ifstream file("ProjectEuler/p022_names.txt");

        REQUIRE(!!file);

        std::string reader;
        for(; file; )
        {
            char c = file.get();
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
    }

    // Отсортировать
    saga::sort(saga::cursor::all(names));

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
        auto cur = saga::cursor::enumerate(::make_fibonacci_sequence(saga::integer(0)
                                                                     , saga::integer(1)));
        cur = saga::find_if(std::move(cur),
                            [&](auto const & elem) {return elem.value.digits().size() >= digits;});
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

    int PE_026_reciprocal_cycle_length(int num)
    {
        assert(num > 0);

        auto slow = pe_026_cursor(num);

        assert(!!slow);

        auto fast = slow;
        ++ fast;

        for(;;)
        {
            if(!fast)
            {
                return 0;
            }

            if(*slow == *fast)
            {
                break;
            }

            ++ slow;
            ++ fast;

            if(!fast)
            {
                return 0;
            }

            ++ fast;
        }

        int result = 1;
        ++ fast;

        for(; *fast != *slow; ++ fast, ++ result)
        {}

        return result;
    }

    int PE_026(int num)
    {
        auto cur = saga::cursor::transform(saga::cursor::indices(2, num)
                                           , ::PE_026_reciprocal_cycle_length);

        return saga::max_element(saga::cursor::cached1(std::move(cur))).base().base().front();
    }
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

            if(!is_coprime_with_sorted(std::abs(value), primes))
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
                                               , saga::cursor::indices(-1000, 1001));

    auto transformed = saga::cursor::transform(std::move(cur), std::ref(fun));

    auto const pos = saga::max_element(saga::cursor::cached1(std::move(transformed))).base().base();

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
TEST_CASE("PE 031")
{
    using IntType = std::size_t;

    std::vector<IntType> const coins{1, 2, 5, 10, 20, 50, 100, 200};

    auto const amount = IntType(200);

    std::vector<IntType> ways(amount+1, 0);
    ways.front() = 1;

    for(auto coin : saga::cursor::all(coins))
    {
        for(auto index : saga::cursor::indices(0u, ways.size() - coin))
        {
            ways[index + coin] += ways[index];
        }
    }

    REQUIRE(ways.back() == 73682);
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

            saga::sort(saga::cursor::all(str));

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

// PE: Факториалы цифр
namespace
{
    template <class IntType>
    constexpr std::array<IntType, 10> PE_034_factorials_table()
    {
        std::array<IntType, 10> factorials{1};

        saga::iota(saga::cursor::all(factorials), 0);
        factorials.front() = 1;

        saga::partial_sum(saga::cursor::all(factorials), saga::cursor::all(factorials)
                          , std::multiplies<>{});

        return factorials;
    }

    template <class IntType>
    constexpr IntType PE_034_digits_factorial_sum(IntType num)
    {
        constexpr auto factorials = ::PE_034_factorials_table<IntType>();

        return saga::transform_reduce(saga::cursor::digits_of(num), IntType(0)
                                      , std::plus<>{}
                                      , [&](IntType arg) { return factorials[arg]; });
    }

    static_assert(::PE_034_digits_factorial_sum(1) == 1);
    static_assert(::PE_034_digits_factorial_sum(2) == 2);
    static_assert(::PE_034_digits_factorial_sum(145) == 145);

    constexpr long PE_034()
    {
        using IntType = long;

        // Можно показать, что если количество цифр больше 7, то сумма факториалов цифр всегда
        // меньше самого этого числа
        auto numbers = saga::cursor::indices(IntType(10), saga::power_natural(IntType(10), 7));
        auto pred = [](IntType num) { return num == PE_034_digits_factorial_sum(num); };

        return saga::reduce(saga::cursor::filter(numbers, pred));
    }
}

TEST_CASE("PE 034")
{
    REQUIRE(::PE_034_digits_factorial_sum(40585) == 40585);

    REQUIRE(PE_034() == 40730);
}
