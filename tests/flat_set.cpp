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
#include <set>

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

// @todo В библиотеку?
namespace saga_test
{
    template <class T>
    struct allocator_with_tag
     : std::allocator<T>
    {
        using Base = std::allocator<T>;

        using is_always_equal = std::false_type;
        using propagate_on_container_swap = std::true_type;

        template <class Other>
        struct rebind
        {
            using other = allocator_with_tag<Other>;
        };

        template <class Other>
        allocator_with_tag(allocator_with_tag<Other> const & other) noexcept
         : Base(other)
         , tag(other.tag)
        {}

        using tag_type = int;

        allocator_with_tag() = default;

        explicit allocator_with_tag(tag_type arg)
         : tag(arg)
        {}

        tag_type tag = 0;

        friend bool operator==(allocator_with_tag const & lhs, allocator_with_tag const & rhs)
        {
            return lhs.tag == rhs.tag
                    && static_cast<Base const &>(lhs) == static_cast<Base const &>(rhs);
        }

        friend bool operator!=(allocator_with_tag const & lhs, allocator_with_tag const & rhs)
        {
            return !(lhs == rhs);
        }
    };

    template <class T>
    struct arbitrary<allocator_with_tag<T>>
    {
        using value_type = allocator_with_tag<T>;

        template <class UniformRandomBitGenerator>
        static value_type generate(generation_t generation, UniformRandomBitGenerator & urbg)
        {
            using Arg = typename value_type::tag_type;

            return value_type(saga_test::arbitrary<Arg>::generate(generation, urbg));
        }
    };

    template <class AssocContainer1, class AssocContainer2>
    bool deep_equal_associative_container(AssocContainer1 const & lhs, AssocContainer2 const & rhs)
    {
        return saga::equal(saga::cursor::all(lhs), saga::cursor::all(rhs))
               && lhs.value_comp() == rhs.value_comp()
               && lhs.get_allocator() == rhs.get_allocator();
    }
}

TEST_CASE("allocator_with_tag")
{
    using Element = std::string;
    using Allocator = saga_test::allocator_with_tag<Element>;

    saga_test::property_checker << [](Allocator::tag_type tag_lhs, Allocator::tag_type tag_rhs)
    {
        REQUIRE((Allocator(tag_lhs) == Allocator(tag_rhs)) == (tag_lhs == tag_rhs));
        REQUIRE((Allocator(tag_lhs) != Allocator(tag_rhs)) == (tag_lhs != tag_rhs));
    };
}

// Тесты
// Базовые тесты, на которые мы полагаемся позже
TEST_CASE("flat_set: constructor from elements, compare and allocator")
{
    using Element = int;
    using Compare = saga_test::strict_weak_order<Element>;
    using Allocator = saga_test::allocator_with_tag<Element>;
    using Container = std::deque<Element, Allocator>;
    using FlatSet = saga::flat_set<Element, Compare, Container>;

    saga_test::property_checker
    << [](std::vector<Element> const & src, Compare const & cmp, Allocator const & alloc)
    {
        static_assert(!std::allocator_traits<Allocator>::is_always_equal{});

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

// @todo Возможно, перенести в тест saga_test
TEST_CASE("flat_set: deep equality - for test")
{
    using Element = unsigned;
    using Compare = saga_test::strict_weak_order<Element>;

    using Allocator = saga_test::allocator_with_tag<Element>;
    static_assert(!std::allocator_traits<Allocator>::is_always_equal{});

    using Container = std::vector<Element, Allocator>;
    using FlatSet = saga::flat_set<Element, Compare, Container>;

    saga_test::property_checker
    << [](Container const & elems_lhs, Compare const & cmp_lhs, Allocator const & alloc_lhs
         ,Container const & elems_rhs, Compare const & cmp_rhs, Allocator const & alloc_rhs)
    {
        FlatSet const lhs(elems_lhs.begin(), elems_lhs.end(), cmp_lhs, alloc_lhs);
        FlatSet const rhs(elems_rhs.begin(), elems_rhs.end(), cmp_rhs, alloc_rhs);

        REQUIRE(saga_test::deep_equal_associative_container(lhs, lhs));
        REQUIRE(saga_test::deep_equal_associative_container(rhs, rhs));
        REQUIRE(saga_test::deep_equal_associative_container(lhs, rhs)
                == (lhs == rhs
                    && lhs.value_comp() == rhs.value_comp()
                    && lhs.get_allocator() == rhs.get_allocator()));
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
// @todo Проверки типов итераторов при явном задании контейнера с нестандартными типами
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

// @todo Проверки size_type и difference_type при явном задании контейнера с нестандартными типами
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

// @todo Настройка функции сравнения и распределителя памяти
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

// @todo Настройка функции сравнения и распределителя памяти
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

// @todo Настройка функции сравнения и распределителя памяти
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

// @todo Настройка функции сравнения и распределителя памяти
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

/* Деструктор не проверяем: так как flat_set хранит элементы во вложенном контейнере,
достаточно убедиться, что flat_set вообще не объявляет деструктор?
*/
// @todo проверить через regular_tracer что все элементы уничтожены?

TEST_CASE("flat_set: begin, end, cbegin, cend")
{
    using Element = long;
    using Compare = saga_test::strict_weak_order<Element>;

    using Allocator = saga_test::allocator_with_tag<Element>;
    static_assert(!std::allocator_traits<Allocator>::is_always_equal{});

    using Container = std::vector<Element, Allocator>;
    using FlatSet = saga::flat_set<Element, Compare, Container>;

    saga_test::property_checker
    << [](Container const & elems, Compare const & cmp, Allocator const & alloc)
    {
        std::set<Element, Compare> const expected(elems.begin(), elems.end(), cmp);
        FlatSet actual(elems.begin(), elems.end(), cmp, alloc);

        static_assert(std::is_same<decltype(actual.begin()), FlatSet::iterator>{});
        static_assert(std::is_same<decltype(actual.end()), FlatSet::iterator>{});

        REQUIRE(std::equal(actual.begin(), actual.end()
                          ,expected.begin(), expected.end()));

        REQUIRE(actual.cbegin() == std::as_const(actual).begin());
        REQUIRE(actual.cend() == std::as_const(actual).end());
    };
}

TEST_CASE("flat_set: begin, end, cbegin, cend - constant")
{
    using Element = long;
    using Compare = saga_test::strict_weak_order<Element>;

    using Allocator = saga_test::allocator_with_tag<Element>;
    static_assert(!std::allocator_traits<Allocator>::is_always_equal{});

    using Container = std::vector<Element, Allocator>;

    using FlatSet = saga::flat_set<Element, Compare, Container>;

    saga_test::property_checker
    << [](Container const & elems, Compare const & cmp, Allocator const & alloc)
    {
        std::set<Element, Compare> const expected(elems.begin(), elems.end(), cmp);
        FlatSet const actual(elems.begin(), elems.end(), cmp, alloc);

        static_assert(std::is_same<decltype(actual.begin()), FlatSet::iterator>{});
        static_assert(std::is_same<decltype(actual.end()), FlatSet::iterator>{});

        REQUIRE(std::equal(actual.begin(), actual.end()
                          ,expected.begin(), expected.end()));

        REQUIRE(actual.cbegin() == std::as_const(actual).begin());
        REQUIRE(actual.cend() == std::as_const(actual).end());
    };
}

// @todo operator<=>: пока определяем операторы, доступные в C++17
TEST_CASE("flat_set: comparisons")
{
    using Element = long;
    using Compare = saga_test::strict_weak_order<Element>;

    using Allocator = saga_test::allocator_with_tag<Element>;
    static_assert(!std::allocator_traits<Allocator>::is_always_equal{});

    using Container = std::vector<Element, Allocator>;
    using FlatSet = saga::flat_set<Element, Compare, Container>;

    saga_test::property_checker
    << [](Container const & elems_lhs, Compare const & cmp_lhs, Allocator const & alloc_lhs
         ,Container const & elems_rhs, Compare const & cmp_rhs, Allocator const & alloc_rhs)
    {
        FlatSet const lhs(elems_lhs.begin(), elems_lhs.end(), cmp_lhs, alloc_lhs);
        FlatSet const rhs(elems_rhs.begin(), elems_rhs.end(), cmp_rhs, alloc_rhs);

        CAPTURE(lhs, rhs);
        REQUIRE((lhs < rhs) == std::lexicographical_compare(lhs.begin(), lhs.end()
                                                           ,rhs.begin(), rhs.end()));
        REQUIRE((lhs > rhs) == (rhs < lhs));
        REQUIRE((lhs <= rhs) == !(lhs > rhs));
        REQUIRE((lhs >= rhs) == !(lhs < rhs));
    };
}

// Равенство проверено ранее

// swap
namespace
{
    template <class T>
    void swap(T & lhs, T & rhs) = delete;
}

TEST_CASE("flat_set : swap")
{
    using Element = long;
    using Compare = saga_test::strict_weak_order<Element>;

    using Allocator = saga_test::allocator_with_tag<Element>;
    static_assert(!std::allocator_traits<Allocator>::is_always_equal{});
    // @todo тест, когда это не выполняется
    static_assert(std::allocator_traits<Allocator>::propagate_on_container_swap{});

    using Container = std::vector<Element, Allocator>;
    using FlatSet = saga::flat_set<Element, Compare, Container>;

    saga_test::property_checker
    << [](Container const & elems_lhs, Compare const & cmp_lhs, Allocator const & alloc_lhs
         ,Container const & elems_rhs, Compare const & cmp_rhs, Allocator const & alloc_rhs)
    {
        FlatSet const lhs_old(elems_lhs.begin(), elems_lhs.end(), cmp_lhs, alloc_lhs);
        FlatSet const rhs_old(elems_rhs.begin(), elems_rhs.end(), cmp_rhs, alloc_rhs);

        auto lhs = lhs_old;
        auto rhs = rhs_old;

        // @todo Константная сложность
        lhs.swap(rhs);
        static_assert(noexcept(lhs.swap(rhs)));

        REQUIRE(saga_test::deep_equal_associative_container(lhs, rhs_old));
        REQUIRE(saga_test::deep_equal_associative_container(rhs, lhs_old));

        // @todo Константная сложность
        swap(lhs, rhs);
        static_assert(noexcept(swap(lhs, rhs)));

        REQUIRE(saga_test::deep_equal_associative_container(lhs, lhs_old));
        REQUIRE(saga_test::deep_equal_associative_container(rhs, rhs_old));
    };
}

// @todo size, max_size, empty - константная сложность
TEST_CASE("flat_set: size, empty")
{
    using Element = long;
    using Compare = saga_test::strict_weak_order<Element>;

    using Allocator = saga_test::allocator_with_tag<Element>;
    static_assert(!std::allocator_traits<Allocator>::is_always_equal{});

    using Container = std::vector<Element, Allocator>;

    using FlatSet = saga::flat_set<Element, Compare, Container>;

    saga_test::property_checker
    << [](Container const & elems, Compare const & cmp, Allocator const & alloc)
    {
        FlatSet const obj(elems.begin(), elems.end(), cmp, alloc);

        REQUIRE(obj.size()
                == static_cast<FlatSet::size_type>(std::distance(obj.begin(), obj.end())));

        REQUIRE(obj.size() <= obj.max_size());
        REQUIRE(obj.max_size() == elems.max_size());

        REQUIRE(obj.empty() == (obj.begin() == obj.end()));

        static_assert(std::is_same<decltype(obj.size()), typename FlatSet::size_type>{});
        static_assert(std::is_same<decltype(obj.max_size()), typename FlatSet::size_type>{});
        static_assert(std::is_same<decltype(obj.empty()), bool>{});

        FlatSet const obj0(elems.end(), elems.end(), cmp, alloc);

        REQUIRE(obj0.size() == 0);
        REQUIRE(obj0.empty());
    };
}

// @todo limited_allocator в библиотеку
namespace saga_test
{
    template <class T, std::ptrdiff_t Max_size>
    class limited_allocator
     : public std::allocator<T>
    {
    public:
        using value_type = T;
        using pointer = value_type *;
        using size_type = std::size_t;

        template <class Other>
        struct rebind
        {
            using other = limited_allocator<Other, Max_size>;
        };

        size_type max_size() const
        {
            return Max_size;
        }

        // @todo Настоящая реализация выделения и освобождения памяти ограниченного размера
    };
}

TEST_CASE("flat_set: max_size with limited allocator")
{
    using Element = long;
    using Compare = std::greater<>;

    constexpr std::size_t allocator_max_size = 42;
    using Allocator = saga_test::limited_allocator<Element, allocator_max_size>;

    using Container = std::vector<Element, Allocator>;

    saga::flat_set<Element, Compare, Container> const obj{};

    REQUIRE(obj.max_size() == allocator_max_size);
}

// @todo Требования пунктов 65-69

// 24.2.2.3 Reversible container requirements
// @todo Обыные итераторы должны быть двунаправленными или произвольного доступа

// Используем saga::reverse_iterator вместо std::reverse_iterator
// Так как itertor bи const_iterator совпадают, то обратные их версии совпадают
// @todo Задание типа распределителя памяти и функции сравнения
static_assert(std::is_same<typename saga::flat_set<int>::reverse_iterator
                          ,saga::reverse_iterator<typename saga::flat_set<int>::iterator>>{});
static_assert(std::is_same<typename saga::flat_set<int>::const_reverse_iterator
                          ,typename saga::flat_set<int>::reverse_iterator>{});

TEST_CASE("flat_set: rbegin, rend, crbegin, crend - for non-constant")
{
    using Element = long;
    using Compare = saga_test::strict_weak_order<Element>;

    using Allocator = saga_test::allocator_with_tag<Element>;
    static_assert(!std::allocator_traits<Allocator>::is_always_equal{});

    using Container = std::vector<Element, Allocator>;
    using FlatSet = saga::flat_set<Element, Compare, Container>;

    saga_test::property_checker
    << [](Container const & elems, Compare const & cmp, Allocator const & alloc)
    {
        std::set<Element, Compare> const expected(elems.begin(), elems.end(), cmp);
        FlatSet actual(elems.begin(), elems.end(), cmp, alloc);

        static_assert(std::is_same<decltype(actual.rbegin()), FlatSet::reverse_iterator>{});
        static_assert(std::is_same<decltype(actual.rend()), FlatSet::reverse_iterator>{});

        REQUIRE(std::equal(actual.rbegin(), actual.rend()
                          ,expected.rbegin(), expected.rend()));

        REQUIRE(actual.crbegin() == std::as_const(actual).rbegin());
        REQUIRE(actual.crend() == std::as_const(actual).rend());
    };
}

TEST_CASE("flat_set: begin, end, cbegin, cend - for constant")
{
    using Element = long;
    using Compare = saga_test::strict_weak_order<Element>;

    using Allocator = saga_test::allocator_with_tag<Element>;
    static_assert(!std::allocator_traits<Allocator>::is_always_equal{});

    using Container = std::vector<Element, Allocator>;

    using FlatSet = saga::flat_set<Element, Compare, Container>;

    saga_test::property_checker
    << [](Container const & elems, Compare const & cmp, Allocator const & alloc)
    {
        std::set<Element, Compare> const expected(elems.begin(), elems.end(), cmp);
        FlatSet const actual(elems.begin(), elems.end(), cmp, alloc);

        static_assert(std::is_same<decltype(actual.rbegin()), FlatSet::reverse_iterator>{});
        static_assert(std::is_same<decltype(actual.rend()), FlatSet::reverse_iterator>{});

        REQUIRE(std::equal(actual.rbegin(), actual.rend()
                          ,expected.rbegin(), expected.rend()));

        REQUIRE(actual.crbegin() == std::as_const(actual).rbegin());
        REQUIRE(actual.crend() == std::as_const(actual).rend());
    };
}

// Создание, копирование, уничтожение
// @todo Задать тип контейнера?
// @todo Проверить, что нет лишних копирований?
TEST_CASE("flat_set: container constructor")
{
    using Element = long;

    using Compare = std::greater<>;
    using FlatSet = saga::flat_set<Element, Compare>;
    using Container = FlatSet::container_type;

    static_assert(std::is_constructible<FlatSet, Container>{});
    static_assert(!std::is_convertible<Container, FlatSet>{});

    saga_test::property_checker
    << [](Container const & src)
    {
        FlatSet const actual(src);
        std::set<Element, Compare> const expected(src.begin(), src.end());

        CAPTURE(actual, expected);
        REQUIRE(::is_equivalent_by_compare(actual, expected, Compare()));
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
