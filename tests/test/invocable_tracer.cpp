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
#include <saga/test/invocable_tracer.hpp>

// Тестовая инфраструктура
#include <catch2/catch_amalgamated.hpp>
#include "../saga_test.hpp"

// Вспомогательные файлы
#include <complex>
#include <utility>

// Тесты
TEST_CASE("make_invocable_tracer: functional object, default tag")
{
    saga_test::property_checker << [](std::string const & lhs, std::string const & rhs)
    {
        auto const fun = std::less<>{};

        auto const tracer = saga::make_invocable_tracer(fun);

        auto const invoke_count_old = tracer.invoke_count();

        REQUIRE(fun(lhs, rhs) == tracer(lhs, rhs));
        REQUIRE(tracer.invoke_count() == invoke_count_old + 1);
    };
}

TEST_CASE("make_invocable_tracer: functional object, custom tag")
{
    struct First_tag{};
    struct Second_tag{};
    saga_test::property_checker << [](std::string const & lhs, std::string const & rhs)
    {
        auto const fun = std::less<>{};

        auto const tracer_1 = saga::make_invocable_tracer<First_tag>(fun);
        auto const tracer_2 = saga::make_invocable_tracer<Second_tag>(fun);

        auto const invoke_count_old_1 = tracer_1.invoke_count();
        auto const invoke_count_old_2 = tracer_2.invoke_count();

        REQUIRE(fun(lhs, rhs) == tracer_1(lhs, rhs));
        REQUIRE(tracer_1.invoke_count() == invoke_count_old_1 + 1);
        REQUIRE(tracer_2.invoke_count() == invoke_count_old_2);
    };
}

namespace
{
    struct cv_ref_functor
    {
        int & operator()() &
        {
            return this->value;
        }

        int && operator()() &&
        {
            return std::move(this->value);
        }

        int const & operator()() const &
        {
            return this->value;
        }

        int const && operator()() const &&
        {
            return std::move(this->value);
        }

        int value = 0;
    };

    using CV_Ref_tracer = saga::invocable_tracer<cv_ref_functor>;
}

static_assert(std::is_same<saga::invoke_result_t<CV_Ref_tracer &>, int &>{}, "");
static_assert(std::is_same<saga::invoke_result_t<CV_Ref_tracer &&>, int &&>{}, "");
static_assert(std::is_same<saga::invoke_result_t<CV_Ref_tracer const &>, int const &>{}, "");
static_assert(std::is_same<saga::invoke_result_t<CV_Ref_tracer const &&>, int const &&>{}, "");

TEST_CASE("invocable_tracer: rvalue transparency")
{
    saga_test::property_checker << [](int const & value)
    {
        auto const fun = ::cv_ref_functor{value};

        auto const tracer = CV_Ref_tracer(fun);

        auto const invoke_count_old = tracer.invoke_count();

        REQUIRE(::CV_Ref_tracer(fun)() == value);
        REQUIRE(static_cast<::CV_Ref_tracer const &&>(::CV_Ref_tracer(fun))() == value);

        REQUIRE(tracer.invoke_count() == invoke_count_old + 2);
    };
}

TEST_CASE("invocable_tracer: variable member pointer")
{
    using Pair = std::pair<long, std::string>;

    saga_test::property_checker <<[](Pair const & value)
    {
        auto const pmv_first = &Pair::first;

        auto tracer_first = saga::make_invocable_tracer(pmv_first);

        auto const invoke_count_old = tracer_first.invoke_count();

        REQUIRE(tracer_first(value) == value.first);

        REQUIRE(tracer_first.invoke_count() == invoke_count_old + 1);
    };
}

TEST_CASE("invocable_tracer: function member pointer")
{
    using Scalar = double;

    saga_test::property_checker <<[](Scalar const real, Scalar const imag)
    {
        using Complex = std::complex<Scalar>;

        auto const pmf = static_cast<Scalar (Complex::*)() const>(&Complex::real);

        auto value = Complex(real, imag);
        auto const tracer = saga::make_invocable_tracer(pmf);

        auto const invoke_count_old = tracer.invoke_count();

        REQUIRE(tracer(value) == real);
        REQUIRE(tracer.invoke_count() == invoke_count_old + 1);
    };
}

namespace
{
    struct may_throw_on_call
    {
        void operator()();
    };
}
static_assert(saga::is_nothrow_invocable<std::less<>, int, int>{}, "");
static_assert(saga::is_nothrow_invocable<saga::invocable_tracer<std::less<>>, int, int>{}, "");
static_assert(saga::is_nothrow_invocable<saga::invocable_tracer<std::less<>> const, int, int>{}, "");
static_assert(saga::is_nothrow_invocable<saga::invocable_tracer<std::less<>> &&, int, int>{}, "");
static_assert(saga::is_nothrow_invocable<saga::invocable_tracer<std::less<>> const &&, int, int>{}, "");

static_assert(!saga::is_nothrow_invocable<::may_throw_on_call>{}, "");
static_assert(!saga::is_nothrow_invocable<saga::invocable_tracer<::may_throw_on_call>>{}, "");
static_assert(!saga::is_nothrow_invocable<saga::invocable_tracer<::may_throw_on_call> const>{}, "");
static_assert(!saga::is_nothrow_invocable<saga::invocable_tracer<::may_throw_on_call> &&>{}, "");
static_assert(!saga::is_nothrow_invocable<saga::invocable_tracer<::may_throw_on_call> const &&>{}, "");

