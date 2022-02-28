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

// PE 005: Наименьшее кратное
namespace
{
    template <class IntType>
    constexpr
    IntType projectEuler_005(IntType num)
    {
        using Iterator = saga::iota_iterator<IntType>;
        using Cursor = saga::subrange_cursor<Iterator>;

        // @todo cursor::iota(2, num)
        return saga::accumulate(Cursor(Iterator(2), Iterator(num), saga::unsafe_tag_t{})
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
