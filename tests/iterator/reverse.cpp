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
#include <saga/iterator/reverse.hpp>

// Тестовая инфраструктура
#include "../saga_test.hpp"
#include <catch/catch.hpp>

// Вспомогательные файлы
#include <saga/view/indices.hpp>

#include <list>
#include <deque>
#include <type_traits>
#include <vector>

// Тесты
namespace
{
    template <class Iterator>
    class reverse_iterator_typedefs_checker
    {
        using Traits = std::iterator_traits<Iterator>;

        using RIterator = saga::reverse_iterator<Iterator>;

        static_assert(std::is_same<typename RIterator::iterator_type, Iterator>{}, "");
        static_assert(std::is_same<typename RIterator::iterator_category,
                                   typename Traits::iterator_category>{}, "");
        static_assert(std::is_same<typename RIterator::value_type,
                                   typename Traits::value_type>{}, "");
        static_assert(std::is_same<typename RIterator::difference_type,
                                   typename Traits::difference_type>{}, "");
        static_assert(std::is_same<typename RIterator::pointer,
                                   typename Traits::pointer>{}, "");
        static_assert(std::is_same<typename RIterator::reference,
                                   typename Traits::reference>{}, "");
    };

    // @todo Как автоматически проверить для списка типов?
    reverse_iterator_typedefs_checker<std::list<int>::iterator> checker1;
    reverse_iterator_typedefs_checker<std::list<int>::const_iterator> checker1_c;
    reverse_iterator_typedefs_checker<std::deque<int>::iterator> checker2;
    reverse_iterator_typedefs_checker<std::deque<int>::const_iterator> checker2_c;
    reverse_iterator_typedefs_checker<std::vector<int>::iterator> checker3;
    reverse_iterator_typedefs_checker<std::vector<int>::const_iterator> checker3_c;

    template <class Iterator>
    void check_reverse_iterator_ctor_for_iterator(Iterator iter)
    {
        std::reverse_iterator<Iterator> r_iter(iter);

        REQUIRE(r_iter.base() == iter);
    }

    template <class Container>
    void check_reverse_iterator_ctor_for_container(Container container)
    {
        auto const pos = saga_test::random_uniform(0*container.size(), container.size());

        ::check_reverse_iterator_ctor_for_iterator(std::next(container.begin(), pos));
        ::check_reverse_iterator_ctor_for_iterator(std::next(container.cbegin(), pos));
    }

    template <class Iterator>
    void check_make_reverse_iterator_for_iterator(Iterator iter)
    {
        auto const r_iter = saga::make_reverse_iterator(iter);

        static_assert(std::is_same<decltype(saga::make_reverse_iterator(iter)),
                                    saga::reverse_iterator<Iterator>>{}, "");

        REQUIRE(r_iter.base() == iter);
    }

    template <class Container>
    void check_make_reverse_iterator_for_container(Container container)
    {
        auto const pos = saga_test::random_uniform(0*container.size(), container.size());

        ::check_make_reverse_iterator_for_iterator(std::next(container.begin(), pos));
        ::check_make_reverse_iterator_for_iterator(std::next(container.cbegin(), pos));
    }

    template <class Container>
    void check_reverse_iterator_equality_for_container(Container container)
    {
        auto const pos1 = saga_test::random_uniform(0*container.size(), container.size());
        auto const pos2 = saga_test::random_uniform(0*container.size(), container.size());

        saga::reverse_iterator<typename Container::iterator> const it0{};
        auto const it1 = saga::make_reverse_iterator(std::next(container.begin(), pos1));
        auto const it1_c = it1;
        auto const it2 = saga::make_reverse_iterator(std::next(container.cbegin(), pos2));

        static_assert(!std::is_same<decltype(it1), decltype(it2)>{}, "");

        REQUIRE(it0 == it0);
        REQUIRE(it1 == it1);
        REQUIRE(it1 == it1_c);
        REQUIRE(it1_c == it1);
        REQUIRE(it2 == it2);

        if(!container.empty())
        {
            REQUIRE(it1 != it0);
            REQUIRE(it0 != it1);
            REQUIRE(it2 != it0);
            REQUIRE(it0 != it2);
        }

        REQUIRE((it1 == it2) == (it1.base() == it2.base()));
        REQUIRE((it1 != it2) == (it1.base() != it2.base()));
    }

    template <class Container>
    void check_compatible_ctor_for_container(Container container)
    {
        auto const pos = saga_test::random_uniform(0*container.size(), container.size());

        using RIterator= saga::reverse_iterator<typename Container::iterator>;
        RIterator const r_iter = saga::make_reverse_iterator(std::next(container.begin(), pos));

        using CRIterator= saga::reverse_iterator<typename Container::const_iterator>;
        CRIterator const cr_iter(r_iter);

        static_assert(!std::is_same<RIterator, CRIterator>{}, "Must be different");

        static_assert(std::is_constructible<CRIterator, RIterator>{},"");
        static_assert(!std::is_constructible<RIterator, CRIterator>{},"");
        static_assert(std::is_convertible<RIterator, CRIterator>{},"");
        static_assert(!std::is_convertible<CRIterator, RIterator>{},"");


        REQUIRE(cr_iter == r_iter);
    }

    template <class Container>
    void check_compatible_assign_for_container(Container container)
    {
        auto const pos1 = saga_test::random_uniform(0*container.size(), container.size());
        auto const pos2 = saga_test::random_uniform(0*container.size(), container.size());

        using RIterator= saga::reverse_iterator<typename Container::iterator>;
        using CRIterator= saga::reverse_iterator<typename Container::const_iterator>;
        static_assert(!std::is_same<RIterator, CRIterator>{}, "Must be different");

        static_assert(std::is_assignable<CRIterator, RIterator>{},"");
        static_assert(!std::is_assignable<RIterator, CRIterator>{},"");

        auto const r_iter = saga::make_reverse_iterator(std::next(container.begin(), pos1));
        auto cr_iter = saga::make_reverse_iterator(std::next(container.begin(), pos2));
        CRIterator cr_iter_def{};

        cr_iter = r_iter;
        cr_iter_def = r_iter;

        REQUIRE(cr_iter == r_iter);
        REQUIRE(cr_iter_def == r_iter);
    }
}

TEST_CASE("reverse_iterator : default ctor")
{
    using Iterator = int*;

    constexpr saga::reverse_iterator<Iterator> const r_iter{};

    constexpr Iterator const iter{};

    static_assert(r_iter.base() == iter, "");
}

TEST_CASE("reverse_iterator : ctor with value")
{
    using Iterator = int const*;

    static constexpr int arr[] = {3, 14, 15};
    constexpr Iterator const iter(arr+1);

    using RIterator = saga::reverse_iterator<Iterator>;
    constexpr const RIterator r_iter(iter);

    static_assert(r_iter.base() == iter, "");

    static_assert(std::is_constructible<RIterator, Iterator>{}, "");
    static_assert(!std::is_convertible<Iterator, RIterator>{}, "Ctor must be explicit!");

    saga_test::property_checker
        << ::check_reverse_iterator_ctor_for_container<std::list<int>>
        << ::check_reverse_iterator_ctor_for_container<std::deque<int>>
        << ::check_reverse_iterator_ctor_for_container<std::vector<int>>;
}

TEST_CASE("reverse_iterator equality")
{
    {
        constexpr saga::detail::iota_iterator<int> it1(13);
        constexpr saga::detail::iota_iterator<long> it1_a(13);
        constexpr saga::detail::iota_iterator<long> it2(42);

        static_assert(it1 == it1_a, "");
        static_assert(it1 != it2, "");

        constexpr auto const r_it1 = saga::make_reverse_iterator(it1);
        constexpr auto const r_it1_a = saga::make_reverse_iterator(it1_a);
        constexpr auto const r_it2 = saga::make_reverse_iterator(it2);

        static_assert(r_it1 == r_it1, "");
        static_assert(r_it1 == r_it1_a, "");
        static_assert(r_it1 != r_it2, "");
    }

    saga_test::property_checker
        << ::check_reverse_iterator_equality_for_container<std::list<int>>
        << ::check_reverse_iterator_equality_for_container<std::deque<int>>
        << ::check_reverse_iterator_equality_for_container<std::vector<int>>;
}

TEST_CASE("make_reverse_iterator")
{
    {
        using Iterator = saga::detail::iota_iterator<int>;

        constexpr Iterator const iter(42);

        using RIterator = saga::reverse_iterator<Iterator>;
        static_assert(std::is_same<decltype(saga::make_reverse_iterator(iter)), RIterator>{}, "");

        constexpr auto const r_iter = saga::make_reverse_iterator(iter);
        static_assert(r_iter.base() == iter, "");
    }

    saga_test::property_checker
        << ::check_make_reverse_iterator_for_container<std::list<int>>
        << ::check_make_reverse_iterator_for_container<std::deque<int>>
        << ::check_make_reverse_iterator_for_container<std::vector<int>>;
}

TEST_CASE("reverse_iterator : compatible reverse_iterator ctor")
{
    {
        constexpr saga::detail::iota_iterator<int> const it1(42);

        constexpr auto const r_it1 = saga::make_reverse_iterator(it1);
        constexpr saga::reverse_iterator<saga::detail::iota_iterator<long>> r_it2(r_it1);

        static_assert(!std::is_same<decltype(r_it1), decltype(r_it2)>{}, "");

        static_assert(r_it2 == r_it1, "");
    }

    saga_test::property_checker
        << ::check_compatible_ctor_for_container<std::list<int>>
        << ::check_compatible_ctor_for_container<std::deque<int>>
        << ::check_compatible_ctor_for_container<std::vector<int>>;
}

namespace
{
    template <class T, class U>
    constexpr T test_constexpr_assign(T lhs, U const & rhs)
    {
        lhs = rhs;
        return lhs;
    }
}

TEST_CASE("reverse_iterator : compatible reverse_iterator assign")
{
    {
        constexpr saga::detail::iota_iterator<int> const it1(13);
        constexpr saga::detail::iota_iterator<long> const it2(42);

        static_assert(!std::is_same<decltype(it1), decltype(it2)>{}, "");

        constexpr auto r_it1 = saga::make_reverse_iterator(it1);
        constexpr auto r_it2 = saga::make_reverse_iterator(it2);

        static_assert(!std::is_same<decltype(r_it1), decltype(r_it2)>{}, "");

        static_assert(r_it2 != r_it1, "");

        constexpr auto r_it2_a = test_constexpr_assign(r_it2, r_it1);

        static_assert(r_it2_a == r_it1, "");
    }

    saga_test::property_checker
        << ::check_compatible_assign_for_container<std::list<int>>
        << ::check_compatible_assign_for_container<std::deque<int>>
        << ::check_compatible_assign_for_container<std::vector<int>>;
}
