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
#include <saga/iterator.hpp>

// Тестовая инфраструктура
#include "saga_test.hpp"
#include <catch/catch.hpp>

// Вспомогательные файлы
#include <saga/view/indices.hpp>

#include <list>

//Тесты

// incrementable_traits
namespace
{
    template <class T>
    using has_difference_type = saga::detail::has_difference_type<T>;

    struct without_difference_type
    {};

    static_assert(!::has_difference_type<int>{}, "");
    static_assert(::has_difference_type<std::vector<int>::iterator>{}, "");

    struct strange_diff
    {
        using difference_type = int;

        long operator-(strange_diff const &) const;
    };
}

static_assert(!::has_difference_type<saga::incrementable_traits<::without_difference_type>>{},"");

static_assert(std::is_same<saga::incrementable_traits<int*>::difference_type, std::ptrdiff_t>{},"");
static_assert(!::has_difference_type<saga::incrementable_traits<void(*)()>>{},"");
static_assert(!::has_difference_type<saga::incrementable_traits<void *>>{},"");
static_assert(!::has_difference_type<saga::incrementable_traits<void const *>>{},"");

static_assert(std::is_same<saga::incrementable_traits<std::vector<int>>::difference_type,
                           std::vector<int>::difference_type>{}, "");
static_assert(std::is_same<saga::incrementable_traits<int>::difference_type, int>{}, "");
static_assert(std::is_same<saga::incrementable_traits<unsigned long>::difference_type, long>{}, "");
static_assert(!::has_difference_type<saga::incrementable_traits<double>>{},"");
static_assert(std::is_same<saga::incrementable_traits<::strange_diff>::difference_type, int>{}, "");

static_assert(!::has_difference_type<saga::incrementable_traits<::without_difference_type const>>{},"");

static_assert(std::is_same<saga::incrementable_traits<int* const>::difference_type, std::ptrdiff_t>{},"");
static_assert(!::has_difference_type<saga::incrementable_traits<std::add_const_t<void(*)()>>>{},"");
static_assert(!::has_difference_type<saga::incrementable_traits<void * const>>{},"");
static_assert(!::has_difference_type<saga::incrementable_traits<void const * const>>{},"");

static_assert(std::is_same<saga::incrementable_traits<std::vector<int> const>::difference_type,
                           std::vector<int>::difference_type>{}, "");
static_assert(std::is_same<saga::incrementable_traits<int const>::difference_type, int>{}, "");
static_assert(std::is_same<saga::incrementable_traits<unsigned long const>::difference_type, long>{}, "");
static_assert(!::has_difference_type<saga::incrementable_traits<double const>>{},"");
static_assert(std::is_same<saga::incrementable_traits<::strange_diff const>::difference_type, int>{}, "");

// Итераторы вставки

namespace
{
    template <class Iterator, class Container>
    void check_emplace_iterators_typedefs()
    {
        static_assert(std::is_same<typename Iterator::iterator_category, std::output_iterator_tag>{}, "");
        static_assert(std::is_same<typename Iterator::value_type, void>{}, "");
        static_assert(std::is_same<typename Iterator::difference_type, void>{}, "");
        static_assert(std::is_same<typename Iterator::pointer, void>{}, "");
        static_assert(std::is_same<typename Iterator::reference, void>{}, "");
        static_assert(std::is_same<typename Iterator::container_type, Container>{}, "");
    }
}

TEST_CASE("back_emplacer")
{
    using Size = std::size_t;
    using Container = std::vector<int>;

    static_assert(std::is_constructible<Container, Size>{}, "");
    static_assert(!std::is_convertible<Size, Container>{}, "");

    // Конструктор по умолчанию
    saga::back_emplace_iterator<Container> it_0;
    REQUIRE(it_0.container() == nullptr);

    using Iterator = decltype(it_0);

    check_emplace_iterators_typedefs<Iterator, Container>();

    // Основной функционал
    saga_test::property_checker
    << [](std::vector<saga_test::container_size<Size>> const & nums)
    {
        std::vector<Container> result;

        auto it = std::copy(nums.begin(), nums.end(), saga::back_emplacer(result));

        REQUIRE(it.container() == std::addressof(result));

        REQUIRE(result.size() == nums.size());

        for(auto i : saga::view::indices_of(nums))
        {
            REQUIRE(result.at(i) == Container(nums.at(i)));
        }
    };
}

TEST_CASE("front_emplacer")
{
    using Size = std::size_t;
    using Container = std::vector<int>;

    static_assert(std::is_constructible<Container, Size>{}, "");
    static_assert(!std::is_convertible<Size, Container>{}, "");

    // Конструктор по умолчанию
    saga::front_emplace_iterator<Container> it_0;
    REQUIRE(it_0.container() == nullptr);

    using Iterator = decltype(it_0);

    check_emplace_iterators_typedefs<Iterator, Container>();

    // Основной функционал
    saga_test::property_checker
    << [](std::vector<saga_test::container_size<Size>> const & nums)
    {
        std::list<Container> temp;

        auto it = std::copy(nums.begin(), nums.end(), saga::front_emplacer(temp));

        REQUIRE(it.container() == std::addressof(temp));

        std::vector<Container> result;
        std::reverse_copy(temp.begin(), temp.end(), saga::back_emplacer(result));

        REQUIRE(result.size() == nums.size());

        for(auto i : saga::view::indices_of(nums))
        {
            REQUIRE(result.at(i) == Container(nums.at(i)));
        }
    };
}

TEST_CASE("emplacer")
{
    using Size = std::size_t;
    using Container = std::vector<int>;

    static_assert(std::is_constructible<Container, Size>{}, "");
    static_assert(!std::is_convertible<Size, Container>{}, "");

    // Конструктор по умолчанию
    saga::front_emplace_iterator<Container> it_0;
    REQUIRE(it_0.container() == nullptr);

    using Iterator = decltype(it_0);

    check_emplace_iterators_typedefs<Iterator, Container>();

    // Основной функционал
    saga_test::property_checker
    << [](std::vector<saga_test::container_size<Size>> const & nums)
    {
        std::vector<Container> result(10, Container{});
        auto const offset = result.size() / 2;
        auto const pos = result.begin() + offset;

        std::vector<Container> obj(result.begin(), pos);
        std::copy(nums.begin(), nums.end(), saga::back_emplacer(obj));
        std::copy(pos, result.end(), std::back_inserter(obj));

        auto it = std::copy(nums.begin(), nums.end(), saga::emplacer(result, pos));
        REQUIRE(it.container() == std::addressof(result));

        CAPTURE(nums);

        REQUIRE(result == obj);
    };
}
