/* (c) 2020-2022 Галушин Павел Викторович, galushin@gmail.com

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
#include <saga/cursor/indices.hpp>

// Тестовая инфраструктура
#include <catch/catch.hpp>
#include "../saga_test.hpp"

// Вспомогательные файлы
#include <saga/algorithm.hpp>
#include <saga/cursor/subrange.hpp>
#include <saga/iterator.hpp>
#include <saga/type_traits.hpp>

#include <forward_list>
#include <list>

// Тесты
static_assert(sizeof(saga::iota_iterator<int>) == sizeof(int), "");

static_assert(std::is_same<saga::iota_iterator<std::forward_list<int>::iterator>::iterator_category,
                           std::forward_iterator_tag>{}, "");
static_assert(std::is_same<saga::iota_iterator<std::list<int>::iterator>::iterator_category,
                           std::bidirectional_iterator_tag>{}, "");
static_assert(std::is_same<saga::iota_iterator<std::vector<int>::iterator>::iterator_category,
                           std::random_access_iterator_tag>{}, "");
static_assert(std::is_same<saga::iota_iterator<int *>::iterator_category,
                           std::random_access_iterator_tag>{}, "");
static_assert(std::is_same<saga::iota_iterator<int const *>::iterator_category,
                           std::random_access_iterator_tag>{}, "");
static_assert(std::is_same<saga::iota_iterator<int>::iterator_category,
                           std::random_access_iterator_tag>{}, "");

namespace
{
    template <class T, class SFINAE = void>
    struct has_type_typedef
     : std::false_type
    {};

    template <class T>
    struct has_type_typedef<T, std::void_t<typename T::type>>
     : std::true_type
    {};
}

static_assert(!::has_type_typedef<saga::detail::iota_iterator_category<bool>>{},
              "operator++ is deprecated for bool");
static_assert(!::has_type_typedef<saga::detail::iota_iterator_category<double>>{},
              "operator++ may be not exact");

TEST_CASE("iota_iterator : distrance plus iterator")
{
    using Iterator = saga::iota_iterator<int>;
    constexpr int const value = 42;
    constexpr Iterator iter(value);
    constexpr Iterator::difference_type num{13};

    static_assert(num + iter == Iterator(value + num), "must be equal");
    static_assert(num + iter == iter + num, "must be equal");
}

namespace
{
    template <class T>
    constexpr T constexpr_postfix_increment_and_decrement(T x)
    {
        x++;
        x--;
        return x;
    }
}

TEST_CASE("iota_iterator : postfix ++ and --")
{
    {
        constexpr int const value = 42;
        constexpr auto const iter = saga::iota_iterator<int>(value);
        constexpr auto const result = ::constexpr_postfix_increment_and_decrement(iter);

        static_assert(result == iter, "");
    }

    using Value = int;
    saga_test::property_checker << [](Value value)
    {
        using Iterator = saga::iota_iterator<Value>;
        Iterator iter(value);

        if(value != std::numeric_limits<Value>::max())
        {
            auto const old_iter = iter;

            auto const result = iter++;
            REQUIRE(result == old_iter);

            REQUIRE(iter == std::next(old_iter));
        }

        if(value != std::numeric_limits<Value>::max())
        {
            auto const old_iter = iter;

            auto const result = iter--;
            REQUIRE(result == old_iter);

            REQUIRE(iter == std::prev(old_iter));
        }
    };
}

namespace
{
    using Value = int;

    static constexpr auto value = Value(17);

    static constexpr auto iter = saga::iota_iterator<Value>(value);

    static_assert(iter.operator->() == std::addressof(*iter), "");
}

TEST_CASE("iota_iterator : operator->")
{
    using Value = int;
    saga_test::property_checker << [](Value value)
    {
        using Iterator = saga::iota_iterator<Value>;
        Iterator iter(value);

        REQUIRE(iter.operator->() == std::addressof(*iter));
    };
}

TEST_CASE("range-for with indices")
{
    using Container = std::vector<int>;

    saga_test::property_checker
    << [](Container const & xs)
    {
        using Index = Container::difference_type;

        std::vector<Index> ins_obj(xs.size());
        std::iota(ins_obj.begin(), ins_obj.end(), Index(0));

        std::vector<Index> ins;

        for(auto const & each : saga::cursor::indices_of(xs))
        {
            ins.push_back(each);
        }

        CHECK(ins == ins_obj);
    };
}

TEST_CASE("indices_of for arrays")
{
    int xs [] = {1, 3, 7, 15};

    using Index = std::size_t;

    std::vector<Index> ins_obj(saga::size(xs));
    std::iota(ins_obj.begin(), ins_obj.end(), Index(0));

    auto indices_view = saga::cursor::indices_of(xs);

    std::vector<Index> ins;
    saga::copy(saga::cursor::all(indices_view), saga::back_inserter(ins));

    CHECK(ins == ins_obj);
}

static_assert(!(saga::unreachable_sentinel_t{} == 42), "");
static_assert(!(42 == saga::unreachable_sentinel_t{}), "");
