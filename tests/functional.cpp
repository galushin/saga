/* (c) 2021-2022 Галушин Павел Викторович, galushin@gmail.com

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
#include <saga/functional.hpp>

// Инфраструктура тестирования
#include "./saga_test.hpp"
#include <catch2/catch_amalgamated.hpp>

// Вспомогательные возможноти

// Тесты
namespace
{
    template <class Unsigned, class IncrementOp>
    void test_increment(Unsigned num)
    {
        static_assert(std::is_unsigned<Unsigned>{}, "Must be unsigned");

        auto fun = IncrementOp{};

        auto const num_old = num;

        auto const & result = fun(num);

        REQUIRE(num == num_old + 1);

        REQUIRE(std::addressof(result) == std::addressof(num));
        static_assert(std::is_same<decltype(fun(num)), Unsigned &>{}, "");
    }

    template <class Unsigned, class IncrementOp>
    void test_decrement(Unsigned num)
    {
        static_assert(std::is_unsigned<Unsigned>{}, "Must be unsigned");

        auto fun = IncrementOp{};

        auto const num_old = num;

        auto const & result = fun(num);

        REQUIRE(num == num_old - 1);

        REQUIRE(std::addressof(result) == std::addressof(num));
        static_assert(std::is_same<decltype(fun(num)), Unsigned &>{}, "");
    }
}
TEST_CASE("increment and decrement functional objects")
{
    // Выбран беззнаковый тип, так как знаковых типов возможно переполнение
    saga_test::property_checker
        << ::test_increment<unsigned, saga::increment<unsigned>>
        << ::test_increment<unsigned, saga::increment<>>
        << ::test_decrement<unsigned, saga::decrement<unsigned>>
        << ::test_decrement<unsigned, saga::decrement<>>;
}

#include <complex>

TEST_CASE("invoke: function member pointer")
{
    using Scalar = double;

    saga_test::property_checker <<[](Scalar const real, Scalar const imag)
    {
        using Complex = std::complex<Scalar>;
        auto const value = Complex(real, imag);

        auto pmf = static_cast<Scalar (Complex::*)() const>(&Complex::real);

        REQUIRE(saga::invoke(pmf, value) == value.real());

        REQUIRE(saga::invoke(pmf, std::ref(value)) == value.real());

        REQUIRE(saga::invoke(pmf, std::addressof(value)) == value.real());
    };
}

TEST_CASE("invoke: variable member pointer")
{
    using Pair = std::pair<long, std::string>;

    saga_test::property_checker <<[](Pair const & value)
    {
        REQUIRE(saga::invoke(&Pair::first, value) == value.first);
        REQUIRE(saga::invoke(&Pair::second, value) == value.second);

        REQUIRE(saga::invoke(&Pair::first, std::addressof(value)) == value.first);
        REQUIRE(saga::invoke(&Pair::second, std::addressof(value)) == value.second);

        REQUIRE(saga::invoke(&Pair::first, std::ref(value)) == value.first);
        REQUIRE(saga::invoke(&Pair::second, std::ref(value)) == value.second);
    };
}

namespace
{
    int id_may_throw(int x)
    {
        return x;
    }

    int id_noexcept(int x) noexcept(true)
    {
        return x;
    }

    static_assert(!noexcept(id_may_throw(42)), "");
    static_assert(!noexcept(saga::invoke(id_may_throw, 42)), "");

#if __cpp_noexcept_function_type >= 201510
    static_assert(noexcept(id_noexcept(42)), "");
    static_assert(saga::is_invocable<decltype(id_noexcept), int>{}, "");
    static_assert(saga::is_nothrow_invocable<decltype(id_noexcept), int>{}, "");
    static_assert(noexcept(saga::invoke(id_noexcept, 42)), "");
#endif
// __cpp_noexcept_function_type
}

TEST_CASE("invoke: function pointer")
{
    saga_test::property_checker << [](int const value)
    {
        REQUIRE(saga::invoke(id_may_throw, value) == id_may_throw(value));
        REQUIRE(saga::invoke(id_noexcept, value) == id_noexcept(value));
    };
}

TEST_CASE("invoke: functional object")
{
    saga_test::property_checker << [](int lhs, int rhs)
    {
        auto const fun = std::bit_xor<>{};

        REQUIRE(saga::invoke(fun, lhs, rhs) == fun(lhs, rhs));

        static_assert(noexcept(saga::invoke(fun, lhs, rhs)) == noexcept(fun(lhs, rhs)), "");
    };
}

namespace
{
    struct identity
    {
        template <class T>
        T const & operator()(T const & arg) noexcept
        {
            return arg;
        }
    };
}

TEST_CASE("invoke: functional object, noexcept")
{
    saga_test::property_checker << [](int arg)
    {
        REQUIRE(saga::invoke(::identity{}, arg) == arg);

        static_assert(noexcept(saga::invoke(::identity{}, arg)), "");
    };
}

namespace
{
    struct Mod_2017
    {
        template <class T>
        constexpr T operator()(T const & arg) const
        {
            return arg % 2017;
        }
    };
}

TEST_CASE("compare_by: default compare")
{
    using Value = int;

    saga_test::property_checker << [](Value const & mask, Value const & lhs, Value const & rhs)
    {
        auto const proj = [=](Value const & arg) { return arg ^ mask; };

        auto const cmp_by = saga::compare_by(proj);

        REQUIRE(cmp_by(lhs, rhs) == (proj(lhs) < proj(rhs)));
    };

    static_assert(saga::compare_by(Mod_2017{})(5, 7), "");
    static_assert(!saga::compare_by(Mod_2017{})(7, 5), "");
    static_assert(saga::compare_by(Mod_2017{})(2018, 2016), "");
}

TEST_CASE("compare_by: custom compare")
{
    using Value = long;

    saga_test::property_checker << [](Value const & mask, Value const & lhs, Value const & rhs)
    {
        auto const proj = [=](Value const & arg) { return arg ^ mask; };
        auto const cmp = std::greater<>{};

        auto const cmp_by = saga::compare_by(proj, cmp);

        REQUIRE(cmp_by(lhs, rhs) == cmp(proj(lhs), proj(rhs)));
    };

    static_assert(saga::compare_by(Mod_2017{}, std::greater<>{})(7, 5), "");
    static_assert(!saga::compare_by(Mod_2017{}, std::greater<>{})(5, 7), "");
    static_assert(!saga::compare_by(Mod_2017{}, std::greater<>{})(2018, 2016), "");
}

TEST_CASE("equivalent_up_to: default compare")
{
    using Value = int;

    saga_test::property_checker << [](Value const & mask, Value const & lhs, Value const & rhs)
    {
        auto const proj = [=](Value const & arg) { return arg ^ mask; };

        auto const equiv = saga::equivalent_up_to(proj);

        REQUIRE(equiv(lhs, rhs) == (proj(lhs) == proj(rhs)));
    };

    static_assert(saga::equivalent_up_to(Mod_2017{})(5, 5), "");
    static_assert(saga::equivalent_up_to(Mod_2017{})(5, 2022), "");
    static_assert(!saga::equivalent_up_to(Mod_2017{})(5, 7), "");
    static_assert(!saga::equivalent_up_to(Mod_2017{})(2018, 2016), "");
}

TEST_CASE("reverse_args")
{
    using Value = long;

    saga_test::property_checker << [](Value const & lhs, Value const & rhs)
    {
        auto const cmp = std::greater<>{};
        auto const cmp_converse = saga::f_transpose(cmp);

        REQUIRE(cmp_converse(lhs, rhs) == cmp(rhs, lhs));
    };

    constexpr auto cmp = std::greater<>{};
    static_assert(saga::f_transpose(cmp)(5, 7) == cmp(7, 5), "");
}
