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

// 4. @todo Шаблона класса expected
// @todo ограничение размера expected

// Типы
namespace
{
    template <class Value, class Error, class OtherValue>
    void check_expected_types()
    {
        using Expected = saga::expected<Value, Error>;

        static_assert(std::is_same<typename Expected::value_type, Value>{}, "");
        static_assert(std::is_same<typename Expected::error_type, Error>{}, "");
        static_assert(std::is_same<typename Expected::unexpected_type,
                                   saga::unexpected<Error>>{}, "");

        static_assert(std::is_same<typename Expected::template rebind<OtherValue>,
                                   saga::expected<OtherValue, Error>>{}, "");
    }
}

TEST_CASE("expected : types")
{
    ::check_expected_types<std::string, long, double>();
    ::check_expected_types<void, long, std::string>();
}

// 4.1 @todo Конструкторы
// Конструктор без аргументов
static_assert(std::is_default_constructible<saga::expected<double, long>>{}, "");
static_assert(std::is_default_constructible<saga::expected<void, long>>{}, "");
static_assert(std::is_default_constructible<saga::expected<std::string, long>>{}, "");
static_assert(std::is_default_constructible<saga::expected<double, std::string>>{}, "");
static_assert(std::is_default_constructible<saga::expected<std::vector<int>, std::string>>{}, "");

namespace
{
    struct no_default_ctor
    {
        no_default_ctor() = delete;
    };
}
static_assert(!std::is_default_constructible<::no_default_ctor>{}, "");
static_assert(!std::is_default_constructible<saga::expected<::no_default_ctor, long>>{}, "");

TEST_CASE("expected<Value, Error> : default ctor")
{
    {
        using Value = long *;
        using Error = int;

        constexpr saga::expected<Value, Error> const obj{};

        static_assert(obj.has_value(), "");
        static_assert(static_cast<bool>(obj), "");
        static_assert(obj.value() == Value{}, "");
    }
    {
        using Value = std::string;
        using Error = long;

        saga::expected<Value, Error> const obj{};

        REQUIRE(obj.has_value());
        REQUIRE(obj.value() == Value{});
    }
    {
        using Value = int;
        using Error = std::string;

        saga::expected<Value, Error> const obj{};

        REQUIRE(obj.has_value());
        REQUIRE(obj.value() == Value{});
    }
    {
        using Value = std::string;
        using Error = std::vector<int>;

        saga::expected<Value, Error> const obj{};

        REQUIRE(obj.has_value());
        REQUIRE(obj.value() == Value{});
    }
}

namespace
{
    template <class T>
    constexpr bool constexpr_check_void_value(T const & obj)
    {
        obj.value();

        using Result = decltype(obj.value());
        static_assert(std::is_same<Result, void>{}, "");

        return true;
    }
}

TEST_CASE("expected<void, Error> : default ctor")
{
    {
        using Value = void;
        using Error = long;

        constexpr saga::expected<Value, Error> const obj{};

        static_assert(obj.has_value(), "");
        static_assert(::constexpr_check_void_value(obj), "");
    }
    {
        using Value = void;
        using Error = std::string;

        saga::expected<Value, Error> const obj{};

        REQUIRE(obj.has_value());
        REQUIRE_NOTHROW(obj.value());
    }
}

// Конструктор с in_place_t
// @todo Устранить дублирование с unexpected<T>?
// Есть конструктор с in_place_t и любым числом аргументов
static_assert(std::is_constructible<std::vector<int>, std::size_t, int>{}, "");
static_assert(std::is_constructible<saga::expected<std::vector<int>, std::string>,
                                    saga::in_place_t, std::size_t, int>{}, "");

// Конструктор с in_place_t является explicit:
static_assert(std::is_constructible<std::vector<int>>{}, "");
static_assert(std::is_constructible<saga::expected<std::vector<int>, std::string>,
                                    saga::in_place_t>{}, "");
static_assert(!std::is_convertible<saga::in_place_t,
                                   saga::expected<std::vector<int>, std::string>>{}, "");

// Конструктор с in_place_t не участвует в разрешение перегрузки, когда его нельзя вызывать
static_assert(!std::is_constructible<int, std::vector<int>>{}, "");
static_assert(!std::is_constructible<saga::expected<int, std::string>,
                                     saga::in_place_t, std::vector<int>>{}, "");

static_assert(!std::is_constructible<int, std::initializer_list<int>&, int>{}, "");
static_assert(!std::is_constructible<saga::expected<int, std::string>,
                                     saga::in_place_t, std::initializer_list<int>&, int>{}, "");

// expected<cv void, Error> нельзя создать, если передать in_place_t дополнительные аргументы
static_assert(std::is_constructible<saga::expected<void, int>, saga::in_place_t>{}, "");
static_assert(!std::is_constructible<saga::expected<void, int>, saga::in_place_t, int>{}, "");
static_assert(!std::is_constructible<saga::expected<const void, int>, saga::in_place_t, int>{}, "");
static_assert(!std::is_constructible<saga::expected<volatile void, int>,
                                     saga::in_place_t, int>{}, "");
static_assert(!std::is_constructible<saga::expected<const volatile void, int>,
                                     saga::in_place_t, int>{}, "");

static_assert(!std::is_constructible<saga::expected<void, int>,
                                     saga::in_place_t, std::initializer_list<int>>{}, "");
static_assert(!std::is_constructible<saga::expected<const void, int>,
                                     saga::in_place_t, std::initializer_list<int>>{}, "");
static_assert(!std::is_constructible<saga::expected<volatile void, int>,
                                     saga::in_place_t, std::initializer_list<int>>{}, "");
static_assert(!std::is_constructible<saga::expected<const volatile void, int>,
                                     saga::in_place_t, std::initializer_list<int>>{}, "");

TEST_CASE("expected<void, Error>: placement constructor")
{
    constexpr saga::expected<void, long> obj0(saga::in_place_t{});

    static_assert(obj0.has_value(), "");
    static_assert(noexcept(obj0.has_value()), "");
    static_assert((obj0.value(), true), "");
}

TEST_CASE("expected: placement constructor")
{
    {
        constexpr saga::expected<int, long*> obj0(saga::in_place_t{});

        static_assert(obj0.has_value(), "");
        static_assert(obj0.value() == 0, "");
        static_assert(noexcept(obj0.has_value()), "");

        constexpr int value = 42;
        constexpr saga::expected<int, long*> obj1(saga::in_place_t{}, value);
        static_assert(obj1.has_value(), "");
        static_assert(obj1.value() == value, "");
    }

    saga_test::property_checker << [](saga_test::container_size<std::size_t> num, int filler)
    {
        std::vector<int> const expected(num, filler);

        saga::expected<std::vector<int>, std::string> const actual(saga::in_place_t{}, num, filler);

        REQUIRE(actual.has_value());
        REQUIRE(actual.value() == expected);
    };
}

TEST_CASE("expected: placement constructor with initializer list")
{
    saga_test::property_checker << [](int value1, int value2)
    {
        std::vector<int> const expected{value1, value2};

        saga::expected<std::vector<int>, std::string> const
            actual(saga::in_place_t{}, {value1, value2});

        REQUIRE(actual.has_value());
        REQUIRE(actual.value() == expected);
    };
}

TEST_CASE("expected: placement constructor with initializer list and more args")
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

        constexpr saga::expected<initializer_list_consumer, long>
            obj(saga::in_place_t{}, {1, 2, 3, 4}, 5);

        static_assert(obj.has_value(), "");
        static_assert(obj.value().value == 15, "");
    }

    saga_test::property_checker << [](int value1, int value2)
    {
        using Compare = bool(*)(int const &, int const &);
        using Container = std::set<int, Compare>;

        auto const cmp = Compare([](int const & x, int const & y) { return x < y; });

        Container const expected({value1, value2}, cmp);

        saga::expected<Container, std::string> const
            actual(saga::in_place_t{}, {value1, value2}, cmp);

        REQUIRE(actual.has_value());
        REQUIRE(actual.value() == expected);
    };
}

// Конструктор на основе unexpect_t
TEST_CASE("expected<Value, Error>: ctor from unexpect_t")
{
    {
        using Value = int;
        using Error = long *;

        constexpr saga::expected<Value, Error> const obj(saga::unexpect);

        static_assert(obj.has_value() == false, "");
        static_assert(obj.error() == Error{}, "");
    }
    {
        using Value = void;
        using Error = std::string;

        saga::expected<Value, Error> const obj(saga::unexpect);

        REQUIRE(obj.has_value() == false);
        REQUIRE(obj.error() == Error{});

        REQUIRE_THROWS_AS(obj.value(), saga::bad_expected_access<Error>);
    }
    {
        using Value = std::string;
        using Error = long;

        saga::expected<Value, Error> const obj(saga::unexpect);

        REQUIRE(obj.has_value() == false);
        REQUIRE(obj.error() == Error{});

        REQUIRE_THROWS_AS(obj.value(), saga::bad_expected_access<Error>);
    }
    {
        using Value = std::vector<int>;
        using Error = std::string;

        saga::expected<Value, Error> const obj(saga::unexpect);

        REQUIRE(obj.has_value() == false);
        REQUIRE(obj.error() == Error{});

        REQUIRE_THROWS_AS(obj.value(), saga::bad_expected_access<Error>);
    }
}

TEST_CASE("expected<void, Error>: ctor from unexpect_t")
{
    {
        using Value = void;
        using Error = long;

        constexpr saga::expected<Value, Error> const obj(saga::unexpect);

        static_assert(obj.has_value() == false, "");
        static_assert(obj.error() == Error{}, "");
    }
    {
        using Value = void;
        using Error = std::string;

        saga::expected<Value, Error> const obj(saga::unexpect);

        REQUIRE(obj.has_value() == false);
        REQUIRE(obj.error() == Error{});

        REQUIRE_THROWS_AS(obj.value(), saga::bad_expected_access<Error>);
    }
}

// Есть конструктор с unexpect_t и любым числом аргументов
static_assert(std::is_constructible<std::vector<int>, std::size_t, int>{}, "");
static_assert(std::is_constructible<saga::expected<std::string, std::vector<int>>,
                                    saga::unexpect_t, std::size_t, int>{}, "");
static_assert(std::is_constructible<saga::expected<void, std::vector<int>>,
                                    saga::unexpect_t, std::size_t, int>{}, "");

// Конструктор с unexpect_t является explicit:
static_assert(std::is_constructible<std::vector<int>>{}, "");
static_assert(std::is_constructible<saga::expected<std::string, std::vector<int>>,
                                    saga::unexpect_t>{}, "");
static_assert(!std::is_convertible<saga::unexpect_t,
                                   saga::expected<std::string, std::vector<int>>>{}, "");

// Конструктор с unexpect_t не участвует в разрешение перегрузки, когда его нельзя вызывать
static_assert(!std::is_constructible<int, std::vector<int>>{}, "");
static_assert(!std::is_constructible<saga::expected<std::string, int>,
                                     saga::unexpect_t, std::vector<int>>{}, "");

static_assert(!std::is_constructible<int, std::initializer_list<int>&, int>{}, "");
static_assert(!std::is_constructible<saga::expected<std::string, int>,
                                     saga::unexpect_t, std::initializer_list<int>&, int>{}, "");

TEST_CASE("expected: unexpect_t constructor")
{
    {
        constexpr saga::expected<int, long*> obj0(saga::unexpect);

        static_assert(obj0.has_value() == false, "");
        static_assert(obj0.error() == 0, "");

        constexpr int value = 42;
        constexpr saga::expected<long *, int> obj1(saga::unexpect, value);
        static_assert(obj1.has_value() == false, "");
        static_assert(obj1.error() == value, "");
    }
    {
        constexpr saga::expected<void, long*> obj0(saga::unexpect);

        static_assert(obj0.has_value() == false, "");
        static_assert(obj0.error() == 0, "");

        constexpr int value = 42;
        constexpr saga::expected<long *, int> obj1(saga::unexpect, value);
        static_assert(obj1.has_value() == false, "");
        static_assert(obj1.error() == value, "");
    }

    saga_test::property_checker << [](saga_test::container_size<std::size_t> num, int filler)
    {
        std::vector<int> const expected(num, filler);

        saga::expected<std::string, std::vector<int>> const actual1(saga::unexpect, num, filler);
        saga::expected<void, std::vector<int>> const actual2(saga::unexpect, num, filler);

        REQUIRE(!actual1.has_value());
        REQUIRE(actual1.error() == expected);

        REQUIRE(!actual2.has_value());
        REQUIRE(actual2.error() == expected);

        try
        {
            actual1.value();
        }
        catch(saga::bad_expected_access<std::vector<int>> & err)
        {
            REQUIRE(err.error() == expected);
        }

        try
        {
            actual2.value();
        }
        catch(saga::bad_expected_access<std::vector<int>> & err)
        {
            REQUIRE(err.error() == expected);
        }
    };
}

TEST_CASE("expected: unexpect_t constructor with initializer list")
{
    saga_test::property_checker << [](int value1, int value2)
    {
        std::vector<int> const expected{value1, value2};

        saga::expected<std::string, std::vector<int>> const actual1(saga::unexpect, {value1, value2});
        saga::expected<void, std::vector<int>> const actual2(saga::unexpect, {value1, value2});

        REQUIRE(!actual1.has_value());
        REQUIRE(actual1.error() == expected);

        REQUIRE(!actual2.has_value());
        REQUIRE(actual2.error() == expected);

        try
        {
            actual1.value();
        }
        catch(saga::bad_expected_access<std::vector<int>> & err)
        {
            REQUIRE(err.error() == expected);
        }

        try
        {
            actual2.value();
        }
        catch(saga::bad_expected_access<std::vector<int>> & err)
        {
            REQUIRE(err.error() == expected);
        }
    };
}

TEST_CASE("expected: unexpect constructor with initializer list and more args")
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

        constexpr saga::expected<long, initializer_list_consumer>
            obj(saga::unexpect, {1, 2, 3, 4}, 5);

        static_assert(!obj.has_value(), "");
        static_assert(obj.error().value == 15, "");
    }
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

        constexpr saga::expected<void, initializer_list_consumer>
            obj(saga::unexpect, {1, 2, 3, 4}, 5);

        static_assert(!obj.has_value(), "");
        static_assert(obj.error().value == 15, "");
    }

    saga_test::property_checker << [](int value1, int value2)
    {
        using Compare = bool(*)(int const &, int const &);
        using Container = std::set<int, Compare>;

        auto const cmp = Compare([](int const & x, int const & y) { return x < y; });

        Container const expected({value1, value2}, cmp);

        saga::expected<std::string, Container> const actual1(saga::unexpect, {value1, value2}, cmp);
        saga::expected<void, Container> const actual2(saga::unexpect, {value1, value2}, cmp);

        REQUIRE(!actual1.has_value());
        REQUIRE(actual1.error() == expected);

        REQUIRE(!actual2.has_value());
        REQUIRE(actual2.error() == expected);

        try
        {
            actual1.value();
        }
        catch(saga::bad_expected_access<Container> & err)
        {
            REQUIRE(err.error() == expected);
        }

        try
        {
            actual2.value();
        }
        catch(saga::bad_expected_access<Container> & err)
        {
            REQUIRE(err.error() == expected);
        }
    };
}

// 4.5 Свойства
namespace
{
    template <class Expected>
    void check_expected_with_error_value_throws(Expected && obj)
    {
        using Error = typename saga::remove_cvref_t<Expected>::error_type;

        assert(!obj.has_value());
        auto const error_old = obj.error();

        try
        {
            std::forward<Expected>(obj).value();
        }
        catch(saga::bad_expected_access<Error> & exc)
        {
            REQUIRE(exc.error() == error_old);
        }
    }

    template <class Value, class Error>
    void observers_property(saga::expected<Value, Error> & obj, Error const & error)
    {
        auto const & c_ref = obj;

        static_assert(!std::is_convertible<saga::expected<Value, Error>, bool>{}, "");
        static_assert(noexcept(c_ref.has_value()), "");
        static_assert(noexcept(static_cast<bool>(c_ref)), "");

        static_assert(std::is_same<decltype(obj.error()), Error &>{}, "");
        static_assert(std::is_same<decltype(c_ref.error()), Error const &>{}, "");
        static_assert(std::is_same<decltype(std::move(obj).error()), Error &&>{}, "");
        static_assert(std::is_same<decltype(std::move(c_ref).error()), Error const &&>{}, "");

        REQUIRE(c_ref.has_value() == static_cast<bool>(c_ref));

        if(c_ref.has_value())
        {
            obj.value();
            c_ref.value();
            std::move(obj).value();
            std::move(c_ref).value();
        }
        else
        {
            REQUIRE(obj.error() == error);
            REQUIRE(std::addressof(c_ref.error()) == std::addressof(obj.error()));

            auto && rvalue_error = std::move(obj).error();
            auto const && crvalue_error = std::move(c_ref).error();

            REQUIRE(std::addressof(rvalue_error) == std::addressof(obj.error()));
            REQUIRE(std::addressof(crvalue_error) == std::addressof(obj.error()));

            ::check_expected_with_error_value_throws(obj);
            ::check_expected_with_error_value_throws(c_ref);
            ::check_expected_with_error_value_throws(std::move(obj));
            ::check_expected_with_error_value_throws(std::move(c_ref));
        }
    }

    template <class Value, class Error>
    void observers_property_not_void(saga::expected<Value, Error> & obj,
                                     Value const & value, Error const & error)
    {
        ::observers_property(obj, error);

        auto const & c_ref = obj;

        static_assert(std::is_same<decltype(obj.operator->()), Value *>{}, "");
        static_assert(std::is_same<decltype(c_ref.operator->()), Value const *>{}, "");

        static_assert(std::is_same<decltype(*obj), Value &>{}, "");
        static_assert(std::is_same<decltype(*c_ref), Value const &>{}, "");
        static_assert(std::is_same<decltype(*std::move(obj)), Value &&>{}, "");
        static_assert(std::is_same<decltype(*std::move(c_ref)), Value const &&>{}, "");

        static_assert(std::is_same<decltype(obj.value()), Value &>{}, "");
        static_assert(std::is_same<decltype(c_ref.value()), Value const &>{}, "");
        static_assert(std::is_same<decltype(std::move(obj).value()), Value &&>{}, "");
        static_assert(std::is_same<decltype(std::move(c_ref).value()), Value const &&>{}, "");

        if(c_ref.has_value())
        {
            REQUIRE(obj.operator->() == std::addressof(obj.value()));
            REQUIRE(c_ref.operator->() == std::addressof(obj.value()));
            REQUIRE(c_ref.operator->() == std::addressof(c_ref.value()));

            REQUIRE(std::addressof(*obj) == std::addressof(obj.value()));
            REQUIRE(std::addressof(*c_ref) == std::addressof(obj.value()));

            auto && rvalue_derefenece = *std::move(obj);
            auto const && crvalue_dereference = *std::move(c_ref);

            REQUIRE(std::addressof(rvalue_derefenece) == std::addressof(obj.value()));
            REQUIRE(std::addressof(crvalue_dereference) == std::addressof(obj.value()));

            REQUIRE(obj.value() == value);
            REQUIRE(std::addressof(c_ref.value()) == std::addressof(obj.value()));

            auto && rvalue_value = std::move(obj).value();
            auto const && crvalue_value = std::move(c_ref).value();

            REQUIRE(std::addressof(rvalue_value) == std::addressof(obj.value()));
            REQUIRE(std::addressof(crvalue_value) == std::addressof(obj.value()));
        }
        else
        {}
    }

    template <class Value, class Error, class OtherValue>
    void check_expected_value_or(saga::expected<Value, Error> const & obj, OtherValue other)
    {
        if(obj.has_value())
        {
            REQUIRE(obj.value_or(other) == obj.value());
        }
        else
        {
            REQUIRE(obj.value_or(other) == other);
        }
    }

    template <class Error, class Value>
    constexpr Value
    check_constexpr_value_ref(Value old_value, Value new_value)
    {
        saga::expected<Value, Error> obj(saga::in_place_t{}, old_value);

        obj.value() = new_value;

        return obj.value();
    }

    template <class Value, class Error>
    constexpr Error
    check_constexpr_error_ref(Error old_error, Error new_error)
    {
        saga::expected<Value, Error> obj(saga::unexpect, old_error);

        obj.error() = new_error;

        return obj.error();
    }
}

TEST_CASE("expected: observers")
{
    using Value = std::vector<int>;
    using Error = std::string;

    saga_test::property_checker << [](Value const & value, Error const & error)
    {
        saga::expected<Value, Error> obj_value(saga::in_place_t{}, value);
        saga::expected<Value, Error> obj_error(saga::unexpect_t{}, error);

        saga::expected<void, Error> obj_void_value(saga::in_place_t{});
        saga::expected<void, Error> obj_void_error(saga::unexpect_t{}, error);

        ::observers_property_not_void(obj_value, value, error);
        ::observers_property_not_void(obj_error, value, error);
        ::observers_property(obj_void_value, error);
        ::observers_property(obj_void_error, error);
    };

    {
        using Value = long;
        using Error = int;
        using Expected = saga::expected<Value, Error>;

        constexpr auto value = Value(42);
        constexpr auto error = Error(0);

        static_assert(Expected(saga::in_place_t{}, value).value() == value, "");
        static_assert(Expected(saga::unexpect, error).error() == error, "");

        constexpr Expected obj_value(saga::in_place_t{}, value);
        static_assert(std::move(obj_value).value() == value, "");

        constexpr Expected obj_error(saga::unexpect, error);
        static_assert(std::move(obj_error).error() == error, "");

        constexpr auto value_new = Value(1);
        constexpr auto error_new = Error(1);

        static_assert(::check_constexpr_value_ref<Error>(value, value_new) == value_new, "");
        static_assert(::check_constexpr_error_ref<Value>(error, error_new) == error_new, "");
    }
}

TEST_CASE("expected: value_or() const &")
{
    using Value = long;
    using Error = std::string;
    using OtherValue = int;

    saga_test::property_checker
    << [](Value const & value, Error const & error, OtherValue const & other)
    {
        saga::expected<Value, Error> const obj_value(saga::in_place_t{}, value);
        saga::expected<Value, Error> const obj_error(saga::unexpect_t{}, error);

        ::check_expected_value_or(obj_value, other);
        ::check_expected_value_or(obj_error, other);
    };

    {
        using Value = long;
        using Error = void *;
        using Expected = saga::expected<Value, Error>;

        constexpr auto value = Value(42);
        constexpr auto error = nullptr;
        constexpr auto other_value = 13;

        constexpr saga::expected<Value, Error> const obj_value(saga::in_place_t{}, value);
        constexpr saga::expected<Value, Error> const obj_error(saga::unexpect_t{}, error);

        static_assert(obj_value.value_or(other_value) == value, "");
        static_assert(obj_error.value_or(other_value) == other_value, "");

        static_assert(Expected(saga::in_place_t{}, value).value_or(other_value) == value, "");
        static_assert(Expected(saga::unexpect_t{}, error).value_or(other_value) == other_value, "");
    }
}

TEST_CASE("expected: value_or() &&")
{
    using Value = std::vector<int>;
    using Error = std::string;

    saga_test::property_checker
    << [](Value const & value, Error const & error, Value const & other)
    {
        {
            saga::expected<Value, Error> obj_value(saga::in_place_t{}, value);

            auto const result_1 = std::move(obj_value).value_or(other);

            static_assert(std::is_same<decltype(result_1), Value const>{}, "");

            REQUIRE(result_1 == value);
            REQUIRE(obj_value.value().empty());
        }
        {
            saga::expected<Value, Error> obj_error(saga::unexpect_t{}, error);

            {
                auto src = other;
                auto const result_2 = std::move(obj_error).value_or(std::move(src));

                static_assert(std::is_same<decltype(result_2), Value const>{}, "");
                REQUIRE(result_2 == other);
                REQUIRE(src.empty());

                REQUIRE(!obj_error.has_value());
                REQUIRE(obj_error.error() == error);
            }
            {
                auto src = other;
                auto const result_2 = obj_error.value_or(std::move(src));

                static_assert(std::is_same<decltype(result_2), Value const>{}, "");
                REQUIRE(result_2 == other);
                REQUIRE(src.empty());

                REQUIRE(!obj_error.has_value());
                REQUIRE(obj_error.error() == error);
            }
        }
    };
}

// 5. Шаблон класса unexpected
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

        constexpr saga::unexpected<initializer_list_consumer>
            unex(saga::in_place_t{}, {1, 2, 3, 4}, 5);

        static_assert(unex.value().value == 15, "");
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
    // Нет конструктора для несовместимого
    static_assert(!std::is_constructible<int, std::vector<int>>{}, "");
    static_assert(!std::is_constructible<saga::unexpected<int>,
                                         saga::unexpected<std::vector<int>>>{}, "");

    struct explicit_ctor_from_int
    {
        constexpr explicit explicit_ctor_from_int(int value)
         : value(value)
        {}

        int value;
    };
}

TEST_CASE("unexpected : copy constructor from compatible unexpected - explicit")
{
    // Конструктор из совместимного unexpected - явный
    static_assert(std::is_constructible<explicit_ctor_from_int, int const &>{}, "");
    static_assert(std::is_constructible<saga::unexpected<explicit_ctor_from_int>,
                                        saga::unexpected<int> const &>{}, "");
    static_assert(!std::is_convertible<int const &, explicit_ctor_from_int>{}, "");
    static_assert(!std::is_convertible<saga::unexpected<int> const &,
                                       saga::unexpected<explicit_ctor_from_int>>{}, "");

    {
        constexpr saga::unexpected<int> src(42);
        constexpr saga::unexpected<explicit_ctor_from_int> obj(src);

        static_assert(obj.value().value == src.value(), "");
    }

    static_assert(std::is_constructible<std::vector<int>, std::size_t const &>{}, "");
    static_assert(std::is_constructible<saga::unexpected<std::vector<int>>,
                                        saga::unexpected<std::size_t> const &>{}, "");
    static_assert(!std::is_convertible<std::size_t const &, std::vector<int>>{}, "");
    static_assert(!std::is_convertible<saga::unexpected<std::size_t> const &,
                                       saga::unexpected<std::vector<int>>>{}, "");

    saga_test::property_checker << [](saga_test::container_size<std::size_t> num)
    {
        saga::unexpected<std::size_t> const elem_count(num);
        saga::unexpected<std::vector<int>> const err(elem_count);

        REQUIRE(err.value() == std::vector<int>(num));
    };
}

TEST_CASE("unexpected : copy constructor from compatible unexpected - implicit")
{
    using Value1 = int;
    using Value2 = long;

    static_assert(std::is_constructible<Value2, Value1>{}, "");
    static_assert(std::is_convertible<Value1, Value2>{}, "");

    static_assert(std::is_constructible<saga::unexpected<Value2>, saga::unexpected<Value1>>{}, "");
    static_assert(std::is_convertible<saga::unexpected<Value1>, saga::unexpected<Value2>>{}, "");

    {
        constexpr Value1 value{42};
        constexpr saga::unexpected<Value1> const obj1(value);

        constexpr saga::unexpected<Value2> const obj2(obj1);

        constexpr auto const value2 = Value2(value);

        static_assert(obj2.value() == value2, "");
    }

    saga_test::property_checker << [](Value1 const & value)
    {
        saga::unexpected<Value1> const obj1(value);

        saga::unexpected<Value2> const obj2(obj1);

        auto const value2 = Value2(value);

        REQUIRE(obj2.value() == value2);
    };
}

namespace
{
    class Base
    {
    public:
        virtual ~Base() {};
    };

    class Derived
     : public Base
    {};
}

TEST_CASE("unexpected : compatrible move ctor - explciit")
{
    // Конструктор из совместимного unexpected - явный
    static_assert(std::is_constructible<explicit_ctor_from_int, int>{}, "");
    static_assert(std::is_constructible<saga::unexpected<explicit_ctor_from_int>,
                                        saga::unexpected<int>>{}, "");
    static_assert(!std::is_convertible<int, explicit_ctor_from_int>{}, "");
    static_assert(!std::is_convertible<saga::unexpected<int>,
                                       saga::unexpected<explicit_ctor_from_int>>{}, "");

    {
        constexpr int value = 42;
        constexpr saga::unexpected<explicit_ctor_from_int> obj{saga::unexpected<int>(value)};

        static_assert(obj.value().value == value, "");
    }

    static_assert(std::is_constructible<std::vector<int>, std::size_t>{}, "");
    static_assert(std::is_constructible<saga::unexpected<std::vector<int>>,
                                        saga::unexpected<std::size_t>>{}, "");
    static_assert(!std::is_convertible<std::size_t, std::vector<int>>{}, "");
    static_assert(!std::is_convertible<saga::unexpected<std::size_t>,
                                       saga::unexpected<std::vector<int>>>{}, "");

    saga_test::property_checker << [](saga_test::container_size<std::size_t> num)
    {
        saga::unexpected<std::size_t> const elem_count(num);
        saga::unexpected<std::vector<int>> const err(elem_count);

        REQUIRE(err.value() == std::vector<int>(num));
    };
}

TEST_CASE("unexpected : compatible move ctor - implicit")
{
    static_assert(std::is_constructible<std::unique_ptr<Base>, std::unique_ptr<Derived>>{}, "");
    static_assert(std::is_assignable<saga::unexpected<std::unique_ptr<Base>>,
                                     saga::unexpected<std::unique_ptr<Derived>>>{}, "");
    static_assert(!std::is_constructible<std::unique_ptr<long>, std::unique_ptr<int>>{}, "");
    static_assert(!std::is_constructible<saga::unexpected<std::unique_ptr<long>>,
                                         saga::unexpected<std::unique_ptr<int>>>{}, "");

    {
        constexpr int value = 2020;
        constexpr saga::unexpected<long> obj{saga::unexpected<int>(value)};

        static_assert(obj.value() == value, "");
    }
    {
        saga::unexpected<std::unique_ptr<Derived>> src(std::make_unique<Derived>());
        auto const src_ptr = src.value().get();

        saga::unexpected<std::unique_ptr<Base>> const obj(std::move(src));

        REQUIRE(obj.value().get() == src_ptr);
        REQUIRE(src.value().get() == nullptr);
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

TEST_CASE("unexpected : compatible move assign")
{
    static_assert(std::is_assignable<std::unique_ptr<Base> &, std::unique_ptr<Derived>>{}, "");
    static_assert(std::is_assignable<saga::unexpected<std::unique_ptr<Base>> &,
                                     saga::unexpected<std::unique_ptr<Derived>>>{}, "");
    static_assert(!std::is_assignable<std::unique_ptr<long> &, std::unique_ptr<int>>{}, "");
    static_assert(!std::is_assignable<saga::unexpected<std::unique_ptr<long>> &,
                                      saga::unexpected<std::unique_ptr<int>>>{}, "");

    {
        constexpr saga::unexpected<long> obj1(2505);
        constexpr saga::unexpected<int> obj2(2020);

        constexpr auto result = ::move_assign_and_return(obj1, obj2);

        static_assert(result == obj2, "");
    }
    {
        saga::unexpected<std::unique_ptr<Base>> obj1(saga::in_place_t{});
        saga::unexpected<std::unique_ptr<Derived>> obj2(std::make_unique<Derived>());

        REQUIRE(obj1.value().get() == nullptr);
        REQUIRE(obj2.value().get() != nullptr);

        auto const obj2_ptr = obj2.value().get();

        static_assert(std::is_same<decltype(obj1 = std::move(obj2)), decltype((obj1))>{}, "");
        auto const & result = (obj1 = std::move(obj2));

        REQUIRE(obj2.value().get() == nullptr);
        REQUIRE(obj1.value().get() == obj2_ptr);

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

// Вывод шаблонных параметров
#if __cpp_deduction_guides >= 201703
TEST_CASE("unexpected : CTAD")
{
    using Value = long;

    saga_test::property_checker << [](Value const & value)
    {
        saga::unexpected const obj(value);

        static_assert(std::is_same<decltype(obj), saga::unexpected<Value> const>{}, "");

        REQUIRE(obj.value() == value);
    };
}
#endif
// __cpp_deduction_guides

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
