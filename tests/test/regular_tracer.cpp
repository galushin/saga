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
#include <saga/test/regular_tracer.hpp>

// Тестовая инфраструктура
#include <catch/catch.hpp>

// Вспомогательные файлы
#include <set>

// Тесты
static_assert(std::is_same<saga::regular_tracer<long>::value_type, long>{}, "");
static_assert(std::is_same<saga::regular_tracer<long>::tag_type, void>{}, "");

static_assert(std::is_same<saga::regular_tracer<char, long>::value_type, char>{}, "");
static_assert(std::is_same<saga::regular_tracer<char, long>::tag_type, long>{}, "");

TEST_CASE("tracer: value constructor")
{
    using Value = int;
    using Tracer = saga::regular_tracer<Value>;

    auto const constructed_old = Tracer::constructed();
    auto const destructed_old = Tracer::destructed();
    auto const copy_constructed_old = Tracer::copy_constructed();
    auto const move_constructed_old = Tracer::move_constructed();
    auto const copy_assignments_old = Tracer::copy_assignments();
    auto const move_assignments_old = Tracer::move_assignments();
    auto const equality_comparision_old = Tracer::equality_comparisons();

    {
        auto const value = Value(42);
        Tracer const obj(value);

        REQUIRE(obj.value() == value);
    }

    REQUIRE(Tracer::constructed() == constructed_old + 1);
    REQUIRE(Tracer::destructed() == destructed_old + 1);

    REQUIRE(Tracer::copy_constructed() == copy_constructed_old);
    REQUIRE(Tracer::move_constructed() == move_constructed_old);
    REQUIRE(Tracer::copy_assignments() == copy_assignments_old);
    REQUIRE(Tracer::move_assignments() == move_assignments_old);
    REQUIRE(Tracer::equality_comparisons() == equality_comparision_old);
}

TEST_CASE("tracer: copy constructor and equality")
{
    using Value = int;
    using Tracer = saga::regular_tracer<Value>;

    auto const constructed_old = Tracer::constructed();
    auto const destructed_old = Tracer::destructed();
    auto const copy_constructed_old = Tracer::copy_constructed();
    auto const move_constructed_old = Tracer::move_constructed();
    auto const copy_assignments_old = Tracer::copy_assignments();
    auto const move_assignments_old = Tracer::move_assignments();
    auto const equality_comparision_old = Tracer::equality_comparisons();

    {
        auto const value = Value(42);

        Tracer const src(value);
        Tracer const dest(src);

        REQUIRE(dest.value() == value);
        REQUIRE(dest == src);
    }

    REQUIRE(Tracer::constructed() == constructed_old + 2);
    REQUIRE(Tracer::destructed() == destructed_old + 2);
    REQUIRE(Tracer::copy_constructed() == copy_constructed_old + 1);
    REQUIRE(Tracer::equality_comparisons() == equality_comparision_old + 1);

    REQUIRE(Tracer::move_constructed() == move_constructed_old);
    REQUIRE(Tracer::copy_assignments() == copy_assignments_old);
    REQUIRE(Tracer::move_assignments() == move_assignments_old);
}

TEST_CASE("tracer: not-equal")
{
    using Value = int;
    using Tracer = saga::regular_tracer<Value>;

    auto const constructed_old = Tracer::constructed();
    auto const destructed_old = Tracer::destructed();
    auto const copy_constructed_old = Tracer::copy_constructed();
    auto const move_constructed_old = Tracer::move_constructed();
    auto const copy_assignments_old = Tracer::copy_assignments();
    auto const move_assignments_old = Tracer::move_assignments();
    auto const equality_comparision_old = Tracer::equality_comparisons();

    {
        auto const value_1 = Value(42);
        auto const value_2 = Value(13);

        REQUIRE(value_1 != value_2);

        Tracer const obj1(value_1);
        Tracer const obj1_copy(obj1);
        Tracer const obj2(value_2);

        REQUIRE(obj1 == obj1);
        REQUIRE(obj1 == obj1_copy);
        REQUIRE(obj2 == obj2);
        REQUIRE(obj1 != obj2);
    }

    REQUIRE(Tracer::constructed() == constructed_old + 3);
    REQUIRE(Tracer::destructed() == destructed_old + 3);
    REQUIRE(Tracer::copy_constructed() == copy_constructed_old + 1);
    REQUIRE(Tracer::equality_comparisons() == equality_comparision_old + 4);

    REQUIRE(Tracer::move_constructed() == move_constructed_old);
    REQUIRE(Tracer::copy_assignments() == copy_assignments_old);
    REQUIRE(Tracer::move_assignments() == move_assignments_old);
}

TEST_CASE("tracer: move constructor")
{
    using Element = int;
    using Value = std::unique_ptr<Element>;
    using Tracer = saga::regular_tracer<Value>;

    auto const constructed_old = Tracer::constructed();
    auto const destructed_old = Tracer::destructed();
    auto const copy_constructed_old = Tracer::copy_constructed();
    auto const move_constructed_old = Tracer::move_constructed();
    auto const copy_assignments_old = Tracer::copy_assignments();
    auto const move_assignments_old = Tracer::move_assignments();
    auto const equality_comparision_old = Tracer::equality_comparisons();

    {
        auto const value = Element(42);

        Tracer src(std::make_unique<Element>(value));
        auto const src_ptr = src.value().get();

        Tracer const dest(std::move(src));

        REQUIRE(!src.value());

        REQUIRE(dest.value().get() == src_ptr);
        REQUIRE(static_cast<bool>(dest.value()));
        REQUIRE(*dest.value() == value);
    }

    REQUIRE(Tracer::constructed() == constructed_old + 2);
    REQUIRE(Tracer::destructed() == destructed_old + 2);
    REQUIRE(Tracer::move_constructed() == move_constructed_old + 1);

    REQUIRE(Tracer::copy_constructed() == copy_constructed_old);
    REQUIRE(Tracer::copy_assignments() == copy_assignments_old);
    REQUIRE(Tracer::move_assignments() == move_assignments_old);
    REQUIRE(Tracer::equality_comparisons() == equality_comparision_old);
}

TEST_CASE("tracer: copy assignment")
{
    using Value = int;
    using Tracer = saga::regular_tracer<Value>;

    auto const constructed_old = Tracer::constructed();
    auto const destructed_old = Tracer::destructed();
    auto const copy_constructed_old = Tracer::copy_constructed();
    auto const move_constructed_old = Tracer::move_constructed();
    auto const copy_assignments_old = Tracer::copy_assignments();
    auto const move_assignments_old = Tracer::move_assignments();
    auto const equality_comparision_old = Tracer::equality_comparisons();

    {
        auto const value_1 = Value(42);
        auto const value_2 = Value(13);

        REQUIRE(value_1 != value_2);

        Tracer const src(value_1);
        Tracer dest(value_2);

        REQUIRE(dest != src);

        auto & result = (dest = src);

        REQUIRE(dest == src);
        REQUIRE(std::addressof(result) == std::addressof(dest));
    }

    REQUIRE(Tracer::constructed() == constructed_old + 2);
    REQUIRE(Tracer::destructed() == destructed_old + 2);
    REQUIRE(Tracer::equality_comparisons() == equality_comparision_old + 2);
    REQUIRE(Tracer::copy_assignments() == copy_assignments_old + 1);

    REQUIRE(Tracer::copy_constructed() == copy_constructed_old);
    REQUIRE(Tracer::move_constructed() == move_constructed_old);
    REQUIRE(Tracer::move_assignments() == move_assignments_old);
}

TEST_CASE("tracer: move assignment")
{
    using Element = int;
    using Value = std::unique_ptr<Element>;
    using Tracer = saga::regular_tracer<Value>;

    auto const constructed_old = Tracer::constructed();
    auto const destructed_old = Tracer::destructed();
    auto const copy_constructed_old = Tracer::copy_constructed();
    auto const move_constructed_old = Tracer::move_constructed();
    auto const copy_assignments_old = Tracer::copy_assignments();
    auto const move_assignments_old = Tracer::move_assignments();
    auto const equality_comparision_old = Tracer::equality_comparisons();

    {
        auto const value_1 = Element(42);
        auto const value_2 = Element(13);

        REQUIRE(value_1 != value_2);

        Tracer src(std::make_unique<Element>(value_1));
        Tracer dest(std::make_unique<Element>(value_2));

        auto const src_ptr = src.value().get();

        auto & result = (dest = std::move(src));

        REQUIRE(dest.value().get() == src_ptr);
        REQUIRE(static_cast<bool>(dest.value()));
        REQUIRE(*dest.value() == value_1);

        REQUIRE(std::addressof(result) == std::addressof(dest));
    }

    REQUIRE(Tracer::constructed() == constructed_old + 2);
    REQUIRE(Tracer::destructed() == destructed_old + 2);
    REQUIRE(Tracer::move_assignments() == move_assignments_old + 1);

    REQUIRE(Tracer::equality_comparisons() == equality_comparision_old);
    REQUIRE(Tracer::copy_assignments() == copy_assignments_old);
    REQUIRE(Tracer::copy_constructed() == copy_constructed_old);
    REQUIRE(Tracer::move_constructed() == move_constructed_old);
}

static_assert(!std::is_constructible<int, std::initializer_list<int>&>{}, "");
static_assert(!std::is_constructible<saga::regular_tracer<int, void*>
                                    , std::initializer_list<int>&>{}, "");

TEST_CASE("regular_tracer: initializer_list constructor")
{
    using Element = int;
    using Value = std::vector<Element>;
    using Tracer = saga::regular_tracer<Value>;

    REQUIRE(Tracer({3, 1, 4, 1, 5}).value() == Value{3, 1, 4, 1, 5});
}

TEST_CASE("regular_tracer: initializer_list constructor and more args")
{
    using Element = int;
    using Compare = bool(*)(Element const &, Element const &);
    using Value = std::set<Element, Compare>;
    using Tracer = saga::regular_tracer<Value>;

    auto const cmp = Compare([](int const & x, int const & y) { return x < y; });

    REQUIRE(Tracer({1, 2, 3}, cmp).value() == Value({1, 2, 3}, cmp));
}
