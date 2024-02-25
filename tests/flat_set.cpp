/* (c) 2024 Галушин Павел Викторович, galushin@gmail.com

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
#include <saga/flat_set.hpp>

// Инфраструктура тестирования
#include "saga_test.hpp"
#include <catch2/catch_amalgamated.hpp>

// Используемые возможности
#include <deque>

namespace
{
    template <class Container_1, class Container_2, class Compare>
    bool is_equivalent_by_compare(Container_1 const & lhs, Container_2 const & rhs, Compare cmp)
    {
        auto const equiv = [&cmp](typename Container_1::value_type const & lhs
                                 ,typename Container_2::value_type const & rhs)
        {
            return !cmp(lhs, rhs) && !cmp(rhs, lhs);
        };

        return saga::equal(saga::cursor::all(lhs), saga::cursor::all(rhs), equiv);
    }

    // @todo В библиотеку?
    template <class Container>
    struct is_container_reference_valid
     : std::is_same<typename Container::reference, typename Container::value_type &>
    {};

    template <class Container>
    struct is_container_const_reference_valid
     : std::is_same<typename Container::const_reference, typename Container::value_type const &>
    {};

    template <class Container>
    struct is_container_difference_type_valid
     : std::conjunction<std::is_signed<typename Container::difference_type>
                       ,std::is_same<typename Container::difference_type
                                    ,typename Container::iterator::difference_type>>
    {};

    template <class Container>
    struct is_container_size_type_valid
     : std::conjunction<std::is_unsigned<typename Container::size_type>
                       ,std::bool_constant<sizeof(typename Container::size_type)
                                           >= sizeof(typename Container::difference_type)>>
    {};
}

// Тесты
// Базовые тесты, на которые мы полагаемся позже
TEST_CASE("flat_set: constructor from elements, compare and allocator")
{
    using Element = int;
    using Compare = saga_test::strict_weak_order<Element>;
    using Allocator = std::allocator<Element>;
    using Container = std::deque<Element, Allocator>;
    using FlatSet = saga::flat_set<Element, Compare, Container>;


    saga_test::property_checker << [](std::vector<Element> const & src, Compare cmp)
    {
        // @todo static_assert(!std::allocator_traits<Allocator>::is_always_equal{});
        // @todo Чем-то инициализировать распределитель, чтобы проверка на равенство имела смысл
        // Возможно, потребуется специальный распределитель памяти для тестов
        Allocator alloc;

        std::set<Element, Compare> const expected(src.begin(), src.end(), cmp);

        FlatSet const actual(src.begin(), src.end(), cmp, alloc);

        CAPTURE(actual, expected);
        REQUIRE(::is_equivalent_by_compare(actual, expected, cmp));

        // Функция сравнения проверяется ещё и за счёт равенства с set: для одинаковых функций
        // сравнения должен получаться один и тот же порядок
        REQUIRE(actual.key_comp() == cmp);
        REQUIRE(actual.value_comp() == cmp);

        REQUIRE(actual.get_allocator() == alloc);
        static_assert(noexcept(actual.get_allocator()));
    };
}

TEST_CASE("flat_set: equality")
{
    using FlatSet = saga::flat_set<int>;
    using Container = FlatSet::container_type;

    saga_test::property_checker << [](Container const & src_lhs, Container const & src_rhs)
    {
        FlatSet const lhs(src_lhs);
        FlatSet const rhs(src_rhs);

        REQUIRE(lhs == lhs);
        REQUIRE(rhs == rhs);
        REQUIRE((lhs == rhs) == saga::equal(saga::cursor::all(lhs), saga::cursor::all(rhs)));
        REQUIRE((rhs == lhs) == (lhs == rhs));

        REQUIRE(!(lhs != lhs));
        REQUIRE(!(rhs != rhs));
        REQUIRE((lhs != rhs) == !(lhs == rhs));
        REQUIRE((rhs != lhs) == (lhs != rhs));
    };
}

// @todo Проверить требования к контейнерам ([container.reqmts]) - 24.2.2.2
// 24.2.2.2 Container requirements

static_assert(std::is_same<typename saga::flat_set<int>::value_type, int>{});
/* Выполнение Cpp17Erasable для value_type не проверяется, так как flat_set не является
осведомлённым о распределителях памяти (allocator-aware container)
- примечание к пункт 2.3 раздела 24.6.5.1
*/

static_assert(::is_container_reference_valid<saga::flat_set<std::string>>{});
static_assert(::is_container_const_reference_valid<saga::flat_set<std::string>>{});

/* Требования к iterator и const_iterator проверять не нужно: вместо этого мы проверяем, что они
они определены как синонимы итераторов контейнера.
Так как iterator и const_iterator определены одинаково, то можно не проверять и преобразование
*/
static_assert(std::is_same<typename saga::flat_set<int, std::greater<>, std::deque<int>>::iterator
                          ,typename std::deque<int>::const_iterator>{});
static_assert(std::is_same<typename saga::flat_set<int>::iterator
                          ,typename std::vector<int>::const_iterator>{});

static_assert(std::is_same<typename saga::flat_set<int>::const_iterator
                          ,typename saga::flat_set<int>::iterator>{});
static_assert(std::is_same<typename saga::flat_set<long, std::greater<>
                                                  ,std::deque<long>>::const_iterator
                          ,typename saga::flat_set<long, std::greater<>
                                                  ,std::deque<long>>::iterator>{});

// @todo тесты size_type и difference_type при явном задании контейнера с нестандартными типами
static_assert(::is_container_difference_type_valid<saga::flat_set<std::string>>{});
static_assert(::is_container_difference_type_valid<saga::flat_set<int, std::less<>
                                                                 ,std::deque<int>>>{});
static_assert(::is_container_size_type_valid<saga::flat_set<std::string>>{});
static_assert(::is_container_size_type_valid<saga::flat_set<int, std::less<>, std::deque<int>>>{});

TEST_CASE("flat_set: default constructor")
{
    saga::flat_set<long, std::greater<>, std::deque<long>> const obj{};

    CHECK(obj.empty());
    static_assert(noexcept(obj.empty()));
}

TEST_CASE("flat_set : copy constructor")
{
    using FlatSet = saga::flat_set<long>;
    using Container = FlatSet::container_type;

    saga_test::property_checker << [](Container const & src)
    {
        FlatSet const obj(src);
        FlatSet const obj_copy(obj);

        REQUIRE(obj_copy == obj);
    };
}

TEST_CASE("flat_set : move constructor")
{
    using FlatSet = saga::flat_set<long>;
    using Container = FlatSet::container_type;

    saga_test::property_checker << [](Container const & src)
    {
        FlatSet const temp_old(src);

        auto temp = temp_old;
        FlatSet const obj(std::move(temp));

        REQUIRE(obj == temp_old);
        REQUIRE(temp.empty());
    };
}

TEST_CASE("flat_set : copy assignment")
{
    using FlatSet = saga::flat_set<std::string>;
    using Container = FlatSet::container_type;

    saga_test::property_checker << [](Container const & src_lhs, Container const & src_rhs)
    {
        FlatSet lhs(src_lhs);
        FlatSet const rhs(src_rhs);

        static_assert(std::is_same<decltype(lhs = rhs), FlatSet &>{});

        lhs = rhs;

        REQUIRE(lhs == rhs);
    };
}

TEST_CASE("flat_set : move assignment")
{
    using FlatSet = saga::flat_set<long>;
    using Container = FlatSet::container_type;

    saga_test::property_checker << [](Container const & src_lhs, Container const & src_rhs)
    {
        FlatSet lhs(src_lhs);
        FlatSet const rhs_old(src_rhs);
        auto rhs = rhs_old;

        lhs = std::move(rhs);

        REQUIRE(lhs == rhs_old);
        REQUIRE(rhs.empty());

        static_assert(std::is_same<decltype(lhs = std::move(rhs)), FlatSet &>{});
    };
}

/* @todo Деструктор не проверяем: так как flat_set хранит элементы во вложенном контейнере,
достаточно убедиться, что flat_set вообще не объявляет деструктор? Или может быть проверить через
regular_tracer что все элементы уничтожены?
*/

// @todo begin, end, cbegin, cend

// @todo operator<=>

// Равенство проверено ранее

// @todo swap
namespace
{
    template <class T>
    void swap(T & lhs, T & rhs) = delete;
}

TEST_CASE("flat_set : swap")
{
    using FlatSet = saga::flat_set<long>;
    using Container = FlatSet::container_type;

    saga_test::property_checker << [](Container const & src_lhs, Container const & src_rhs)
    {
        FlatSet const lhs_old(src_lhs);
        FlatSet const rhs_old(src_rhs);

        auto lhs = lhs_old;
        auto rhs = rhs_old;

        // @todo Константная сложность
        lhs.swap(rhs);
        static_assert(noexcept(lhs.swap(rhs)));

        REQUIRE(lhs == rhs_old);
        REQUIRE(rhs == lhs_old);

        // @todo Проверить, что обменяны распределитель памяти и функция сравнения

        // @todo Константная сложность
        swap(lhs, rhs);
        static_assert(noexcept(swap(lhs, rhs)));

        REQUIRE(lhs == lhs_old);
        REQUIRE(rhs == rhs_old);
        // @todo Проверить, что обменяны распределитель памяти и функция сравнения
    };
}

static_assert(std::is_same<typename saga::flat_set<int>::reverse_iterator
                          ,saga::reverse_iterator<typename saga::flat_set<int>::iterator>>{});
static_assert(std::is_same<typename saga::flat_set<int>::const_reverse_iterator
                          ,saga::reverse_iterator<typename saga::flat_set<int>::const_iterator>>{});

// Создание, копирование, уничтожение
// @todo Задать тип контейнера?
// @todo Проверить, что нет лишних копирований?
TEST_CASE("flat_set: container constructor")
{
    using Element = long;

    struct mod_7_t
    {
        auto operator()(Element const & num) const { return num % 7; }
    };

    auto cmp = saga::compare_by(mod_7_t{}, std::greater<>{});
    using Compare = decltype(cmp);

    using FlatSet = saga::flat_set<Element, Compare>;
    using Container = FlatSet::container_type;

    static_assert(std::is_constructible<FlatSet, Container>{});
    static_assert(!std::is_convertible<Container, FlatSet>{});

    saga_test::property_checker << [](Container const & src)
    {
        FlatSet const actual(src);
        std::set<Element, Compare> const expected(src.begin(), src.end());

        CAPTURE(actual, expected);

        auto const equiv = [cmp = Compare()](Element const & lhs, Element const & rhs)
        {
            return !cmp(lhs, rhs) && !cmp(rhs, lhs);
        };

        REQUIRE(std::equal(actual.begin(), actual.end(), expected.begin(), expected.end(), equiv));
        REQUIRE(actual.empty() == expected.empty());

        // @todo Проверить распределитель памяти и функция сравнения
    };
}

static_assert(std::is_same<typename saga::flat_set<char>::key_type, char>{});

static_assert(std::is_same<typename saga::flat_set<int>::key_compare, std::less<int>>{});
static_assert(std::is_same<typename saga::flat_set<int, std::greater<>>::key_compare
                          ,std::greater<>>{});

static_assert(std::is_same<typename saga::flat_set<std::string>::container_type
                          ,std::vector<std::string>>{});
static_assert(std::is_same<typename saga::flat_set<int, std::greater<>
                                                  ,std::deque<int>>::container_type
                          ,std::deque<int>>{});

static_assert(std::is_constructible<saga::flat_set<int, saga_test::strict_weak_order<int>>
                                   ,saga_test::strict_weak_order<int>>{});
static_assert(!std::is_convertible<saga_test::strict_weak_order<int>
                                  ,saga::flat_set<int, saga_test::strict_weak_order<int>>>{});
