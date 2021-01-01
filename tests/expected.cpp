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

// Тестируемый файл
#include <saga/expected.hpp>

// Тестовая инфраструктура
#include "./saga_test.hpp"
#include <catch/catch.hpp>
#include <type_traits>

// Вспомогательные файлы
#include <set>

using namespace saga_test;

namespace
{
    struct no_default_ctor
    {
        no_default_ctor() = delete;
    };
}

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

static_assert(!std::is_default_constructible<::no_default_ctor>{}, "");
static_assert(!std::is_default_constructible<saga::expected<::no_default_ctor, long>>{}, "");

namespace
{
    template <class Value, class Error, std::enable_if_t<!std::is_void<Value>{}> * = nullptr>
    void check_expected_default_ctor()
    {
        saga::expected<Value, Error> const obj{};
        REQUIRE(obj.has_value());
        REQUIRE(obj.value() == Value{});
    }

    template <class Value, class Error, std::enable_if_t<std::is_void<Value>{}> * = nullptr>
    void check_expected_default_ctor()
    {
        saga::expected<Value, Error> const obj{};

        REQUIRE(obj.has_value());
        REQUIRE_NOTHROW(obj.value());

        using Result = decltype(obj.value());
        static_assert(std::is_same<Result, void>{}, "");
    }

    template <class Value, class Error, std::enable_if_t<!std::is_void<Value>{}> * = nullptr>
    constexpr bool constexpr_check_expected_default_ctor()
    {
        constexpr saga::expected<Value, Error> obj{};

        static_assert(obj.has_value(), "");
        static_assert(obj.value() == Value{}, "");

        return true;
    }

    template <class Value, class Error, std::enable_if_t<std::is_void<Value>{}> * = nullptr>
    constexpr bool constexpr_check_expected_default_ctor()
    {
        constexpr saga::expected<Value, Error> obj{};

        static_assert(obj.has_value(), "");

        obj.value();

        using Result = decltype(obj.value());
        static_assert(std::is_same<Result, void>{}, "");

        return true;
    }
}

// @todo  Убедиться, что есть проверка obj.has_value() == static_cast<bool>(obj)

TEST_CASE("expected<Value, Error> : default ctor")
{
    ::constexpr_check_expected_default_ctor<long *, int>();
    ::constexpr_check_expected_default_ctor<long, int>();

    ::check_expected_default_ctor<int *, long>();
    ::check_expected_default_ctor<std::string, long>();
    ::check_expected_default_ctor<int, std::string>();
    ::check_expected_default_ctor<std::string, std::vector<int>>();
}

TEST_CASE("expected<void, Error> : constexpr default ctor")
{
    ::constexpr_check_expected_default_ctor<void, long>();
    ::constexpr_check_expected_default_ctor<void const, long>();
    ::constexpr_check_expected_default_ctor<void volatile, long>();
    ::constexpr_check_expected_default_ctor<void const volatile, long>();

    ::check_expected_default_ctor<void, long>();
    ::check_expected_default_ctor<void, std::string>();
    ::check_expected_default_ctor<void const, long>();
    ::check_expected_default_ctor<void const, std::string>();
    ::check_expected_default_ctor<void volatile, long>();
    ::check_expected_default_ctor<void volatile, std::string>();
    ::check_expected_default_ctor<void const volatile, long>();
    ::check_expected_default_ctor<void const volatile, std::string>();
}

// @todo Конструктор с одним аргументом-значением
static_assert(std::is_constructible<long, int>{}, "");
static_assert(std::is_convertible<int, long>{}, "");
static_assert(std::is_constructible<saga::expected<long, std::string>, int>{}, "");
static_assert(std::is_convertible<int, saga::expected<long, std::string>>{}, "");

static_assert(std::is_constructible<std::vector<int>, std::size_t>{}, "");
static_assert(!std::is_convertible<std::size_t, std::vector<int>>{}, "");
static_assert(std::is_constructible<saga::expected<std::vector<int>, long>, std::size_t>{}, "");
static_assert(!std::is_convertible<std::size_t, saga::expected<std::vector<int>, long>>{}, "");

static_assert(std::is_constructible<saga::expected<std::unique_ptr<int>, long>,
                                    std::unique_ptr<int>>{}, "");
static_assert(!std::is_constructible<saga::expected<void, int>, long>{}, "");

static_assert(!std::is_constructible<int, std::vector<int>>{}, "");
static_assert(!std::is_constructible<saga::expected<int, void*>, std::vector<int>>{}, "");

TEST_CASE("expected : explicit constructor from one argument")
{
    {
        using Value = ::explicit_ctor_from<int>;
        using Error = long*;
        using Argument = int;

        static_assert(std::is_constructible<Value, Argument>{}, "");
        static_assert(!std::is_convertible<Argument, Value>{}, "");

        constexpr auto arg = Argument(1911);

        constexpr saga::expected<Value, Error> obj(arg);

        static_assert(obj.has_value(), "");
        static_assert(obj.value().value == arg, "");
    }

    using Value = std::vector<int>;
    using Error = std::string;
    using Argument = std::size_t;

    saga_test::property_checker << [](saga_test::container_size<Argument> const & arg)
    {
        saga::expected<Value, Error> const obj(arg.value);

        REQUIRE(obj.has_value());
        REQUIRE(obj.value() == Value(arg.value));
    };
}

TEST_CASE("expected : implicit constructor from one argument")
{
    {
        using Value = long;
        using Error = int *;
        using Argument = int;

        constexpr auto arg = Argument(17);

        constexpr saga::expected<Value, Error> obj = arg;

        static_assert(obj.has_value(), "");
        static_assert(obj.value() == Value(arg), "");
    }

    using Value = long;
    using Error = std::string;
    using Argument = int;

    saga_test::property_checker << [](Argument const & arg)
    {
        saga::expected<Value, Error> const obj = arg;

        REQUIRE(obj.has_value());
        REQUIRE(obj.value() == Value(arg));
    };
}

// Конструктор на основе unexpected
static_assert(std::is_constructible<long, int const &>{}, "");
static_assert(std::is_convertible<int const &, long>{}, "");
static_assert(std::is_constructible<saga::expected<void, long>,
                                    saga::unexpected<long> const &>{}, "");
static_assert(std::is_convertible<saga::unexpected<long> const &,
                                  saga::expected<void, long>>{}, "");
static_assert(std::is_constructible<saga::expected<std::string, long>,
                                    saga::unexpected<long> const &>{}, "");
static_assert(std::is_constructible<saga::expected<std::string, long>,
                                    saga::unexpected<long> &>{}, "");
static_assert(std::is_convertible<saga::unexpected<long> const &,
                                  saga::expected<std::string, long>>{}, "");

static_assert(std::is_constructible<std::vector<int>, std::size_t const &>{}, "");
static_assert(!std::is_convertible<std::size_t const &, std::vector<int>>{}, "");
static_assert(std::is_constructible<saga::expected<void, std::vector<int>>,
                                    saga::unexpected<std::size_t> const &>{}, "");
static_assert(!std::is_convertible<saga::unexpected<std::size_t> const &,
                                  saga::expected<void, std::vector<int>>>{}, "");
static_assert(std::is_constructible<saga::expected<std::string, std::vector<int>>,
                                    saga::unexpected<std::size_t> const &>{}, "");
static_assert(!std::is_convertible<saga::unexpected<std::size_t> const &,
                                  saga::expected<std::string, std::vector<int>>>{}, "");

static_assert(!std::is_constructible<int, std::vector<int>>{}, "");
static_assert(!std::is_constructible<saga::expected<void, int>,
                                     saga::unexpected<std::vector<int> const &>>{}, "");
static_assert(!std::is_constructible<saga::expected<std::string, int>,
                                     saga::unexpected<std::vector<int> const &>>{}, "");

TEST_CASE("expected: explicit constructor from unexpected const &")
{
    {
        using Error = ::explicit_ctor_from<int const&>;
        using ErrorArg = int;

        static_assert(std::is_constructible<Error, ErrorArg const &>{}, "");
        static_assert(!std::is_convertible<ErrorArg const &, Error>{}, "");

        constexpr ErrorArg error_arg(2259);


        constexpr saga::unexpected<ErrorArg> unex(error_arg);

        constexpr saga::expected<void, Error> ex1(unex);
        constexpr saga::expected<int, Error> ex2(unex);

        static_assert(!ex1.has_value(), "");
        static_assert(ex1.error().value == error_arg, "");

        static_assert(!ex2.has_value(), "");
        static_assert(ex2.error().value == error_arg, "");
    }

    using Error = std::vector<int>;
    using ErrorArg = std::size_t;

    saga_test::property_checker << [](saga_test::container_size<ErrorArg> const & error_arg)
    {
        saga::unexpected<ErrorArg> const unex_arg(error_arg.value);

        saga::expected<void, Error> const obj(unex_arg);

        REQUIRE(!obj.has_value());
        REQUIRE(obj.error() == Error(error_arg.value));
    };
}

TEST_CASE("expected: implicit constructor from unexpected const & ")
{
    using Error = long;
    using ErrorArg = int;

    {
        constexpr ErrorArg error_arg(1603);

        constexpr saga::unexpected<ErrorArg> unex_arg(error_arg);

        constexpr saga::expected<void, Error> const obj = unex_arg;

        static_assert(!obj.has_value(), "");
        static_assert(obj.error() == Error(error_arg), "");
    }

    saga_test::property_checker << [](ErrorArg const & error_arg)
    {
        saga::unexpected<ErrorArg> const unex_arg(error_arg);

        saga::expected<void, Error> const obj = unex_arg;

        REQUIRE(!obj.has_value());
        REQUIRE(obj.error() == Error(error_arg));
    };
}

// Порождение произвольного expected
namespace
{
    template <class Value, class Error>
    auto value_to_string(saga::expected<Value, Error> const & obj)
    -> std::enable_if_t<std::is_void<Value>{}, std::string>
    {
        assert(obj.has_value());
        return {};
    }

    template <class Value, class Error>
    auto value_to_string(saga::expected<Value, Error> const & obj)
    -> std::enable_if_t<!std::is_void<Value>{}, std::string>
    {
        assert(obj.has_value());

        return Catch::StringMaker<Value>::convert(*obj);
    }

    template <class Value, class Error>
    struct expected_carrier
    {
        friend bool operator==(expected_carrier const & lhs, expected_carrier const & rhs)
        {
            return lhs.value == rhs.value;
        }

        template <class... Args>
        explicit expected_carrier(saga::in_place_t, Args &&... args)
         : value(saga::in_place_t{}, std::forward<Args>(args)...)
        {}

        explicit expected_carrier(saga::unexpect_t, Error error)
         : value(saga::unexpect_t{}, std::move(error))
        {}

        ::saga::expected<Value, Error> value;
    };
}

namespace Catch
{
    template <class Value, class Error>
    struct StringMaker<::expected_carrier<Value, Error>>
    {
        static std::string convert(::expected_carrier<Value, Error> const & rhs)
        {
            if(rhs.value.has_value())
            {
                return "Value: " + ::value_to_string(rhs.value);
            }
            else
            {
                return "Unexpected: " + Catch::StringMaker<Error>::convert(rhs.value.error());
            }
        }
    };
}
// namespace Catch

namespace saga_test
{
    template <class Value, class Error, class SFINAE = void>
    struct arbitrary_expected_with_value
    {
        using value_type = expected_carrier<Value, Error>;

        template <class UniformRandomBitGenerator>
        static value_type generate(generation_t generation, UniformRandomBitGenerator & urbg)
        {
            auto value = saga_test::arbitrary<Value>::generate(generation, urbg);

            return value_type(saga::in_place, std::move(value));
        }
    };

    template <class Value, class Error>
    struct arbitrary_expected_with_value<Value, Error, std::enable_if_t<std::is_void<Value>{}>>
    {
        static_assert(std::is_void<Value>{}, "");

        using value_type = expected_carrier<Value, Error>;

        template <class UniformRandomBitGenerator>
        static value_type generate(generation_t, UniformRandomBitGenerator &)
        {
            return value_type(saga::in_place);
        }
    };

    template <class Value, class Error>
    struct arbitrary<expected_carrier<Value, Error>>
    {
        using value_type = expected_carrier<Value, Error>;

        template <class UniformRandomBitGenerator>
        static value_type generate(generation_t generation, UniformRandomBitGenerator & urbg)
        {
            auto const has_value = saga_test::arbitrary<bool>::generate(generation, urbg);
            generation /= 2;

            if(has_value)
            {
                return ::arbitrary_expected_with_value<Value, Error>::generate(generation, urbg);
            }
            else
            {
                auto error = saga_test::arbitrary<Error>::generate(generation, urbg);

                return value_type(saga::unexpect, std::move(error));
            }
        }
    };
}

// 4.6 Сравнение expected
TEST_CASE("expected<void, Error> : equality")
{
    {
        constexpr saga::expected<void, int> obj1{};
        constexpr saga::expected<void, int> obj2(saga::unexpect, 42);
        constexpr saga::expected<void, long> obj3(saga::unexpect, 2020);

        static_assert(obj1 == obj1, "");
        static_assert(obj1 != obj2, "");
        static_assert(obj1 != obj3, "");

        static_assert(obj2 != obj1, "");
        static_assert(obj2 == obj2, "");
        static_assert(obj2 != obj3, "");

        static_assert(obj3 != obj1, "");
        static_assert(obj3 != obj2, "");
        static_assert(obj3 == obj3, "");
    }

    using Error1 = char;
    using Error2 = short;

    static_assert(!std::is_same<Error1, Error2>{}, "");

    saga_test::property_checker << [](Error1 const & err1, Error2 const & err2)
    {
        saga::expected<void, Error1> const obj_value_1{};
        saga::expected<void const, Error2> const obj_value_2{};

        saga::expected<void volatile, Error1> const obj_error_1(saga::unexpect, err1);
        saga::expected<void volatile const, Error2> const obj_error_2(saga::unexpect, err2);

        REQUIRE(obj_value_1 == obj_value_1);
        REQUIRE(obj_value_1 == obj_value_2);
        REQUIRE(obj_value_1 != obj_error_1);
        REQUIRE(obj_value_1 != obj_error_2);

        REQUIRE(obj_value_2 == obj_value_1);
        REQUIRE(obj_value_2 == obj_value_2);
        REQUIRE(obj_value_2 != obj_error_1);
        REQUIRE(obj_value_2 != obj_error_2);

        REQUIRE(obj_error_1 != obj_value_1);
        REQUIRE(obj_error_1 != obj_value_2);
        REQUIRE(obj_error_1 == obj_error_1);
        REQUIRE((obj_error_1 == obj_error_2) == (err1 == err2));
        REQUIRE((obj_error_1 != obj_error_2) == !(obj_error_1 == obj_error_2));

        REQUIRE(obj_error_2 != obj_value_1);
        REQUIRE(obj_error_2 != obj_value_2);
        REQUIRE(obj_error_2 == obj_error_2);
        REQUIRE((obj_error_2 == obj_error_1) == (err2 == err1));
        REQUIRE((obj_error_2 != obj_error_1) == !(obj_error_2 == obj_error_1));
    };
}

TEST_CASE("expected<Value, Error> : equality")
{
    using Value1 = int;
    using Value2 = long;
    using Error1 = char;
    using Error2 = short;

    {
        constexpr Value1 val1 = 906;
        constexpr Value2 val2 = 2020;

        constexpr Error1 err1 = 22;
        constexpr Error2 err2 = 38;

        constexpr saga::expected<Value1, Error1> obj_value_1(saga::in_place_t{}, val1);
        constexpr saga::expected<Value2, Error2> obj_value_2(saga::in_place_t{}, val2);

        constexpr saga::expected<Value1, Error1> const obj_error_1(saga::unexpect, err1);
        constexpr saga::expected<Value2, Error2> const obj_error_2(saga::unexpect, err2);

        static_assert(obj_value_1 == obj_value_1, "");
        static_assert(obj_value_1 != obj_value_2, "");
        static_assert(obj_value_1 != obj_error_1, "");
        static_assert(obj_value_1 != obj_error_2, "");

        static_assert(obj_value_2 != obj_value_1, "");
        static_assert(obj_value_2 == obj_value_2, "");
        static_assert(obj_value_2 != obj_error_1, "");
        static_assert(obj_value_2 != obj_error_2, "");

        static_assert(obj_error_1 != obj_value_1, "");
        static_assert(obj_error_1 != obj_value_2, "");
        static_assert(obj_error_1 == obj_error_1, "");
        static_assert(obj_error_1 != obj_error_2, "");

        static_assert(obj_error_2 != obj_value_1, "");
        static_assert(obj_error_2 != obj_value_2, "");
        static_assert(obj_error_2 != obj_error_1, "");
        static_assert(obj_error_2 == obj_error_2, "");
    }

    static_assert(!std::is_same<Value1, Value2>{}, "");
    static_assert(!std::is_same<Error1, Error2>{}, "");

    saga_test::property_checker << [](Value1 const & val1, Value2 const & val2,
                                      Error1 const & err1, Error2 const & err2)
    {
        saga::expected<Value1, Error1> const obj_value_1(saga::in_place_t{}, val1);
        saga::expected<Value2, Error2> const obj_value_2(saga::in_place_t{}, val2);

        saga::expected<Value1, Error1> const obj_error_1(saga::unexpect, err1);
        saga::expected<Value2, Error2> const obj_error_2(saga::unexpect, err2);

        REQUIRE(obj_value_1 == obj_value_1);
        REQUIRE(obj_value_2 == obj_value_2);

        REQUIRE(obj_error_1 == obj_error_1);
        REQUIRE(obj_error_2 == obj_error_2);

        REQUIRE(obj_value_1 != obj_error_1);
        REQUIRE(obj_value_1 != obj_error_2);

        REQUIRE(obj_value_2 != obj_error_1);
        REQUIRE(obj_value_2 != obj_error_2);

        REQUIRE((obj_value_1 == obj_value_2) == (val1 == val2));
        REQUIRE((obj_error_1 == obj_error_2) == (err1 == err2));

        REQUIRE((obj_value_1 != obj_value_2) == !(obj_value_1 == obj_value_2));
        REQUIRE((obj_error_1 != obj_error_2) == !(obj_error_1 == obj_error_2));
    };
}

// 4.7 Сравнение со значением
TEST_CASE("expected : equality with value")
{
    {
        using Value = long;
        using Error = void *;
        using OtherValue = int;

        constexpr auto value = Value(42);
        constexpr auto error = Error(nullptr);
        constexpr auto other = OtherValue(806);

        constexpr saga::expected<Value, Error> obj_value(saga::in_place_t{}, value);
        constexpr saga::expected<Value, Error> obj_error(saga::unexpect, error);

        static_assert(value != other, "");

        static_assert(obj_value == value, "");
        static_assert(value == obj_value, "");

        static_assert(obj_value != other, "");
        static_assert(other != obj_value, "");

        static_assert(obj_error != value, "");
        static_assert(other != obj_error, "");
    }

    using Value = long;
    using Error = std::string;
    using OtherValue = int;

    saga_test::property_checker
    << [](Value const & value, Error const & error, OtherValue const & other)
    {
        {
            saga::expected<Value, Error> const obj_value(saga::in_place_t{}, value);

            REQUIRE(obj_value == value);
            REQUIRE(value == obj_value);

            REQUIRE((obj_value == other) == (value == other));
            REQUIRE((obj_value != other) == !(obj_value == other));

            REQUIRE((other == obj_value) == (other == value));
            REQUIRE((other != obj_value) == !(other == obj_value));
        }
        {
            saga::expected<Value, Error> const obj_error(saga::unexpect, error);

            REQUIRE(obj_error != value);
            REQUIRE(value != obj_error);

            REQUIRE(!(obj_error == value));
            REQUIRE(!(value == obj_error));
        }
    };
}

// 4.8 Сравнение со unexpected
TEST_CASE("expected : equality with unexpected")
{
    {
        using Value = void *;
        using Error = long;
        using OtherError = int;

        constexpr auto value = Value(nullptr);
        constexpr auto unex = saga::unexpected<Error>(42);
        constexpr auto other_unex = saga::unexpected<OtherError>(906);

        constexpr saga::expected<Value, Error> obj_value(saga::in_place_t{}, value);
        constexpr saga::expected<Value, Error> obj_error(saga::unexpect, unex.value());

        static_assert(unex != other_unex, "");

        static_assert(obj_error == unex, "");
        static_assert(unex == obj_error, "");

        static_assert(obj_error != other_unex, "");
        static_assert(other_unex != obj_error, "");

        static_assert(obj_value != other_unex, "");
        static_assert(other_unex != obj_value, "");

        static_assert(obj_value != unex, "");
        static_assert(unex != obj_value, "");
    }

    using Value = std::string;
    using Error = long;
    using OtherError = int;

    saga_test::property_checker
    << [](Value const & value, Error const & err, OtherError const & other_err)
    {
        saga::unexpected<Error> const unex(err);
        saga::unexpected<OtherError> const other_unex(other_err);

        {
            saga::expected<Value, Error> const obj_value(saga::in_place_t{}, value);

            REQUIRE(obj_value != unex);
            REQUIRE(unex != obj_value);

            REQUIRE(!(obj_value == unex));
            REQUIRE(!(unex == obj_value));
        }
        {
            saga::expected<Value, Error> const obj_error(saga::unexpect, err);

            REQUIRE(obj_error == unex);
            REQUIRE(unex == obj_error);

            REQUIRE((obj_error == other_unex) == (unex == other_unex));
            REQUIRE((other_unex == obj_error) == (other_unex == unex));

            REQUIRE((obj_error != other_unex) == !(obj_error == other_unex));
            REQUIRE((other_unex != obj_error) == !(other_unex == obj_error));
        }
    };
}

// Конструктор копий
static_assert(std::is_copy_constructible<saga::expected<std::vector<int>, std::string>>{}, "");
static_assert(std::is_copy_constructible<saga::expected<void, std::string>>{}, "");

static_assert(!std::is_copy_constructible<saga::expected<std::unique_ptr<int>, std::string>>{}, "");
static_assert(!std::is_copy_constructible<saga::expected<void, std::unique_ptr<int>>>{}, "");
static_assert(!std::is_copy_constructible<saga::expected<std::unique_ptr<long>,
                                                         std::unique_ptr<int>>>{}, "");

TEST_CASE("expected<void, Error>: copy constructor")
{
    // constexpr
    {
        using Value = void;
        using Error = int;

        constexpr saga::expected<Value, Error> const src{};

        constexpr saga::expected<Value, Error> const obj(src);

        static_assert(obj == src, "");
    }
    {
        using Value = void;
        using Error = int;
        constexpr auto error = Error(42);

        constexpr saga::expected<Value, Error> const src(saga::unexpect, error);

        constexpr saga::expected<Value, Error> const obj(src);

        static_assert(obj == src, "");
    }
    // не constexpr
    {
        using Value = void;
        using Error = int;

        saga::expected<Value, Error> const src{};

        saga::expected<Value, Error> const obj(src);

        REQUIRE(obj == src);
    }
    {
        using Value = void;
        using Error = std::string;

        saga::expected<Value, Error> const src{};

        saga::expected<Value, Error> const obj(src);

        REQUIRE(obj == src);
    }
    {
        using Value = void;
        using Error = std::string;

        saga_test::property_checker << [](Error const & error)
        {
            saga::expected<Value, Error> const src(saga::unexpect, error);

            saga::expected<Value, Error> const obj(src);

            REQUIRE(obj == src);
        };
    }
}

TEST_CASE("expected<Value, Error>: copy constructor")
{
    // constexpr
    {
        using Value = long;
        using Error = int *;

        constexpr auto value = Value(13);

        constexpr saga::expected<Value, Error> const src(saga::in_place_t{}, value);

        constexpr saga::expected<Value, Error> const obj(src);

        static_assert(obj == src, "");
    }
    {
        using Value = long;
        using Error = int;
        constexpr auto error = Error(42);

        constexpr saga::expected<Value, Error> const src(saga::unexpect, error);

        constexpr saga::expected<Value, Error> const obj(src);

        static_assert(obj == src, "");
    }
    // не constexpr
    {
        using Value = long;
        using Error = int *;

        auto const value = Value(13);

        saga::expected<Value, Error> const src(saga::in_place_t{}, value);

        saga::expected<Value, Error> const obj(src);

        REQUIRE(obj == src);
    }
    {
        using Value = std::vector<int>;
        using Error = std::string;

        saga_test::property_checker << [](Value const & value)
        {
            saga::expected<Value, Error> const src(saga::in_place_t{}, value);

            saga::expected<Value, Error> const obj(src);

            REQUIRE(obj == src);
        }
        << [](Error const & error)
        {
            saga::expected<Value, Error> const src(saga::unexpect, error);

            saga::expected<Value, Error> const obj(src);

            REQUIRE(obj == src);
        };
    }
}

// @tood Конструктор перемещения
namespace
{
    struct not_move_constructible
    {
        not_move_constructible(not_move_constructible const &) = delete;
        not_move_constructible(not_move_constructible &&) = delete;
    };

    struct throwing_move_ctor
    {
        throwing_move_ctor(throwing_move_ctor const &) noexcept(false) {}
        throwing_move_ctor(throwing_move_ctor &&) noexcept(false) {}

        throwing_move_ctor & operator=(throwing_move_ctor const &) noexcept(false);
        throwing_move_ctor & operator=(throwing_move_ctor &&) noexcept(false);
    };
}

// Наличие конструктора перемещения
static_assert(std::is_move_constructible<saga::expected<int, long>>{}, "");
static_assert(std::is_move_constructible<saga::expected<std::unique_ptr<int>, long>>{}, "");
static_assert(std::is_move_constructible<saga::expected<int, std::unique_ptr<long>>>{}, "");
static_assert(std::is_move_constructible<saga::expected<std::unique_ptr<int>, std::unique_ptr<long>>>{}, "");

static_assert(std::is_move_constructible<saga::expected<void, long>>{}, "");
static_assert(std::is_move_constructible<saga::expected<void, std::unique_ptr<long>>>{}, "");

static_assert(!std::is_move_constructible<::not_move_constructible>{}, "");
static_assert(!std::is_move_constructible<saga::expected<::not_move_constructible, long>>{}, "");
static_assert(!std::is_move_constructible<saga::expected<void, ::not_move_constructible>>{}, "");
static_assert(!std::is_move_constructible<saga::expected<std::string, ::not_move_constructible>>{}, "");
static_assert(!std::is_move_constructible<saga::expected<::not_move_constructible, ::not_move_constructible>>{}, "");

// noexcept для конструктора перемещения
static_assert(std::is_nothrow_move_constructible<saga::expected<void, long>>{}, "");
static_assert(std::is_nothrow_move_constructible<saga::expected<int, long>>{}, "");

static_assert(std::is_nothrow_move_constructible<std::string>{}, "");
static_assert(std::is_nothrow_move_constructible<saga::expected<int, std::string>>{}, "");
static_assert(std::is_nothrow_move_constructible<saga::expected<void, std::string>>{}, "");
static_assert(std::is_nothrow_move_constructible<saga::expected<std::string, long *>>{}, "");
static_assert(std::is_nothrow_move_constructible<saga::expected<std::string, std::string>>{}, "");

static_assert(!std::is_nothrow_move_constructible<::throwing_move_ctor>{}, "");
static_assert(!std::is_nothrow_move_constructible<void>{}, "");
static_assert(!std::is_nothrow_move_constructible<saga::expected<void, ::throwing_move_ctor>>{}, "");
static_assert(!std::is_nothrow_move_constructible<saga::expected<int, ::throwing_move_ctor>>{}, "");
static_assert(!std::is_nothrow_move_constructible<saga::expected<std::string, ::throwing_move_ctor>>{}, "");
static_assert(!std::is_nothrow_move_constructible<saga::expected<::throwing_move_ctor, int>>{}, "");
static_assert(!std::is_nothrow_move_constructible<saga::expected<::throwing_move_ctor, std::string>>{}, "");

TEST_CASE("expected<void, Error>: move constructor")
{
    using Value = void const;

    // не constexpr
    {
        using Error = int;
        saga::expected<Value, Error> const src_old(saga::in_place_t{});
        auto src = src_old;

        saga::expected<Value, Error> const obj(std::move(src));

        REQUIRE(obj == src_old);
        REQUIRE(src.has_value());
    }

    {
        using Error = std::string;
        saga::expected<Value, Error> const src_old(saga::in_place_t{});
        auto src = src_old;

        saga::expected<Value, Error> const obj(std::move(src));

        REQUIRE(obj == src_old);
        REQUIRE(src.has_value());
    }

    using Error = std::string;
    saga_test::property_checker << [](Error const & error)
    {
        saga::expected<Value, Error> const src_old(saga::unexpect, error);
        auto src = src_old;

        saga::expected<Value, Error> const obj(std::move(src));

        REQUIRE(obj == src_old);
        REQUIRE(!src.has_value());
        REQUIRE(src.error().empty());
    };

    {
        using Error = int;
        constexpr saga::expected<Value, Error> const src(saga::in_place_t{});
        constexpr saga::expected<Value, Error> const obj(::use_constexpr_move(src));

        static_assert(obj == src, "");
    }
    {
        using Error = long;
        constexpr auto const error = Error{42};
        constexpr saga::expected<Value, Error> const src(saga::unexpect, error);
        constexpr saga::expected<Value, Error> const obj(::use_constexpr_move(src));

        static_assert(obj == src, "");
    }
}

TEST_CASE("expected<Value, Error>: move constructor")
{
    // не constexpr
    {
        using Value = std::vector<int>;
        using Error = std::string;

        saga_test::property_checker << [](Value const & value)
        {
            saga::expected<Value, Error> const src_old(saga::in_place, value);
            auto src = src_old;

            saga::expected<Value, Error> const obj(std::move(src));

            REQUIRE(obj == src_old);
            REQUIRE(src.has_value());
            REQUIRE(src.value().empty());
        }
        << [](Error const & error)
        {
            saga::expected<Value, Error> const src_old(saga::unexpect, error);
            auto src = src_old;

            saga::expected<Value, Error> const obj(std::move(src));

            REQUIRE(obj == src_old);
            REQUIRE(!src.has_value());
            REQUIRE(src.error().empty());
        };
    }

    // constexpr
    {
        using Value = long;
        using Error = int;

        {
            constexpr auto const value = Value{42};
            constexpr saga::expected<Value, Error> const src(saga::in_place_t{}, value);
            constexpr saga::expected<Value, Error> const obj(::use_constexpr_move(src));

            static_assert(obj == src, "");
        }
        {
            constexpr auto const error = Error{42};
            constexpr saga::expected<Value, Error> const src(saga::unexpect, error);
            constexpr saga::expected<Value, Error> const obj(::use_constexpr_move(src));

            static_assert(obj == src, "");
        }
    }
}

// Конструктор на основе временного unexpected
static_assert(std::is_constructible<std::unique_ptr<Base>, std::unique_ptr<Derived> &&>{}, "");
static_assert(std::is_constructible<saga::expected<void, std::unique_ptr<Base>>,
                                    saga::unexpected<std::unique_ptr<Derived>> &&>{}, "");
static_assert(std::is_convertible<std::unique_ptr<Derived> &&, std::unique_ptr<Base>>{}, "");
static_assert(std::is_convertible<saga::unexpected<std::unique_ptr<Derived>> &&,
                                  saga::expected<std::string, std::unique_ptr<Base>>>{}, "");

static_assert(std::is_constructible<::explicit_ctor_from<int>, int &&>{}, "");
static_assert(!std::is_convertible<int &&, ::explicit_ctor_from<int>>{}, "");
static_assert(std::is_constructible<saga::expected<int, ::explicit_ctor_from<int>>,
                                    saga::unexpected<int>&&>{}, "");
static_assert(!std::is_convertible<saga::unexpected<int> &&,
                                   saga::expected<int, ::explicit_ctor_from<int>>>{}, "");

static_assert(std::is_nothrow_constructible<long, int>{}, "");
static_assert(std::is_nothrow_constructible<saga::expected<void, long>,
                                            saga::unexpected<int> &&>{}, "");
static_assert(std::is_nothrow_constructible<::explicit_ctor_from<int>, int>{}, "");
static_assert(std::is_nothrow_constructible<saga::expected<void, ::explicit_ctor_from<int>>,
                                            saga::unexpected<int> &&>{}, "");

static_assert(!std::is_nothrow_constructible<std::vector<char>, std::size_t &&>{}, "");
static_assert(!std::is_nothrow_constructible<saga::expected<double, std::vector<char>>,
                                             saga::unexpected<std::size_t> &&>{}, "");

namespace
{
    struct throwing_implicit_ctor_from_int
    {
        throwing_implicit_ctor_from_int(int value) noexcept(false)
         : value(value)
        {}

        int value;
    };
}

static_assert(std::is_constructible<::throwing_implicit_ctor_from_int, int>{}, "");
static_assert(std::is_convertible<int, ::throwing_implicit_ctor_from_int>{}, "");
static_assert(!std::is_nothrow_constructible<::throwing_implicit_ctor_from_int, int>{}, "");
static_assert(!std::is_nothrow_constructible<saga::expected<int, throwing_implicit_ctor_from_int>,
                                             saga::unexpected<int> &&>{}, "");

TEST_CASE("expected: explicit constructor from unexpected &&")
{
    {
        using Error = ::explicit_ctor_from<int>;
        using ErrorArg = int;

        static_assert(std::is_constructible<Error, ErrorArg const &>{}, "");
        static_assert(!std::is_convertible<ErrorArg const &, Error>{}, "");

        constexpr ErrorArg error_arg(2259);

        constexpr auto ex1 = saga::expected<void, Error>(saga::unexpected<ErrorArg>(error_arg));
        constexpr auto ex2 = saga::expected<int, Error>(saga::unexpected<ErrorArg>(error_arg));

        static_assert(!ex1.has_value(), "");
        static_assert(ex1.error().value == error_arg, "");

        static_assert(!ex2.has_value(), "");
        static_assert(ex2.error().value == error_arg, "");
    }

    using Error = std::vector<int>;
    using ErrorArg = std::size_t;

    saga_test::property_checker << [](saga_test::container_size<ErrorArg> const & error_arg)
    {
        auto const obj = saga::expected<void, Error>(saga::unexpected<ErrorArg>(error_arg.value));

        REQUIRE(!obj.has_value());
        REQUIRE(obj.error() == Error(error_arg.value));
    };

    {
        saga::unexpected<std::vector<int>> const unex_old(saga::in_place_t{}, {1, 2, 3, 4, 5});
        auto unex = unex_old;

        saga::expected<void, std::vector<int>> const obj(std::move(unex));

        REQUIRE(!obj.has_value());
        REQUIRE(obj.error() == unex_old.value());
        REQUIRE(unex.value().empty());
    }
}

TEST_CASE("expected: implicit constructor from unexpected &&")
{
    using Error = long;
    using ErrorArg = int;

    {
        constexpr ErrorArg error_arg(1603);

        constexpr saga::expected<void, Error> const obj = saga::unexpected<ErrorArg>(error_arg);

        static_assert(!obj.has_value(), "");
        static_assert(obj.error() == Error(error_arg), "");
    }

    saga_test::property_checker << [](ErrorArg const & error_arg)
    {
        saga::expected<void, Error> const obj = saga::unexpected<ErrorArg>(error_arg);

        REQUIRE(!obj.has_value());
        REQUIRE(obj.error() == Error(error_arg));
    };
}

// Конструктор с in_place_t
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

namespace
{
    struct ctor_from_in_place_t
    {
        ctor_from_in_place_t() = default;
        ctor_from_in_place_t(saga::in_place_t)
         : default_constructed(false)
        {}

        bool default_constructed = true;
    };
}

TEST_CASE("expected: placement ctor over one argument ctor")
{
    {
        constexpr saga::expected<ctor_from_in_place_t, long> obj(saga::in_place_t{});

        static_assert(obj.has_value(), "");
        static_assert(obj.value().default_constructed == true, "");
    }
    {
        saga::expected<ctor_from_in_place_t, std::string> const obj(saga::in_place_t{});

        REQUIRE(obj.has_value());
        REQUIRE(obj.value().default_constructed == true);
    }
}

TEST_CASE("expected: placement constructor with no additional arguments")
{
    using Value = std::vector<int>;
    using Error = std::string;

    {
        saga::expected<Value, Error> const obj(saga::in_place_t{});

        REQUIRE(obj.has_value());
        REQUIRE(obj.value() == Value());
    }
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

namespace
{
    using initializer_list_consumer = saga_test::init_by_reduce_initializer_list;
}

TEST_CASE("expected: placement constructor with initializer list and more args")
{
    {
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
        constexpr saga::expected<long, initializer_list_consumer>
            obj(saga::unexpect, {1, 2, 3, 4}, 5);

        static_assert(!obj.has_value(), "");
        static_assert(obj.error().value == 15, "");
    }
    {
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

// Присваивание
// Копирующее присваивание
namespace
{
    template <class Value, class Error>
    void check_expected_copy_assign(::expected_carrier<Value, Error> dest_carrier,
                                    ::expected_carrier<Value, Error> const & src_carrier)
    {
        auto & dest = dest_carrier.value;
        auto const & src = src_carrier.value;

        auto & result = (dest = src);

        REQUIRE(dest == src);
        REQUIRE(std::addressof(result) == std::addressof(dest));
        static_assert(std::is_same<decltype(result), saga::expected<Value, Error> &>{}, "");
    }
}

TEST_CASE("expected: copy assignment")
{
    saga_test::property_checker
        << ::check_expected_copy_assign<void, long>
        << ::check_expected_copy_assign<void, std::string>
        << ::check_expected_copy_assign<void const, long>
        << ::check_expected_copy_assign<void const, std::string>
        << ::check_expected_copy_assign<void volatile, long>
        << ::check_expected_copy_assign<void volatile, std::string>
        << ::check_expected_copy_assign<void const volatile, long>
        << ::check_expected_copy_assign<void const volatile, std::string>

        << ::check_expected_copy_assign<int, long>
        << ::check_expected_copy_assign<std::string, long>
        << ::check_expected_copy_assign<long, std::vector<int>>
        << ::check_expected_copy_assign<std::string, std::vector<long>>;
}

// Присваивание с перемещением
namespace
{
    template <class Value, class Error>
    auto check_expected_after_move(saga::expected<Value, Error> const & src,
                                   saga::expected<Value, Error> const & src_old)
    -> std::enable_if_t<!std::is_void<Value>{}>
    {
        REQUIRE(src.has_value() == src_old.has_value());

        if(src_old.has_value())
        {
            auto value = src_old.value();
            auto value_sink = std::move(value);

            REQUIRE(value == src.value());
            REQUIRE(value_sink == src_old.value());
        }
        else
        {
            auto error = src_old.error();
            auto error_sink = std::move(error);

            REQUIRE(error == src.error());
            REQUIRE(error_sink == src_old.error());
        }
    }

    template <class Value, class Error>
    auto check_expected_after_move(saga::expected<Value, Error> const & src,
                                   saga::expected<Value, Error> const & src_old)
    -> std::enable_if_t<std::is_void<Value>{}>
    {
        if(src_old.has_value())
        {}
        else
        {
            auto error = src_old.error();
            auto error_sink = std::move(error);

            REQUIRE(error == src.error());
            REQUIRE(error_sink == src_old.error());
        }
    }

    template <class Value, class Error>
    void check_expected_move_assign(saga::expected<Value, Error> & dest,
                                    saga::expected<Value, Error> const & src_old)
    {
        using Expected = saga::expected<Value, Error>;
        auto src = src_old;

        auto & result = (dest = std::move(src));

        REQUIRE(dest == src_old);

        static_assert(std::is_same<decltype(result), Expected &>{}, "");
        REQUIRE(std::addressof(result) == std::addressof(dest));

        ::check_expected_after_move(src, src_old);
    }

    template <class Value, class Error, class = std::enable_if_t<std::is_void<Value>{}>>
    void check_expected_move_assign_void_both_value()
    {
        using Expected = saga::expected<Value, Error>;

        Expected dest(saga::in_place);
        Expected src(saga::in_place);

        ::check_expected_move_assign(dest, src);
    }

    template <class Value, class Error, class = std::enable_if_t<std::is_void<Value>{}>>
    void check_expected_move_assign_void_value_error(Error const & src_error)
    {
        using Expected = saga::expected<Value, Error>;

        Expected dest(saga::in_place);
        Expected const src_old(saga::unexpect, src_error);

        ::check_expected_move_assign(dest, src_old);
    }

    template <class Value, class Error, class = std::enable_if_t<std::is_void<Value>{}>>
    void check_expected_move_assign_void_error_value(Error const & dest_error)
    {
        using Expected = saga::expected<Value, Error>;

        Expected dest(saga::unexpect, dest_error);
        Expected const src_old(saga::in_place);

        ::check_expected_move_assign(dest, src_old);
    }

    template <class Value, class Error, class = std::enable_if_t<std::is_void<Value>{}>>
    void check_expected_move_assign_void_both_error(Error const & dest_error, Error const & src_error)
    {
        using Expected = saga::expected<Value, Error>;

        Expected dest(saga::unexpect, dest_error);
        Expected const src_old(saga::unexpect, src_error);

        ::check_expected_move_assign(dest, src_old);
    }
}

TEST_CASE("expected<void, Error>::move assign")
{
    ::check_expected_move_assign_void_both_value<void, long>();
    ::check_expected_move_assign_void_both_value<void, std::string>();
    ::check_expected_move_assign_void_both_value<void const, long>();
    ::check_expected_move_assign_void_both_value<void const, std::string>();
    ::check_expected_move_assign_void_both_value<void volatile, long>();
    ::check_expected_move_assign_void_both_value<void volatile, std::string>();
    ::check_expected_move_assign_void_both_value<void const volatile, long>();
    ::check_expected_move_assign_void_both_value<void const volatile, std::string>();

    saga_test::property_checker
    << check_expected_move_assign_void_value_error<void, long>
    << check_expected_move_assign_void_value_error<void, std::vector<int>>
    << check_expected_move_assign_void_value_error<void const, long>
    << check_expected_move_assign_void_value_error<void const, std::vector<int>>
    << check_expected_move_assign_void_value_error<void volatile, long>
    << check_expected_move_assign_void_value_error<void volatile, std::vector<int>>
    << check_expected_move_assign_void_value_error<void const volatile, long>
    << check_expected_move_assign_void_value_error<void const volatile, std::vector<int>>

    << check_expected_move_assign_void_error_value<void, long>
    << check_expected_move_assign_void_error_value<void, std::vector<int>>
    << check_expected_move_assign_void_error_value<void const, long>
    << check_expected_move_assign_void_error_value<void const, std::vector<int>>
    << check_expected_move_assign_void_error_value<void volatile, long>
    << check_expected_move_assign_void_error_value<void volatile, std::vector<int>>
    << check_expected_move_assign_void_error_value<void const volatile, long>
    << check_expected_move_assign_void_error_value<void const volatile, std::vector<int>>

    << check_expected_move_assign_void_both_error<void, long>
    << check_expected_move_assign_void_both_error<void, std::vector<int>>
    << check_expected_move_assign_void_both_error<void const, long>
    << check_expected_move_assign_void_both_error<void const, std::vector<int>>
    << check_expected_move_assign_void_both_error<void volatile, long>
    << check_expected_move_assign_void_both_error<void volatile, std::vector<int>>
    << check_expected_move_assign_void_both_error<void const volatile, long>
    << check_expected_move_assign_void_both_error<void const volatile, std::vector<int>>;
}

TEST_CASE("expected<Value, Error>: move assignment")
{
    using Value = std::vector<int>;
    using Error = long;
    using Expected = saga::expected<Value, Error>;

    saga_test::property_checker
    << [](Value const & dest_value, Value const & src_value)
    {
        Expected dest(saga::in_place, dest_value);
        Expected const src(saga::in_place, src_value);

        check_expected_move_assign(dest, src);
    }
    << [](Error const & dest_error, Value const & src_value)
    {
        Expected dest(saga::unexpect, dest_error);
        Expected const src(saga::in_place, src_value);

        check_expected_move_assign(dest, src);
    }
    << [](Value const & dest_value, Error const & src_error)
    {
        Expected dest(saga::in_place, dest_value);
        Expected const src(saga::unexpect, src_error);

        check_expected_move_assign(dest, src);
    }
    << [](Error const & dest_error, Error const & src_error)
    {
        Expected dest(saga::unexpect, dest_error);
        Expected const src(saga::unexpect, src_error);

        check_expected_move_assign(dest, src);
    };
}

// operator=(U &&)
namespace
{
    template <class Value, class Error, class Arg>
    void check_expected_copy_assign_value(saga::expected<Value, Error> & obj, Arg const & arg)
    {
        auto const & result = (obj = arg);

        REQUIRE(obj.has_value());
        REQUIRE(obj.value() == arg);

        static_assert(std::is_same<decltype(result), saga::expected<Value, Error> const &>{}, "");
        REQUIRE(std::addressof(result) == std::addressof(obj));
    }
}

TEST_CASE("expected: copy assign value")
{
    using Value = long;
    using Error = std::string;
    using Arg = int;

    saga_test::property_checker
    << [](::expected_carrier<Value, Error> carrier, Arg const & new_value)
    {
        check_expected_copy_assign_value(carrier.value, new_value);
    };
}

TEST_CASE("expected: move assign value")
{
    using Value = std::vector<int>;
    using Error = long;
    using Arg = Value;

    saga_test::property_checker
    << [](::expected_carrier<Value, Error> obj_carrier, Arg const & old_arg)
    {
        using Expected = saga::expected<Value, Error>;
        Expected & obj = obj_carrier.value;

        auto arg = old_arg;
        obj = std::move(arg);

        REQUIRE(obj.has_value());
        REQUIRE(*obj == old_arg);

        REQUIRE(arg.empty());
    };
}

static_assert(!std::is_assignable<saga::expected<void, std::string> &, int const &>{}, "");
static_assert(!std::is_assignable<saga::expected<void const, std::string> &, int const &>{}, "");
static_assert(!std::is_assignable<saga::expected<void volatile, std::string> &, int const &>{}, "");
static_assert(!std::is_assignable<saga::expected<void const volatile, std::string> &, int const &>{}, "");

static_assert(!std::is_constructible<int, std::string>{}, "");
static_assert(!std::is_assignable<saga::expected<int, long*> &, std::string>{}, "");

namespace
{
    struct ctor_but_no_assign
    {
        ctor_but_no_assign(int arg)
         : value(arg)
        {}

        ctor_but_no_assign & operator=(int) = delete;

        int value;
    };
}

static_assert(std::is_constructible<ctor_but_no_assign, int>{}, "");
static_assert(!std::is_assignable<ctor_but_no_assign &, int>{}, "");
// @todo Добавить проверку static_assert(!std::is_assignable<saga::expected<ctor_but_no_assign, long*> &, int>{}, "");
// В настоящий момент она порождает ошибку в is_default_constructible<expected<void, E>>

// operator=(unexpected<G> const &)
namespace
{
    template <class Value, class Error, class Arg>
    void check_expected_copy_assign_unexpected(::expected_carrier<Value, Error> obj_carrier,
                                               Arg const & arg)
    {
        auto & obj = obj_carrier.value;

        saga::unexpected<Arg> unex(arg);
        auto const & result = (obj = unex);

        REQUIRE(!obj.has_value());
        REQUIRE(obj.error() == Error(arg));

        static_assert(std::is_same<decltype(result), saga::expected<Value, Error> const &>{}, "");
        REQUIRE(std::addressof(result) == std::addressof(obj));
    }
}

TEST_CASE("expected: assign unexpected const &")
{
    // @todo Тесты для нетривиального типа Error
    saga_test::property_checker
        << ::check_expected_copy_assign_unexpected<std::string, long, int>
        << ::check_expected_copy_assign_unexpected<int, long, int>

        << ::check_expected_copy_assign_unexpected<void, long, int>
        << ::check_expected_copy_assign_unexpected<void const, long, int>
        << ::check_expected_copy_assign_unexpected<void volatile, long, int>
        << ::check_expected_copy_assign_unexpected<void const volatile, long, int>;
}

// operator=(unexpected<G> &&)
namespace
{
    template <class T>
    struct wrapper
    {
        T value;

        wrapper(T init_value)
         : value(init_value)
        {}
    };
}

TEST_CASE("expected<Value, Error>: assgin unexpected &&")
{
    using Value = std::string;
    using Error = std::vector<int>;

    saga_test::property_checker
    << [](Value const & init_value, Error const & src_old)
    {
        using Expected = saga::expected<Value, wrapper<Error>>;
        Expected obj(saga::in_place, init_value);

        saga::unexpected<Error> unex(src_old);
        auto & result = (obj = std::move(unex));

        REQUIRE(!obj.has_value());
        REQUIRE(obj.error().value == src_old);
        REQUIRE(unex.value().empty());

        static_assert(std::is_same<decltype(result), Expected &>{}, "");
    }
    << [](Error const & init_error, Error const & src_old)
    {
        using Expected = saga::expected<Value, wrapper<Error>>;
        Expected obj(saga::unexpect, init_error);

        saga::unexpected<Error> unex(src_old);
        auto & result = (obj = std::move(unex));

        REQUIRE(!obj.has_value());
        REQUIRE(obj.error().value == src_old);
        REQUIRE(unex.value().empty());

        static_assert(std::is_same<decltype(result), Expected &>{}, "");
    };
}

TEST_CASE("expected<void, Error>: assgin unexpected &&")
{
    using Error = std::vector<int>;

    saga_test::property_checker
    << [](Error const & src_old)
    {
        using Expected = saga::expected<void, wrapper<Error>>;
        Expected obj;

        saga::unexpected<Error> unex(src_old);
        auto & result = (obj = std::move(unex));

        REQUIRE(!obj.has_value());
        REQUIRE(obj.error().value == src_old);
        REQUIRE(unex.value().empty());

        static_assert(std::is_same<decltype(result), Expected &>{}, "");
    }
    << [](Error const & init_error, Error const & src_old)
    {
        using Expected = saga::expected<void, wrapper<Error>>;
        Expected obj(saga::unexpect, init_error);

        saga::unexpected<Error> unex(src_old);
        auto & result = (obj = std::move(unex));

        REQUIRE(!obj.has_value());
        REQUIRE(obj.error().value == src_old);
        REQUIRE(unex.value().empty());

        static_assert(std::is_same<decltype(result), Expected &>{}, "");
    };
}

// emplace
namespace
{
    template <class Value, class Error>
    void check_expected_emplace_void(saga::expected<Value, Error> obj)
    {
        static_assert(std::is_void<Value>{}, "");

        obj.emplace();

        REQUIRE(obj.has_value());
        REQUIRE_NOTHROW(obj.value());

        static_assert(std::is_same<decltype(obj.emplace()), void>{}, "");
    }

    template <class Value, class Error>
    void check_expected_emplace_for_unexpected(Error const & error)
    {
        return ::check_expected_emplace_void(saga::expected<Value, Error>(saga::unexpect, error));
    }

    template <class Error>
    void check_expected_void_emplace_with_value()
    {
        ::check_expected_emplace_void<void, Error>({});
        ::check_expected_emplace_void<void const, Error>({});
        ::check_expected_emplace_void<void volatile, Error>({});
        ::check_expected_emplace_void<void const volatile, Error>({});
    }

    template <class Error>
    void check_expected_void_emplace_with_error(Error const & error)
    {
        ::check_expected_emplace_for_unexpected<void>(error);
        ::check_expected_emplace_for_unexpected<void const>(error);
        ::check_expected_emplace_for_unexpected<void volatile>(error);
        ::check_expected_emplace_for_unexpected<void const volatile>(error);
    }

    // @todo Может быть параметризовать и тип new_value?
    template <class Value, class Error>
    void check_expected_emplace_not_void(saga::expected<Value, Error> obj, Value const & value)
    {
        static_assert(!std::is_void<Value>{}, "");

        auto & result = obj.emplace(value);

        REQUIRE(obj.has_value());
        REQUIRE(obj.value() == value);
        REQUIRE(std::addressof(result) == std::addressof(obj.value()));

        static_assert(std::is_same<decltype(result), Value &>{}, "");
    }

    template <class Value, class Error>
    void check_expected_emplace_in_value(Value const & old_value, Value const & new_value)
    {
        saga::expected<Value, Error> obj(saga::in_place, old_value);

        REQUIRE(obj.has_value());

        ::check_expected_emplace_not_void(obj, new_value);
    }

    template <class Value, class Error>
    void check_expected_emplace_in_error(Error const & error, Value const & value)
    {
        saga::expected<Value, Error> obj(saga::unexpect, error);

        REQUIRE(!obj.has_value());

        ::check_expected_emplace_not_void(obj, value);
    }
}

TEST_CASE("expected<void, Error>::emplace")
{
    check_expected_void_emplace_with_value<int>();
    check_expected_void_emplace_with_value<std::string>();

    saga_test::property_checker
        << ::check_expected_void_emplace_with_error<int>
        << ::check_expected_void_emplace_with_error<std::string>;
}

TEST_CASE("expected<Value, Error>::emplace in value")
{
    // @todo Проверить для разных типов аргументов emplace
    saga_test::property_checker
        << ::check_expected_emplace_in_value<long, int*>
        << ::check_expected_emplace_in_value<std::string, int*>
        << ::check_expected_emplace_in_value<std::string, std::vector<int>>
        << ::check_expected_emplace_in_value<int, std::vector<int>>;
}

TEST_CASE("expected<Value, Error>::emplace in error")
{
    // @todo Проверить для разных типов аргументов emplace
    saga_test::property_checker
        << ::check_expected_emplace_in_error<long, int>
        << ::check_expected_emplace_in_error<std::string, int>
        << ::check_expected_emplace_in_error<std::string, std::vector<int>>
        << ::check_expected_emplace_in_error<int, std::vector<int>>;
}

TEST_CASE("expected<Value, Error>::emplace with initializer_list")
{
    {
        using Value = initializer_list_consumer;
        using Error = long;

        static_assert(std::is_trivially_destructible<Value>{} &&
                      std::is_trivially_destructible<Error>{}, "");

        using Expected = saga::expected<Value, Error>;

        Expected const expected(saga::in_place_t{}, {1, 2, 3, 4}, 5);

        {
            Expected obj(saga::unexpect, 42);
            auto & result = obj.emplace({1, 2, 3, 4}, 5);

            REQUIRE(obj == expected);
            REQUIRE(std::addressof(result) == std::addressof(obj.value()));
            static_assert(std::is_same<decltype(result), Value &>{}, "");
        }

        {
            Expected obj(saga::in_place, {13}, 42);
            auto & result = obj.emplace({1, 2, 3, 4}, 5);

            REQUIRE(obj == expected);
            REQUIRE(std::addressof(result) == std::addressof(obj.value()));
            static_assert(std::is_same<decltype(result), Value &>{}, "");
        }
    }

    saga_test::property_checker << [](int value1, int value2)
    {
        using Compare = bool(*)(int const &, int const &);
        using Container = std::set<int, Compare>;

        auto const cmp = Compare([](int const & x, int const & y) { return x < y; });

        using Error = std::string;
        static_assert(!std::is_trivially_destructible<Container>{} ||
                      std::is_trivially_destructible<Error>{}, "");
        using Expected = saga::expected<Container, Error>;
        Expected const expected(saga::in_place_t{}, {value1, value2}, cmp);

        {
            Expected obj(saga::unexpect, "abc");
            auto & result = obj.emplace({value1, value2}, cmp);

            REQUIRE(obj == expected);
            REQUIRE(std::addressof(result) == std::addressof(obj.value()));
            static_assert(std::is_same<decltype(result), Container &>{}, "");
        }
        {
            Expected obj(saga::in_place);
            auto & result = obj.emplace({value1, value2}, cmp);

            REQUIRE(obj == expected);
            REQUIRE(std::addressof(result) == std::addressof(obj.value()));
            static_assert(std::is_same<decltype(result), Container &>{}, "");
        }
    };
}

// 4.4 Обмен
namespace
{
    template <class Value, class Error>
    void check_expected_swap_member(::expected_carrier<Value, Error> const & lhs_old,
                                    ::expected_carrier<Value, Error> const & rhs_old)
    {
        auto lhs = lhs_old;
        auto rhs = rhs_old;

        lhs.value.swap(rhs.value);

        static_assert(std::is_same<void, decltype(lhs.value.swap(rhs.value))>{}, "");

        static_assert(noexcept(lhs.value.swap(rhs.value))
                      == ((std::is_void<Value>{} || (std::is_nothrow_move_constructible<Value>{}
                         && saga::is_nothrow_swappable<Value>{}))
                         && std::is_nothrow_move_constructible<Error>{}
                         && saga::is_nothrow_swappable<Error>{}), "");

        REQUIRE(lhs == rhs_old);
        REQUIRE(rhs == lhs_old);
    }

    template <class Value, class Error>
    void check_expected_swap_free(::expected_carrier<Value, Error> const & lhs_old,
                                  ::expected_carrier<Value, Error> const & rhs_old)
    {
        auto lhs = lhs_old;
        auto rhs = rhs_old;

        swap(lhs.value, rhs.value);

        static_assert(std::is_same<void, decltype(swap(lhs.value, rhs.value))>{}, "");

        static_assert(noexcept(swap(lhs.value, rhs.value)) == noexcept(lhs.value.swap(rhs.value)), "");

        REQUIRE(lhs == rhs_old);
        REQUIRE(rhs == lhs_old);
    }

    template <class Value, class Error>
    void check_expected_swap(::expected_carrier<Value, Error> const & lhs,
                             ::expected_carrier<Value, Error> const & rhs)
    {
        check_expected_swap_member(lhs, rhs);
        check_expected_swap_free(lhs, rhs);
    }

    template <class T>
    using swap_return = decltype(std::declval<T&>().swap(std::declval<T&>()));

    struct throwing_move_int
    {
        int value = 0;

        throwing_move_int(int init_value)
         : value(init_value)
        {}

        throwing_move_int(throwing_move_int const & rhs) noexcept(false)
         : value(rhs.value)
        {}

        throwing_move_int & operator=(throwing_move_int const & rhs) noexcept(false)
        {
            this->value = rhs.value;
            return *this;
        }

        ~throwing_move_int() = default;

        friend bool operator==(throwing_move_int const & lhs, throwing_move_int const & rhs)\
        {
            return lhs.value == rhs.value;
        }
    };
}

namespace saga_test
{
    template <>
    struct arbitrary<throwing_move_int>
    {
        using value_type = throwing_move_int;

        template <class UniformRandomBitGeneration>
        static value_type generate(generation_t gen, UniformRandomBitGeneration & urbg)
        {
            return value_type(saga_test::arbitrary<int>::generate(gen, urbg));
        }
    };
}

TEST_CASE("expected::swap")
{
    static_assert(std::is_nothrow_move_constructible<int>{}, "");
    static_assert(std::is_nothrow_move_constructible<std::string>{}, "");
    static_assert(!std::is_nothrow_move_constructible<::throwing_move_int>{}, "");

    static_assert(saga::is_swappable<int>{}, "");
    static_assert(saga::is_swappable<std::string>{}, "");
    static_assert(saga::is_swappable<std::vector<int>>{}, "");
    static_assert(saga::is_swappable<::throwing_move_ctor>{}, "");

    static_assert(!saga::is_swappable<saga_test::not_swapable>{}, "");


    // Swappable
    static_assert(!saga::is_swappable<saga::expected<void, saga_test::not_swapable>>{}, "");
    static_assert(!saga::is_swappable<saga::expected<void const, saga_test::not_swapable>>{}, "");
    static_assert(!saga::is_swappable<saga::expected<void volatile, saga_test::not_swapable>>{}, "");
    static_assert(!saga::is_swappable<saga::expected<void const volatile, saga_test::not_swapable>>{}, "");

    static_assert(!saga::is_swappable<saga::expected<int, saga_test::not_swapable>>{}, "");
    static_assert(!saga::is_swappable<saga::expected<saga_test::not_swapable, int>>{}, "");
    static_assert(!saga::is_swappable<saga::expected<::throwing_move_ctor, ::throwing_move_ctor>>{}, "");

    // Проверка свойств swap
    saga_test::property_checker
        << ::check_expected_swap<void, int>
        << ::check_expected_swap<void, std::string>
        << ::check_expected_swap<void const, int>
        << ::check_expected_swap<void const, std::string>
        << ::check_expected_swap<void volatile, int>
        << ::check_expected_swap<void volatile, std::string>
        << ::check_expected_swap<void const volatile, int>
        << ::check_expected_swap<void const volatile, std::string>

        << ::check_expected_swap<long, int>
        << ::check_expected_swap<std::vector<int>, int>
        << ::check_expected_swap<long, std::string>
        << ::check_expected_swap<std::vector<int>, std::string>
        << ::check_expected_swap<int, ::throwing_move_int>
        // @todo Больше разных типов, в том числе, возбуждающих исключения при перемещении и обмене
        ;
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
        {
            // @todo Нормально ли, что эта ветка пуста?
        }
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

namespace
{
    template <class T, class SFINAE = void>
    struct has_dereference_op
     : std::false_type
    {};

    template <class T>
    struct has_dereference_op<T, saga::void_t<decltype(*std::declval<T>())>>
     : std::true_type
    {};
}

static_assert(::has_dereference_op<int*>{}, "");
static_assert(!::has_dereference_op<int>{}, "");

static_assert(::has_dereference_op<saga::expected<int, long>>{}, "");
static_assert(::has_dereference_op<saga::expected<std::string, long>>{}, "");

static_assert(!::has_dereference_op<saga::expected<void, long> &>{}, "");
static_assert(!::has_dereference_op<saga::expected<void const, long> &>{}, "");
static_assert(!::has_dereference_op<saga::expected<void volatile, long> &>{}, "");
static_assert(!::has_dereference_op<saga::expected<void const volatile, long> &>{}, "");

static_assert(!::has_dereference_op<saga::expected<void, long> const &>{}, "");
static_assert(!::has_dereference_op<saga::expected<void const, long> const &>{}, "");
static_assert(!::has_dereference_op<saga::expected<void volatile, long> const &>{}, "");
static_assert(!::has_dereference_op<saga::expected<void const volatile, long> const &>{}, "");

static_assert(!::has_dereference_op<saga::expected<void, long> &&>{}, "");
static_assert(!::has_dereference_op<saga::expected<void const, long> &&>{}, "");
static_assert(!::has_dereference_op<saga::expected<void volatile, long> &&>{}, "");
static_assert(!::has_dereference_op<saga::expected<void const volatile, long> &&>{}, "");

static_assert(!::has_dereference_op<saga::expected<void, long> const &&>{}, "");
static_assert(!::has_dereference_op<saga::expected<void const, long> const &&>{}, "");
static_assert(!::has_dereference_op<saga::expected<void volatile, long> const &&>{}, "");
static_assert(!::has_dereference_op<saga::expected<void const volatile, long> const &&>{}, "");

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
