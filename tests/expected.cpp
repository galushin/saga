/* (c) 2020 Галушин Павел Викторович, galushin@gmail.com

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
#include <saga/expected.hpp>

// Тестовая инфраструктура
#include "./saga_test.hpp"
#include <catch/catch.hpp>
#include <type_traits>

// Вспомогательне файлы
#include <stdexcept>
#include <set>

// Тесты

// @todo 5. Шаблон класса unexpected
static_assert(std::is_copy_constructible<saga::unexpected<int>>{}, "");
static_assert(std::is_move_constructible<saga::unexpected<int>>{}, "");

// Есть конструктор с in_place_t и любым числом аргументов
static_assert(std::is_constructible<std::vector<int>, std::size_t, int>{}, "");
static_assert(std::is_constructible<saga::unexpected<std::vector<int>>,
                                    saga::in_place_t, std::size_t, int>{}, "");

// Конструктор с in_place_t является explicit:
static_assert(std::is_constructible<std::vector<int>>{}, "");
static_assert(std::is_constructible<saga::unexpected<std::vector<int>>, saga::in_place_t>{}, "");
static_assert(!std::is_convertible<saga::in_place_t, saga::unexpected<std::vector<int>>>{}, "");

// Конструктор с in_place_t не участвует в разрешение перегрузки, когда его нельзя вызывать
static_assert(!std::is_constructible<int, std::vector<int>>{}, "");
static_assert(!std::is_constructible<saga::unexpected<int>,
                                     saga::in_place_t, std::vector<int>>{}, "");

static_assert(!std::is_constructible<int, std::initializer_list<int>&, int>{}, "");
static_assert(!std::is_constructible<saga::unexpected<int>,
                                     saga::in_place_t, std::initializer_list<int>&, int>{}, "");

TEST_CASE("unexpected: placement constructor")
{
    {
        constexpr saga::unexpected<int> err0(saga::in_place_t{});

        static_assert(err0.value() == 0, "");
        static_assert(noexcept(err0.value()), "");

        constexpr int value = 42;
        constexpr saga::unexpected<int> err1(saga::in_place_t{}, value);
        static_assert(err1.value() == value, "");
    }

    saga_test::property_checker << [](saga_test::container_size<std::size_t> num, int filler)
    {
        std::vector<int> const expected(num, filler);

        saga::unexpected<std::vector<int>> const actual(saga::in_place_t{}, num, filler);

        REQUIRE(actual.value() == expected);
    };
}

TEST_CASE("unexpected: placement constructor with initializer list")
{
    saga_test::property_checker << [](int value1, int value2)
    {
        std::vector<int> const expected{value1, value2};

        saga::unexpected<std::vector<int>> const actual(saga::in_place_t{}, {value1, value2});

        REQUIRE(actual.value() == expected);
    };
}

TEST_CASE("unexpected: placement constructor with initializer list and more args")
{
    {
        struct initializer_list_consumer
        {
            constexpr initializer_list_consumer(std::initializer_list<int> inits, int arg)
             : value(arg)
            {
                for(auto const & each : inits)
                {
                    value += each;
                }
            }

            int value = 0;
        };

        constexpr saga::unexpected<initializer_list_consumer> unex(saga::in_place_t{}, {1, 2, 3, 4}, 5);
    }

    saga_test::property_checker << [](int value1, int value2)
    {
        using Compare = bool(*)(int const &, int const &);
        using Container = std::set<int, Compare>;

        auto const cmp = Compare([](int const & x, int const & y) { return x < y; });

        Container const expected({value1, value2}, cmp);

        saga::unexpected<Container> const actual(saga::in_place_t{}, {value1, value2}, cmp);

        REQUIRE(actual.value() == expected);
    };
}

TEST_CASE("unexpected : copy constructor")
{
    {
        constexpr int value = 42;
        constexpr saga::unexpected<int> err1(saga::in_place_t{}, value);
        static_assert(err1.value() == value, "");

        constexpr saga::unexpected<int> err2(err1);

        static_assert(err1.value() == err2.value(), "");
    }

    saga_test::property_checker << [](saga_test::container_size<std::size_t> num, int filler)
    {
        saga::unexpected<std::vector<int>> const obj1(saga::in_place_t{}, num, filler);

        saga::unexpected<std::vector<int>> const obj2(obj1);

        REQUIRE(obj1.value() == obj2.value());
    };
}

namespace
{
    template <class T>
    struct move_only
    {
    public:
        constexpr explicit move_only(T init_value)
         : value(std::move(init_value))
        {}

        move_only(move_only const &) = delete;
        move_only(move_only &&) = default;

        T value;
    };

    template <class T>
    constexpr T use_constexpr_move(T x)
    {
        auto y = std::move(x);

        return y;
    }
}

TEST_CASE("unexpected : move constructor")
{
    {
        constexpr int value = 17;

        constexpr auto obj
            = use_constexpr_move(saga::unexpected<move_only<int>>(saga::in_place_t{}, value));

        static_assert(obj.value().value == value, "");
    }

    using Value = std::vector<int>;
    saga_test::property_checker << [](Value const & value)
    {
        saga::unexpected<Value> obj1(value);
        auto const obj1_old = obj1;

        saga::unexpected<Value> const obj2(std::move(obj1));

        REQUIRE(obj2 == obj1_old);
        REQUIRE(obj1.value().empty());
    };
}

// Конструктор с одним значением не участвует в разрешение перегрузки, когда его нельзя вызывать
static_assert(!std::is_constructible<int, std::vector<int>>{}, "");
static_assert(!std::is_constructible<saga::unexpected<int>, std::vector<int>>{}, "");

// Конструктор с одним аргументом является explicit:
static_assert(std::is_constructible<saga::unexpected<std::string>, std::string>{}, "");
static_assert(!std::is_convertible<std::string, saga::unexpected<std::string>>{}, "");

static_assert(std::is_constructible<saga::unexpected<std::vector<int>>, std::size_t>{}, "");
static_assert(!std::is_convertible<std::size_t, saga::unexpected<std::vector<int>>>{}, "");

TEST_CASE("unexpected: one argument constructor")
{
    {
        constexpr int value = 42;
        constexpr saga::unexpected<int> err1(value);

        static_assert(err1.value() == value, "");
    }

    {
        constexpr int value = 42;
        constexpr saga::unexpected<long> err1(value);

        static_assert(err1.value() == value, "");
    }

    saga_test::property_checker << [](std::string const & value)
    {
        saga::unexpected<std::string> err1(value);

        REQUIRE(err1.value() == value);
    };
}

namespace
{
    template <class T, class U>
    constexpr T copy_assign_and_return(T lhs, U const & rhs)
    {
        lhs = rhs;

        return lhs;
    }

    template <class T, class U>
    constexpr T move_assign_and_return(T lhs, U rhs)
    {
        lhs = std::move(rhs);

        return lhs;
    }
}

TEST_CASE("unexpected : copy assign")
{
    {
        constexpr saga::unexpected<int> obj1(2020);
        constexpr saga::unexpected<int> obj2(42);

        constexpr auto result = ::copy_assign_and_return(obj1, obj2);
        static_assert(result == obj2, "");
    }

    using Value = int;

    saga_test::property_checker << [](Value const & value_1, Value const & value_2)
    {
        saga::unexpected<Value> const obj1(value_1);
        saga::unexpected<Value> obj2(value_2);

        static_assert(std::is_same<decltype(obj2 = obj1), saga::unexpected<Value>&>{}, "");

        auto const & result = (obj2 = obj1);

        REQUIRE(obj2 == obj1);
        REQUIRE(std::addressof(result) == std::addressof(obj2));
    };
}

TEST_CASE("unexpected : move assign")
{
    {
        constexpr saga::unexpected<int> obj1(2020);
        constexpr saga::unexpected<int> obj2(42);

        constexpr auto result = ::move_assign_and_return(obj1, obj2);
        static_assert(result == obj2, "");
    }

    using Value = int;

    saga_test::property_checker << [](Value const & value_1, Value const & value_2)
    {
        using Unexpected = saga::unexpected<std::unique_ptr<Value>>;

        Unexpected obj1(std::make_unique<Value>(value_1));
        auto const obj1_old_ptr = obj1.value().get();

        Unexpected obj2(std::make_unique<Value>(value_2));

        static_assert(std::is_same<decltype(obj2 = std::move(obj1)), Unexpected &>{}, "");
        auto const & result = (obj2 = std::move(obj1));

        REQUIRE(obj2.value().get() == obj1_old_ptr);
        REQUIRE(obj1.value() == nullptr);
        REQUIRE(std::addressof(result) == std::addressof(obj2));

    };
}

TEST_CASE("unexpected : compatible copy assign")
{
    static_assert(std::is_assignable<long &, int const &>{}, "");
    static_assert(std::is_assignable<saga::unexpected<long> &, saga::unexpected<int> const &>{},"");

    static_assert(!std::is_assignable<int &, std::vector<int>>{},"");
    static_assert(!std::is_assignable<saga::unexpected<int> &,
                                      saga::unexpected<std::vector<int>> const &>{},"");

    {
        constexpr saga::unexpected<long> obj1(2020);
        constexpr saga::unexpected<int> const obj2(34);

        constexpr auto result = ::copy_assign_and_return(obj1, obj2);

        static_assert(result.value() == obj2.value(), "");
    }

    saga_test::property_checker << [](long value1, int value2)
    {
        saga::unexpected<long> obj1(value1);
        saga::unexpected<int> const obj2(value2);

        static_assert(std::is_same<decltype(obj1 = obj2), decltype((obj1))>{}, "");
        auto const & result = (obj1 = obj2);

        REQUIRE(obj1.value() == obj2.value());
        REQUIRE(std::addressof(result) == std::addressof(obj1));
    };
}

namespace
{
    template <class T>
    constexpr saga::unexpected<T> assign_to_value(saga::unexpected<T> obj, T const & new_value)
    {
        obj.value() = new_value;
        return obj;
    }
}

TEST_CASE("unexpected : value() &")
{
    {
        constexpr int init_value = 42;
        constexpr int new_value = 17;

        constexpr saga::unexpected<int> err(init_value);
        constexpr auto new_err = ::assign_to_value(err, new_value);

        static_assert(new_err.value() == new_value, "");
    }

    using Value = std::string;

    saga_test::property_checker << [](Value const & init_value, Value const & new_value)
    {
        saga::unexpected<Value> obj(init_value);

        static_assert(std::is_same<decltype(obj.value()), Value &>{}, "");
        static_assert(noexcept(obj.value()), "");

        obj.value() = new_value;

        auto const & c_ref = obj;
        REQUIRE(c_ref.value() == new_value);
    };
}

namespace
{
    template <class T>
    constexpr T use_constexpr_value(saga::unexpected<T> obj)
    {
        return std::move(obj).value();
    }
}

TEST_CASE("unexpected : value() &&")
{
    {
        constexpr int init_value = 42;

        constexpr saga::unexpected<int> err(init_value);

        constexpr auto sink = ::use_constexpr_value(err);

        static_assert(sink == init_value, "");
    }

    using Value = std::vector<int>;

    saga_test::property_checker << [](Value const & value)
    {
        saga::unexpected<Value> obj(value);

        Value const sink = std::move(obj).value();

        static_assert(std::is_same<decltype(std::move(obj).value()), Value &&>{}, "");
        static_assert(noexcept(std::move(obj).value()), "");

        REQUIRE(sink == value);

        auto const & c_ref = obj;
        REQUIRE(c_ref.value().empty());
    };
}

TEST_CASE("unexpected : value() const &&")
{
    {
        constexpr int init_value = 42;
        constexpr saga::unexpected<int> err(init_value);

        static_assert(std::move(err).value() == init_value, "");
    }

    using Value = int;
    saga_test::property_checker << [](Value const & value)
    {
        saga::unexpected<Value> const obj(value);

        static_assert(std::is_same<decltype(std::move(obj).value()), Value const &&>{}, "");
        static_assert(noexcept(std::move(obj).value()), "");

        REQUIRE(std::move(obj).value() == value);
    };
}

namespace
{
    template <class Value>
    void check_swap_member(Value const & value1, Value const & value2)
    {
        saga::unexpected<Value> err1(value1);
        saga::unexpected<Value> err2(value2);

        auto const err1_old = err1;
        auto const err2_old = err2;

        err1.swap(err2);

        REQUIRE(err1 == err2_old);
        REQUIRE(err2 == err1_old);

        using std::swap;
        static_assert(noexcept(err1.swap(err2))
                      == noexcept(swap(std::declval<Value&>(), std::declval<Value&>())), "");
    }
}

TEST_CASE("unexpected : swap - noexcept(true)")
{
    using Value = int;
    {
        using std::swap;
        static_assert(noexcept(swap(std::declval<Value&>(), std::declval<Value&>())), "");
    }

    saga_test::property_checker << [](Value const & value1, Value const & value2)
    {
        saga::unexpected<Value> err1(value1);
        saga::unexpected<Value> err2(value2);

        auto const err1_old = err1;
        auto const err2_old = err2;

        err1.swap(err2);

        REQUIRE(err1 == err2_old);
        REQUIRE(err2 == err1_old);

        swap(err1, err2);

        REQUIRE(err1 == err1_old);
        REQUIRE(err2 == err2_old);

        static_assert(noexcept(err1.swap(err2)), "");
        static_assert(noexcept(swap(err1, err2)), "");

        static_assert(saga::is_swappable<saga::unexpected<Value>>{}, "");
        static_assert(saga::is_nothrow_swappable<saga::unexpected<Value>>{}, "");
    };
}

namespace
{
    template <class T>
    struct throwing_swap
    {
        explicit throwing_swap(T init_value)
         : value(std::move(init_value))
        {}

        friend bool operator==(throwing_swap const & lhs, throwing_swap const & rhs)
        {
            return lhs.value == rhs.value;
        }

        friend void swap(throwing_swap & lhs, throwing_swap & rhs) noexcept(false)
        {
            using std::swap;
            swap(lhs.value, rhs.value);
        }

        T value;
    };
}

TEST_CASE("unexpected : swap - noexcept(false)")
{
    using Value = ::throwing_swap<int>;
    {
        using std::swap;
        static_assert(!noexcept(swap(std::declval<Value&>(), std::declval<Value&>())), "");
    }

    saga_test::property_checker << [](int const & value1, int const & value2)
    {
        saga::unexpected<Value> err1(saga::in_place_t{}, value1);
        saga::unexpected<Value> err2(saga::in_place_t{}, value2);

        auto const err1_old = err1;
        auto const err2_old = err2;

        err1.swap(err2);

        REQUIRE(err1 == err2_old);
        REQUIRE(err2 == err1_old);

        swap(err1, err2);

        REQUIRE(err1 == err1_old);
        REQUIRE(err2 == err2_old);

        static_assert(!noexcept(err1.swap(err2)), "");
        static_assert(!noexcept(swap(err1, err2)), "");

        static_assert(saga::is_swappable<saga::unexpected<Value>>{}, "");
        static_assert(!saga::is_nothrow_swappable<saga::unexpected<Value>>{}, "");
    };
}

namespace
{
    struct deleted_swap
    {
        deleted_swap() = default;

        deleted_swap(deleted_swap const &) = delete;
        deleted_swap(deleted_swap &&) = delete;
        deleted_swap & operator=(deleted_swap const &) = delete;
        deleted_swap & operator=(deleted_swap &&) = delete;

        friend void swap(deleted_swap &, deleted_swap &) noexcept = delete;
    };

    static_assert(!saga::is_swappable<deleted_swap>{}, "");
    static_assert(!saga::is_swappable<saga::unexpected<deleted_swap>>{}, "");
    static_assert(!saga::is_nothrow_swappable<deleted_swap>{}, "");
    static_assert(!saga::is_nothrow_swappable<saga::unexpected<deleted_swap>>{}, "");
}

TEST_CASE("unexpected : operators == and !=")
{
    {
        constexpr int value1 = 42;
        constexpr long value2 = 13;

        constexpr saga::unexpected<int> lhs(value1);
        constexpr saga::unexpected<long> rhs(value2);

        static_assert(lhs == lhs, "");
        static_assert(rhs == rhs, "");

        static_assert((lhs == rhs) == (value1 == value2), "");
        static_assert((lhs == rhs) == (lhs.value() == rhs.value()), "");
        static_assert((lhs != rhs) == !(lhs == rhs), "");
    }

    saga_test::property_checker << [](int value1, long value2)
    {
        saga::unexpected<int> const lhs(value1);
        saga::unexpected<long> const rhs(value2);

        REQUIRE(lhs == lhs);
        REQUIRE(rhs == rhs);

        REQUIRE((lhs == rhs) == (value1 == value2));
        REQUIRE((lhs == rhs) == (lhs.value() == rhs.value()));
        REQUIRE((lhs != rhs) == !(lhs == rhs));
    };
}

// 6. bad_expected_access
namespace
{
    static_assert(std::is_base_of<saga::bad_expected_access<void>,
                                  saga::bad_expected_access<int>>{}, "");
    static_assert(std::is_base_of<saga::bad_expected_access<void>,
                                  saga::bad_expected_access<std::string>>{}, "");
    static_assert(std::is_convertible<saga::bad_expected_access<int> *,
                                      saga::bad_expected_access<void> *>{}, "");
    static_assert(std::is_convertible<saga::bad_expected_access<std::string> *,
                                      saga::bad_expected_access<void> *>{}, "");
    static_assert(std::is_constructible<saga::bad_expected_access<int>, int>{}, "");
    static_assert(!std::is_convertible<int, saga::bad_expected_access<int>>{}, "");
    static_assert(std::is_constructible<saga::bad_expected_access<std::string>, std::string>{}, "");
    static_assert(!std::is_convertible<std::string, saga::bad_expected_access<std::string>>{}, "");
}

TEST_CASE("bad_expected_access<std::string>")
{
    using Value = std::string;
    saga_test::property_checker << [](Value const & value)
    {
        using Exception = saga::bad_expected_access<Value>;
        Exception exc(value);

        REQUIRE_THAT(std::string(exc.what()), Catch::Matchers::Contains("expected"));

        Exception const & c_ref = exc;

        static_assert(std::is_same<decltype(exc.error()), Value &>{}, "");
        static_assert(std::is_same<decltype(c_ref.error()), Value const &>{}, "");
        static_assert(std::is_same<decltype(std::move(exc).error()), Value &&>{}, "");

        REQUIRE(exc.error() == value);
        REQUIRE(c_ref.error() == value);
        REQUIRE(std::addressof(c_ref.error()) == std::addressof(exc.error()));

        auto const old_value = exc.error();

        auto sink = std::move(exc).error();

        REQUIRE(sink == old_value);
    };
}

TEST_CASE("bad_expected_access<std::string> const &&")
{
    using Value = std::string;
    saga_test::property_checker << [](Value const & value)
    {
        using Exception = saga::bad_expected_access<Value>;
        Exception const exc(value);

        static_assert(std::is_same<decltype(std::move(exc).error()), Value const &&>{}, "");

        REQUIRE(exc.error() == value);

        Value const && sink = std::move(exc).error();

        REQUIRE(std::addressof(sink) == std::addressof(exc.error()));
    };
}

// 7. bad_expected_access<void>
namespace
{
    static_assert(std::is_base_of<std::exception, saga::bad_expected_access<void>>{}, "");
    static_assert(std::is_convertible<saga::bad_expected_access<void> *, std::exception *>{}, "");

    static_assert(std::is_default_constructible<saga::bad_expected_access<void>>{}, "");
    static_assert(std::is_nothrow_default_constructible<saga::bad_expected_access<void>>{}, "");
}

TEST_CASE("bad_expected_access<void>")
{
    saga::bad_expected_access<void> const exc{};

    REQUIRE(std::string(exc.what()) == std::string(std::exception().what()));
}

// 8. Тэг unexpect
namespace
{
    static_assert(std::is_trivially_default_constructible<saga::unexpect_t>{}, "");
    static_assert(std::is_nothrow_default_constructible<saga::unexpect_t>{}, "");

    constexpr auto tag1 = saga::unexpect_t{};
    constexpr auto tag2 = saga::unexpect;

    static_assert(std::is_same<decltype(tag1), saga::unexpect_t const>{}, "");
    static_assert(std::is_same<decltype(tag2), saga::unexpect_t const>{}, "");
}
