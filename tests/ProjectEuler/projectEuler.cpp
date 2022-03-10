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
#include <saga/algorithm.hpp>
#include <saga/cursor/filter.hpp>
#include <saga/cursor/subrange.hpp>
#include <saga/cursor/take_while.hpp>
#include <saga/cursor/transform.hpp>
#include <saga/pipes/filter.hpp>
#include <saga/pipes/for_each.hpp>
#include <saga/pipes/transform.hpp>
#include <saga/math.hpp>
#include <saga/numeric.hpp>
#include <saga/utility/exchange.hpp>
#include <saga/view/indices.hpp>

#include <vector>
#include <optional>

// Тесты
// PE 001 : сумма кратных 3 или 5
namespace
{
    template <class IntType>
    constexpr IntType projectEuler_001_range_for_loop_indices(IntType n_max)
    {
        auto result = IntType(0);

        for(auto num : saga::view::indices(n_max))
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
        saga::copy_if(saga::cursor::all(saga::view::indices(n_max))
                      , saga::back_inserter(tmp)
                      , [](IntType const & num) { return num % 3 == 0 || num % 5 == 0; });

        return saga::reduce(saga::cursor::all(tmp));
    }

    template <class IntType>
    constexpr IntType projectEuler_001_cursor_algorithms_filter(IntType n_max)
    {
        auto pred = [](IntType const & num) { return num % 3 == 0 || num % 5 == 0; };

        auto input = saga::cursor::filter(saga::cursor::all(saga::view::indices(n_max)), pred);

        return saga::reduce(std::move(input));
    }

    template <class IntType>
    constexpr IntType projectEuler_001_smart_output_cursor(IntType n_max)
    {
        auto pred = [](IntType const & num) { return num % 3 == 0 || num % 5 == 0; };

        auto result = IntType{0};
        auto accumulator = [&result](IntType const & arg) { result += arg; };

        saga::copy(saga::cursor::all(saga::view::indices(n_max))
                   , saga::pipes::filter(saga::pipes::for_each(accumulator), pred));

        return result;
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

TEST_CASE("ProjectEuler 001")
{
    CHECK(projectEuler_001_cursor_algorithms(10) == 23);
    CHECK(projectEuler_001_cursor_algorithms(1000) == 233168);

    for(auto const & num : saga::view::indices(1, 1000))
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
        return saga::accumulate(saga::cursor::all(saga::view::indices(2, num))
                                , IntType{1}, saga::lcm);
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
        return saga::reduce(saga::cursor::all(saga::view::indices(1, num+1)), IntType{0});
    }

    template <class IntType>
    constexpr IntType projectEuler_006_sum_pipes(IntType num)
    {
        auto result = IntType{0};
        auto accumulator = [&result](IntType const & arg) { result += arg; };

        saga::copy(saga::cursor::all(saga::view::indices(1, num+1))
                   , saga::pipes::for_each(accumulator));

        return result;
    }

    template <class IntType>
    constexpr IntType projectEuler_006_sum_squares_cursor(IntType num)
    {
        auto values = saga::cursor::all(saga::view::indices(1, num+1));

        return saga::reduce(saga::cursor::transform(values, saga::square), IntType{0});
    }

    template <class IntType>
    constexpr IntType projectEuler_006_sum_squares_pipes(IntType num)
    {
        auto result = IntType{0};
        auto accumulator = [&result](IntType const & arg) { result += arg; };

        saga::copy(saga::cursor::all(saga::view::indices(1, num+1))
                   , saga::pipes::transform(saga::pipes::for_each(accumulator), saga::square));

        return result;
    }

    // Square pyramidal number
    template <class IntType>
    constexpr IntType projectEuler_006_sum_squares_formula(IntType num)
    {
        auto P = saga::triangular_number(num);

        if(P % 3 == 0)
        {
            return P / 3 * (2 * num + 1);
        }
        else
        {
            return (2 * num + 1) / 3 * P;
        }
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
                - ::projectEuler_006_sum_squares_formula(num);
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
static_assert(::projectEuler_006_sum_squares_formula(10) == 385, "");

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
    constexpr char const pe008_data[] =
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
        for(auto a : saga::view::indices(3, std::max(3, (sum - 3)/3)))
        for(auto b : saga::view::indices(a+1, std::max(a+1, (sum - 2)/2)))
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

        for(auto m : saga::view::indices(2, mlimit))
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
