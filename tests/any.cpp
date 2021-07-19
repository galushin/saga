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
#include <saga/any.hpp>

// Тестовая инфраструктура
#include "./saga_test.hpp"
#include <catch/catch.hpp>

// Вспомогательные файлы
#include <saga/utility/as_const.hpp>
#include <saga/test/regular_tracer.hpp>

#include <set>

namespace
{
    using Value_types_list = std::tuple<int, double, std::string, std::vector<long>>;
}

// Тесты
TEST_CASE("any - default constructor")
{
    saga::any const obj{};

    static_assert(noexcept(saga::any{}), "default ctor must be noexcept");

    REQUIRE(!obj.has_value());
    static_assert(noexcept(obj.has_value()), "has_value must be noexcept");

    REQUIRE(obj.type() == typeid(void));
    static_assert(noexcept(obj.type()), "type() must be noexcept");
    static_assert(std::is_same<decltype(obj.type()), std::type_info const &>{}, "");

    REQUIRE(saga::any_cast<std::string>(&obj) == nullptr);
    static_assert(std::is_same<decltype(saga::any_cast<long>(&obj)), long const *>{}, "");
    static_assert(noexcept(saga::any_cast<std::string>(&obj))
                  , "any_cast for pointer must be noexcept");

    saga::any var{};
    REQUIRE(var.has_value() == false);
    REQUIRE(var.type() == typeid(void));
    REQUIRE(saga::any_cast<double>(&var) == nullptr);
    static_assert(std::is_same<decltype(saga::any_cast<long>(&var)), long *>{}, "");
    static_assert(noexcept(saga::any_cast<std::string>(&var)), "");
}

TEST_CASE("any_cast for nullptr")
{
    REQUIRE(saga::any_cast<double>(static_cast<saga::any*>(nullptr)) == nullptr);
    REQUIRE(saga::any_cast<std::vector<int>>((static_cast<saga::any const*>(nullptr))) == nullptr);
}

TEMPLATE_LIST_TEST_CASE("any - value constructor", "any", Value_types_list)
{
    using Value = saga::regular_tracer<TestType>;

    saga_test::property_checker << [](Value const & value)
    {
        saga::any const obj(value);

        REQUIRE(obj.has_value());
        REQUIRE(obj.type() == typeid(Value));

        auto ptr = saga::any_cast<Value>(&obj);
        static_assert(std::is_same<decltype(ptr), Value const *>{}, "");

        REQUIRE(ptr != nullptr);
        REQUIRE(*ptr == value);
    };

    REQUIRE(Value::constructed() == Value::destructed());
}

static_assert(!std::is_constructible<saga::any, std::unique_ptr<int>>{}
              , "Must be Cpp17CopyConstructible");

TEST_CASE("any - value constructor performs move")
{
    using Container = saga::regular_tracer<std::vector<int>>;

    saga_test::property_checker << [](Container const & value_old)
    {
        auto value = value_old;

        auto const move_constructed_old = Container::move_constructed();

        saga::any const obj(std::move(value));

        REQUIRE(value.value().empty());
        REQUIRE(Container::move_constructed() == move_constructed_old + 1);

        REQUIRE(obj.has_value());
        REQUIRE(obj.type() == typeid(Container));

        auto ptr = saga::any_cast<Container>(&obj);

        REQUIRE(ptr != nullptr);
        REQUIRE(*ptr == value_old);
    };

    REQUIRE(Container::constructed() == Container::destructed());
}

TEMPLATE_LIST_TEST_CASE("any_cast mutable pointer", "any", Value_types_list)
{
    using Value = saga::regular_tracer<TestType>;

    REQUIRE(saga::any_cast<Value>(static_cast<saga::any*>(nullptr)) == nullptr);

    saga_test::property_checker << [](Value const & value)
    {
        saga::any var(value);

        REQUIRE(var.has_value());
        REQUIRE(var.type() == typeid(Value));

        auto ptr = saga::any_cast<Value>(&var);
        static_assert(std::is_same<decltype(ptr), Value *>{}, "");

        REQUIRE(ptr != nullptr);
        REQUIRE(*ptr == value);
    };

    REQUIRE(Value::constructed() == Value::destructed());
}

TEST_CASE("any: copy constructor from empty")
{
    saga::any const empty_src;

    saga::any const obj(empty_src);

    REQUIRE(obj.has_value() == false);
    REQUIRE(obj.type() == typeid(void));
    REQUIRE(saga::any_cast<long>(&obj) == nullptr);
    REQUIRE(saga::any_cast<std::vector<int>>(&obj) == nullptr);
}

TEMPLATE_LIST_TEST_CASE("any: copy constructor from not empty", "any", Value_types_list)
{
    using Value = saga::regular_tracer<TestType>;

    saga_test::property_checker << [](Value const & value)
    {
        saga::any const src(value);

        saga::any const obj(src);

        REQUIRE(obj.has_value());
        REQUIRE(obj.type() == src.type());
        REQUIRE(saga::any_cast<Value>(&obj) != nullptr);
        REQUIRE(*saga::any_cast<Value>(&obj) == value);
    };

    REQUIRE(Value::constructed() == Value::destructed());
}

TEST_CASE("any: copy assignment")
{
    using Value1 = saga::regular_tracer<std::string>;
    using Value2 = saga::regular_tracer<long>;

    saga_test::property_checker << [](Value1 const & value_1, Value2 const & value_2)
    {
        saga::any const src_0{};
        saga::any const src_1(value_1);
        saga::any const src_2(value_2);

        saga::any var;

        static_assert(std::is_same<decltype(var = src_0), saga::any &>{}, "");

        auto const & res1 = (var = src_0);

        REQUIRE(var.has_value() == false);
        REQUIRE(var.type() == typeid(void));
        REQUIRE(saga::any_cast<Value1>(&var) == nullptr);
        REQUIRE(saga::any_cast<Value2>(&var) == nullptr);
        REQUIRE(std::addressof(res1) == std::addressof(var));

        auto const & res2 = (var = src_1);

        REQUIRE(var.has_value());
        REQUIRE(var.type() == typeid(Value1));
        REQUIRE(saga::any_cast<Value1>(&var) != nullptr);
        REQUIRE(saga::any_cast<Value2>(&var) == nullptr);
        REQUIRE(*saga::any_cast<Value1>(&var) == value_1);
        REQUIRE(std::addressof(res2) == std::addressof(var));

        auto const & res3 = (var = src_2);

        REQUIRE(var.has_value());
        REQUIRE(var.type() == typeid(Value2));
        REQUIRE(saga::any_cast<Value1>(&var) == nullptr);
        REQUIRE(saga::any_cast<Value2>(&var) != nullptr);
        REQUIRE(*saga::any_cast<Value2>(&var) == value_2);
        REQUIRE(std::addressof(res3) == std::addressof(var));

        auto const & res4 = (var = src_0);

        REQUIRE(var.has_value() == false);
        REQUIRE(var.type() == typeid(void));
        REQUIRE(saga::any_cast<Value1>(&var) == nullptr);
        REQUIRE(saga::any_cast<Value2>(&var) == nullptr);
        REQUIRE(std::addressof(res4) == std::addressof(var));
    };
}

static_assert(std::is_base_of<std::bad_cast, saga::bad_any_cast>{}, "");

TEST_CASE("bad_any_cast")
{
    saga::bad_any_cast obj;

    REQUIRE(std::string(obj.what()).empty() == false);
}

TEMPLATE_LIST_TEST_CASE("any_cast: from empty", "any", Value_types_list)
{
    using Value = TestType;

    saga::any obj;
    REQUIRE(obj.has_value() == false);

    REQUIRE_THROWS_AS(saga::any_cast<Value>(obj), saga::bad_any_cast);
    REQUIRE_THROWS_AS(saga::any_cast<Value>(saga::as_const(obj)), saga::bad_any_cast);
    REQUIRE_THROWS_AS(saga::any_cast<Value>(std::move(obj)), saga::bad_any_cast);
}

TEMPLATE_LIST_TEST_CASE("any_cast: value and reference", "any", Value_types_list)
{
    using Value = saga::regular_tracer<TestType>;

    saga_test::property_checker << [](Value const & old_value, Value const & new_value)
    {
        saga::any obj(old_value);

        static_assert(std::is_same<decltype(saga::any_cast<Value>(obj)), Value>{}, "");
        static_assert(std::is_same<decltype(saga::any_cast<Value&>(obj)), Value&>{}, "");
        static_assert(std::is_same<decltype(saga::any_cast<Value const&>(saga::as_const(obj)))
                                  , Value const&>{}, "");

        REQUIRE(saga::any_cast<Value>(obj) == old_value);

        saga::any_cast<Value&>(obj) = new_value;

        REQUIRE(saga::any_cast<Value const &>(saga::as_const(obj)) == new_value);
    };

    REQUIRE(Value::constructed() == Value::destructed());
}

TEST_CASE("any_cast: rvalue reference")
{
    using Container = saga::regular_tracer<std::vector<int>>;

    saga_test::property_checker << [](Container const & value)
    {
        saga::any obj(value);

        auto res = saga::any_cast<Container>(std::move(obj));

        REQUIRE(res == value);
        REQUIRE(saga::any_cast<Container const &>(obj).value().empty());
    };

    REQUIRE(Container::constructed() == Container::destructed());
}

TEST_CASE("any: copy of non-const any")
{
    using Value = saga::regular_tracer<std::string>;

    saga_test::property_checker << [](Value const & value)
    {
        saga::any src(value);

        saga::any const dest(src);

        REQUIRE(saga::any_cast<Value const &>(dest) == value);
    };

    REQUIRE(Value::constructed() == Value::destructed());
}

TEST_CASE("any: in_place_type ctor no args")
{
    using Container = saga::regular_tracer<std::vector<long>>;

    {
        saga::any const obj(saga::in_place_type<Container>);
        REQUIRE(saga::any_cast<const Container&>(obj) == Container{});
    }

    REQUIRE(Container::constructed() == Container::destructed());
}

static_assert(!std::is_constructible<saga::any, saga::in_place_type_t<std::unique_ptr<int>>>{}, "");

static_assert(std::is_constructible<saga::any, saga::in_place_type_t<int>>{}, "");
static_assert(!std::is_convertible<saga::in_place_type_t<int>, saga::any>{}, "");

static_assert(!std::is_constructible<int, std::string>{}, "");
static_assert(!std::is_constructible<saga::any, saga::in_place_type_t<int>, std::string>{}, "");

TEST_CASE("any: in_place_type ctor with args")
{
    using Element = int;
    using Container = std::vector<Element>;
    using Tracer = saga::regular_tracer<Container>;

    saga_test::property_checker << [](saga_test::container_size<Container::size_type> num
                                      , Element const & value)
    {
        saga::any const obj(saga::in_place_type<Tracer>, num, value);

        REQUIRE(saga::any_cast<const Tracer&>(obj) == Tracer(num, value));
    };

    REQUIRE(Tracer::constructed() == Tracer::destructed());
}

TEST_CASE("any: placement constructor with initializer list")
{
    using Element = int;
    using Container = saga::regular_tracer<std::vector<Element>>;

    saga_test::property_checker << [](Element const & value1, Element const & value2)
    {
        Container const expected({value1, value2});

        saga::any const actual(saga::in_place_type<Container>, {value1, value2});

        REQUIRE(saga::any_cast<Container const &>(actual) == expected);
    };

    REQUIRE(Container::constructed() == Container::destructed());
}

namespace
{
    template <class T>
    using detect_return_implicit_initializer_list
        = decltype(saga::any_cast<int>({saga::in_place_type<std::vector<int>>, {1, 2, 3}}));

    struct no_copy_init_list
    {
        explicit no_copy_init_list(std::initializer_list<int>);

        no_copy_init_list(no_copy_init_list const &) = delete;
        no_copy_init_list & operator=(no_copy_init_list const &) = delete;
    };
}

static_assert(!saga::is_detected<detect_return_implicit_initializer_list, saga::any>{}
              , "Must be explicit");

static_assert(std::is_constructible<::no_copy_init_list, std::initializer_list<int>&>{}, "");
static_assert(!std::is_constructible<saga::any, saga::in_place_type_t<::no_copy_init_list>
                                                , std::initializer_list<int>&>{}, "");

TEST_CASE("any: placement constructor with initializer list and more args")
{
    using Element = int;
    using Compare = bool(*)(Element const &, Element const &);
    using Container = saga::regular_tracer<std::set<Element, Compare>>;

    saga_test::property_checker << [](Element value1, Element value2)
    {
        auto const cmp = Compare([](Element const & x, Element const & y) { return x < y; });

        Container const expected({value1, value2}, cmp);

        saga::any const obj(saga::in_place_type<Container>, {value1, value2}, cmp);

        REQUIRE(saga::any_cast<Container const &>(obj) == expected);
    };

    REQUIRE(Container::constructed() == Container::destructed());
}

static_assert(!std::is_constructible<int, std::initializer_list<int>&>{}, "");
static_assert(!std::is_constructible<saga::any, saga::in_place_type_t<int>
                                              , std::initializer_list<int>&>{}, "");

TEST_CASE("make_any: no initializer_list")
{
    using Value = int;
    using Container = saga::regular_tracer<std::vector<Value>>;

    saga_test::property_checker << [](saga_test::container_size<std::size_t> num
                                      , Value const & value)
    {
        auto obj = saga::make_any<Container>(num, value);

        static_assert(std::is_same<decltype(obj), saga::any>{}, "");

        REQUIRE(saga::any_cast<const Container&>(obj) == Container(num, value));
    };

    REQUIRE(Container::constructed() == Container::destructed());
}

TEST_CASE("make_any: with initializer list and more args")
{
    using Element = int;
    using Compare = bool(*)(Element const &, Element const &);
    using Container = saga::regular_tracer<std::set<Element, Compare>>;

    saga_test::property_checker << [](int value1, int value2)
    {
        auto const cmp = Compare([](int const & x, int const & y) { return x < y; });

        Container const expected({value1, value2}, cmp);

        auto obj = saga::make_any<Container>({value1, value2}, cmp);
        static_assert(std::is_same<decltype(obj), saga::any>{}, "");

        REQUIRE(saga::any_cast<Container const &>(obj) == expected);
    };

    REQUIRE(Container::constructed() == Container::destructed());
}

static_assert(std::is_nothrow_move_constructible<saga::any>{}, "any must be nothrow move constructible");
static_assert(std::is_nothrow_move_assignable<saga::any>{}, "any must be nothrow move assignable");

TEST_CASE("any: move constructor from empty")
{
    saga::any empty_src;

    saga::any const obj(std::move(empty_src));

    REQUIRE(empty_src.has_value() == false);
    REQUIRE(empty_src.type() == typeid(void));
    REQUIRE(saga::any_cast<std::vector<int>>(&empty_src) == nullptr);

    REQUIRE(obj.has_value() == false);
    REQUIRE(obj.type() == typeid(void));
    REQUIRE(saga::any_cast<std::vector<int>>(&obj) == nullptr);
}

TEST_CASE("any: move constructor from big value")
{
    using Container = saga::regular_tracer<std::vector<int>>;

    saga_test::property_checker << [](Container const & value)
    {
        saga::any src(value);

        auto const old_data = saga::any_cast<Container const &>(src).value().data();

        saga::any const obj(std::move(src));

        REQUIRE(saga::any_cast<Container const &>(obj) == value);
        REQUIRE(saga::any_cast<Container const &>(obj).value().data() == old_data);

        REQUIRE((!src.has_value() || saga::any_cast<Container const &>(obj).value().empty()));
    };

    REQUIRE(Container::constructed() == Container::destructed());
}

TEST_CASE("any: move assignment")
{
    using Value1 = saga::regular_tracer<std::vector<long>>;
    using Value2 = saga::regular_tracer<std::vector<int>>;

    saga_test::property_checker << [](Value1 const & value_1, Value2 const & value_2)
    {
        saga::any src_0{};
        saga::any src_1(value_1);
        saga::any src_2(value_2);

        auto const old_data_1 = saga::any_cast<Value1 const &>(src_1).value().data();
        auto const old_data_2 = saga::any_cast<Value2 const &>(src_2).value().data();

        saga::any var;

        static_assert(std::is_same<decltype(var = std::move(src_0)), saga::any &>{}, "");

        auto const & res1 = (var = std::move(src_0));

        REQUIRE(var.has_value() == false);
        REQUIRE(std::addressof(res1) == std::addressof(var));
        REQUIRE(!src_0.has_value());

        auto const & res2 = (var = std::move(src_1));

        REQUIRE(saga::any_cast<Value1 const &>(var) == value_1);
        REQUIRE(saga::any_cast<Value1 const &>(var).value().data() == old_data_1);
        REQUIRE(std::addressof(res2) == std::addressof(var));

        auto const & res3 = (var = std::move(src_2));

        REQUIRE(saga::any_cast<Value2 const &>(var) == value_2);
        REQUIRE(saga::any_cast<Value2 const &>(var).value().data() == old_data_2);
        REQUIRE(std::addressof(res3) == std::addressof(var));

        auto const & res4 = (var = std::move(src_0));

        REQUIRE(var.has_value() == false);
        REQUIRE(std::addressof(res4) == std::addressof(var));
    };

    REQUIRE(Value1::constructed() == Value1::destructed());
    REQUIRE(Value2::constructed() == Value2::destructed());
}

TEST_CASE("any: reset empty")
{
    saga::any obj;

    obj.reset();

    REQUIRE(obj.has_value() == false);

    static_assert(noexcept(obj.reset()), "any::reset must be noexcept");
}

TEMPLATE_LIST_TEST_CASE("any: reset not empty", "any", Value_types_list)
{
    using Value = saga::regular_tracer<TestType>;

    saga_test::property_checker << [](Value const & value)
    {
        saga::any obj(value);

        REQUIRE(saga::any_cast<Value const &>(obj) == value);

        obj.reset();

        REQUIRE(obj.has_value() == false);
    };

    REQUIRE(Value::constructed() == Value::destructed());
}

TEST_CASE("any::emplace: no args")
{
    using Value = saga::regular_tracer<std::vector<long>>;

    {
        saga::any obj;

        auto const & result = obj.emplace<Value>();

        REQUIRE(saga::any_cast<Value const &>(obj) == Value{});

        static_assert(std::is_same<decltype(obj.emplace<Value>()), Value&>{}, "");
        REQUIRE(std::addressof(result) == std::addressof(saga::any_cast<Value const &>(obj)));
    }

    using Other = saga::regular_tracer<int>;

    saga_test::property_checker << [](Other const & init_value)
    {
        saga::any obj(init_value);
        auto const & result = obj.emplace<Value>();

        REQUIRE(saga::any_cast<Value const &>(obj) == Value{});

        REQUIRE(std::addressof(result) == std::addressof(saga::any_cast<Value const &>(obj)));
    };

    REQUIRE(Value::constructed() == Value::destructed());
    REQUIRE(Other::constructed() == Other::destructed());
}

namespace
{
    template <class Container, class Element>
    void check_any_emplace(saga::any & obj, std::size_t num, Element const & value)
    {
        auto const & result = obj.emplace<Container>(num, value);

        REQUIRE(saga::any_cast<const Container&>(obj) == Container(num, value));

        static_assert(std::is_same<decltype(obj.emplace<Container>(num, value)), Container&>{}, "");
        REQUIRE(std::addressof(result) == std::addressof(saga::any_cast<Container const &>(obj)));
    }

    template <class Container, class Element>
    void check_any_emplace_init_list(saga::any & obj
                                     , Element const & value1
                                     , Element const & value2)
    {

        Container const expected({value1, value2});

        auto & result = obj.emplace<Container>({value1, value2});

        REQUIRE(saga::any_cast<Container const &>(obj) == expected);

        static_assert(std::is_same<decltype(result), Container&>{}, "");
        REQUIRE(std::addressof(result) == std::addressof(saga::any_cast<Container const &>(obj)));
    }

    template <class Container, class Compare, class Element>
    void check_any_emplace_init_list_args(saga::any & obj,
                                          Element const & value1, Element const & value2)
    {
        auto const cmp = Compare([](Element const & x, Element const & y) { return x < y; });

        Container const expected({value1, value2}, cmp);

        auto & result = obj.emplace<Container>({value1, value2}, cmp);

        REQUIRE(saga::any_cast<Container const &>(obj) == expected);

        static_assert(std::is_same<decltype(result), Container&>{}, "");
        REQUIRE(std::addressof(result) == std::addressof(saga::any_cast<Container const &>(obj)));
    }

    template <class T, class... Args>
    using any_emplace_type = decltype(saga::any{}.emplace<T>(std::declval<Args>()...));
}

static_assert(std::is_same<decltype(saga::any{}.emplace<int const>()), int &>{}, "");
static_assert(std::is_same<decltype(saga::any{}.emplace<std::vector<int> const>({1, 2, 3}))
                           , std::vector<int> &>{}, "");

static_assert(!saga::is_detected<any_emplace_type, int, std::vector<int>>{}, "");
static_assert(!saga::is_detected<any_emplace_type, std::unique_ptr<int>>{}, "");

static_assert(!saga::is_detected<any_emplace_type, int, std::initializer_list<int>&>{}, "");
static_assert(!saga::is_detected<any_emplace_type,
                                 std::vector<int>, std::initializer_list<int>&, std::string>{}, "");

TEST_CASE("any::emplace with args")
{
    using Element = int;
    using Container = saga::regular_tracer<std::vector<Element>>;
    using Other = saga::regular_tracer<long>;

    saga_test::property_checker
    << [](saga_test::container_size<std::size_t> num, Element const & value)
    {
        saga::any obj;

        ::check_any_emplace<Container>(obj, num, value);
    }
    << [](Other const & other, saga_test::container_size<std::size_t> num, Element const & value)
    {
        saga::any obj(other);

        ::check_any_emplace<Container>(obj, num, value);
    };

    REQUIRE(Container::constructed() == Container::destructed());
    REQUIRE(Other::constructed() == Other::destructed());
}

TEST_CASE("any::emplace with initializer list")
{
    using Container = saga::regular_tracer<std::vector<int>>;
    using Other = saga::regular_tracer<std::string>;

    saga_test::property_checker
    << [](int value1, int value2)
    {
        saga::any obj;
        ::check_any_emplace_init_list<Container>(obj, value1, value2);
    }
    << [](Other const & other, int value1, int value2)
    {
        saga::any obj(other);

        ::check_any_emplace_init_list<Container>(obj, value1, value2);
    };

    REQUIRE(Container::constructed() == Container::destructed());
    REQUIRE(Other::constructed() == Other::destructed());
}

TEST_CASE("any::emplace: with initializer list and more args")
{
    using Other = saga::regular_tracer<std::string>;

    using Element = int;
    using Compare = bool(*)(Element const &, Element const &);
    using Container = saga::regular_tracer<std::set<Element, Compare>>;

    saga_test::property_checker
    << [](int value1, int value2)
    {
        saga::any obj;

        ::check_any_emplace_init_list_args<Container, Compare>(obj, value1, value2);
    }
    << [](std::string const & other, int value1, int value2)
    {
        saga::any obj(other);

        ::check_any_emplace_init_list_args<Container, Compare>(obj, value1, value2);
    };

    REQUIRE(Other::constructed() == Other::destructed());
    REQUIRE(Container::constructed() == Container::destructed());
}

namespace
{
    template <class T>
    void check_any_assign_value(saga::any & obj, T const & value)
    {
        auto & result = (obj = value);

        static_assert(std::is_same<decltype(result), saga::any &>{}, "");

        REQUIRE(std::addressof(result) == std::addressof(obj));
        REQUIRE(saga::any_cast<T const &>(obj) == value);
    }
}

TEST_CASE("any: assign value")
{
    using Value = saga::regular_tracer<int>;
    using Other = saga::regular_tracer<std::string>;

    saga_test::property_checker
    << [](Value const & value)
    {
        saga::any obj;
        ::check_any_assign_value(obj, value);
    }
    << [](Other const & other, Value const & value)
    {
        saga::any obj(other);
        ::check_any_assign_value(obj, value);
    };

    REQUIRE(Value::constructed() == Value::destructed());
    REQUIRE(Other::constructed() == Other::destructed());
}

TEST_CASE("any: assign temporary value do move")
{
    using Container = saga::regular_tracer<std::vector<int>>;
    using Other = saga::regular_tracer<int>;

    saga_test::property_checker << [](Other const & other, Container const & old_value)
    {
        saga::any obj(other);

        auto tmp = old_value;
        obj = std::move(tmp);

        REQUIRE(saga::any_cast<Container const &>(obj) == old_value);
        REQUIRE(tmp.value().empty());
    };

    REQUIRE(Container::constructed() == Container::destructed());
    REQUIRE(Other::constructed() == Other::destructed());
}

static_assert(!std::is_assignable<saga::any, std::unique_ptr<int>>{}, "");

TEST_CASE("any: throwing assign value has no effect")
{
    using Value = saga::regular_tracer<std::string>;

    saga_test::property_checker << [](Value const & old_value, int const & new_value)
    {
        saga::any obj(old_value);
        auto const ptr_old = saga::any_cast<Value>(&obj);

        REQUIRE_THROWS(obj = saga_test::throws_on_move{new_value});

        REQUIRE(saga::any_cast<Value const &>(obj) == old_value);
        REQUIRE(saga::any_cast<Value>(&obj) == ptr_old);
    };

    REQUIRE(Value::constructed() == Value::destructed());
}

TEST_CASE("any: assign non-const any")
{
    saga::any src;
    saga::any dest(42);

    dest = src;

    REQUIRE(!dest.has_value());
}

TEST_CASE("any::swap: two empty")
{
    saga::any obj1;
    saga::any obj2;

    obj1.swap(obj2);
    static_assert(noexcept(obj1.swap(obj2)), "any::swap must be noexcept");

    REQUIRE(obj1.has_value() == false);
    REQUIRE(obj2.has_value() == false);

    swap(obj1, obj2);
    static_assert(noexcept(swap(obj1, obj2)), "swapof any's must be noexcept");

    REQUIRE(obj1.has_value() == false);
    REQUIRE(obj2.has_value() == false);
}

TEST_CASE("any::swap: empty and value")
{
    using Value = saga::regular_tracer<long>;

    saga_test::property_checker << [](Value const & value)
    {
        saga::any obj0;
        saga::any obj1(value);

        obj0.swap(obj1);

        REQUIRE(saga::any_cast<Value const &>(obj0) == value);
        REQUIRE(obj1.has_value() == false);

        swap(obj0, obj1);

        REQUIRE(obj0.has_value() == false);
        REQUIRE(saga::any_cast<Value const &>(obj1) == value);
    };

    REQUIRE(Value::constructed() == Value::destructed());
}

TEST_CASE("any::swap: two values")
{
    using Value1 = saga::regular_tracer<long>;
    using Value2 = saga::regular_tracer<std::string>;

    saga_test::property_checker << [](Value1 const & value1, Value2 const & value2)
    {
        saga::any obj1(value1);
        saga::any obj2(value2);

        obj1.swap(obj2);

        REQUIRE(saga::any_cast<Value2 const &>(obj1) == value2);
        REQUIRE(saga::any_cast<Value1 const &>(obj2) == value1);

        swap(obj1, obj2);

        REQUIRE(saga::any_cast<Value1 const &>(obj1) == value1);
        REQUIRE(saga::any_cast<Value2 const &>(obj2) == value2);
    };

    REQUIRE(Value1::constructed() == Value1::destructed());
    REQUIRE(Value2::constructed() == Value2::destructed());
}
