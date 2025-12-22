/* (c) 2022-2025 Галушин Павел Викторович, galushin@gmail.com

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

// Используемые возможности
#include <saga/algorithm.hpp>
#include <saga/cursor/cached1.hpp>
#include <saga/cursor/cursor_facade.hpp>
#include <saga/cursor/filter.hpp>
#include <saga/cursor/indices.hpp>
#include <saga/cursor/take.hpp>
#include <saga/cursor/take_while.hpp>
#include <saga/cursor/transform.hpp>
#include <saga/cursor/to.hpp>
#include <saga/math.hpp>
#include <saga/utility/exchange.hpp>

#include "./integer10.hpp"

// Инфраструктура тестирования
#include <catch2/catch_amalgamated.hpp>

// PE 026: Циклы в обратных числах
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

    std::size_t digits_count(saga::experimental::integer10 const & num)
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
    REQUIRE(::digits_count(saga::experimental::integer10()) == 0);
}

TEMPLATE_TEST_CASE("sqrt(2) convergents", "convergent", int, saga::experimental::integer10)
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

using saga::experimental::integer10;

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
         , sqrt_floor_(saga::isqrt(this->number_))
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

        return saga::experimental::digits_sum(conv.numerator());
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
