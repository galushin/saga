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
    void check_reverse_iterator_typedefs()
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
    }

    template <class Iterator>
    void check_reverse_iterator_ctor_for_iterator(Iterator iter)
    {
        std::reverse_iterator<Iterator> r_iter(iter);

        REQUIRE(r_iter.base() == iter);
    }

    template <class Container>
    void check_reverse_iterator_ctor_for_container(Container container)
    {
        ::check_reverse_iterator_ctor_for_iterator(saga_test::random_iterator_of(container));
        ::check_reverse_iterator_ctor_for_iterator(saga_test::random_const_iterator_of(container));
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
        ::check_make_reverse_iterator_for_iterator(saga_test::random_iterator_of(container));
        ::check_make_reverse_iterator_for_iterator(saga_test::random_const_iterator_of(container));
    }

    template <class Container>
    void check_make_reverse_iterator_is_inverse_of_itself(Container const & container)
    {
        auto const iter = saga_test::random_const_iterator_of(container);

        auto const r_iter = saga::make_reverse_iterator(iter);
        auto const rr_iter = saga::make_reverse_iterator(r_iter);

        static_assert(std::is_same<decltype(rr_iter), decltype(iter)>{}, "");

        REQUIRE(saga::make_reverse_iterator(r_iter) == iter);
    }

    template <class Container>
    void check_reverse_iterator_equality_for_container(Container container)
    {
        saga::reverse_iterator<typename Container::iterator> const it0{};
        auto const it1 = saga::make_reverse_iterator(saga_test::random_iterator_of(container));
        auto const it1_c = it1;
        auto const it2 = saga::make_reverse_iterator(saga_test::random_const_iterator_of(container));

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
        using RIterator = saga::reverse_iterator<typename Container::iterator>;
        RIterator const r_iter
            = saga::make_reverse_iterator(saga_test::random_iterator_of(container));

        using CRIterator = saga::reverse_iterator<typename Container::const_iterator>;
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
        using RIterator = saga::reverse_iterator<typename Container::iterator>;
        using CRIterator = saga::reverse_iterator<typename Container::const_iterator>;
        static_assert(!std::is_same<RIterator, CRIterator>{}, "Must be different");

        static_assert(std::is_assignable<CRIterator, RIterator>{},"");
        static_assert(!std::is_assignable<RIterator, CRIterator>{},"");

        auto const r_iter = saga::make_reverse_iterator(saga_test::random_iterator_of(container));
        auto cr_iter = saga::make_reverse_iterator(saga_test::random_const_iterator_of(container));
        CRIterator cr_iter_def{};

        cr_iter = r_iter;
        cr_iter_def = r_iter;

        REQUIRE(cr_iter == r_iter);
        REQUIRE(cr_iter_def == r_iter);
    }

    template <class Container>
    void check_reverse_iterator_comparison_for_container(Container container)
    {
        auto const it1 = saga_test::random_iterator_of(container);
        auto const it2 = saga_test::random_const_iterator_of(container);

        static_assert(!std::is_same<decltype(it1), decltype(it2)>{}, "");

        auto const r_it1 = saga::make_reverse_iterator(it1);
        auto const r_it2 = saga::make_reverse_iterator(it2);

        static_assert(!std::is_same<decltype(r_it1), decltype(r_it2)>{}, "");

        REQUIRE((r_it1 < r_it2) == (it1 > it2));
        REQUIRE((r_it1 > r_it2) == (it1 < it2));
        REQUIRE((r_it1 <= r_it2) == (it1 >= it2));
        REQUIRE((r_it1 >= r_it2) == (it1 <= it2));

        REQUIRE(!(r_it1 < r_it1));
        REQUIRE(!(r_it2 < r_it2));
        REQUIRE(!(r_it1 > r_it1));
        REQUIRE(!(r_it2 > r_it2));

        REQUIRE(r_it1 <= r_it1);
        REQUIRE(r_it2 <= r_it2);
        REQUIRE(r_it1 >= r_it1);
        REQUIRE(r_it2 >= r_it2);
    }

    template <class Container>
    void check_reverse_iterator_dereference_for_container(Container container)
    {
        auto const iter = saga_test::random_iterator_of(container);

        auto const r_iter = saga::make_reverse_iterator(iter);

        static_assert(std::is_same<decltype(*iter), decltype(*r_iter)>{}, "");

        if(iter != container.begin())
        {
            auto pi = std::prev(iter);
            REQUIRE(std::addressof(*r_iter) == std::addressof(*pi));
            REQUIRE(r_iter.operator->() == std::addressof(*r_iter));
        }
    }

    template <class Container>
    void check_reverse_iterator_increment_and_decrement_for_container(Container container)
    {
        auto iter = saga_test::random_iterator_of(container);
        auto r_iter = saga::make_reverse_iterator(iter);

        static_assert(std::is_same<decltype(++r_iter), decltype(r_iter) &>{}, "");
        static_assert(std::is_same<decltype(--r_iter), decltype(r_iter) &>{}, "");
        static_assert(std::is_same<decltype(r_iter++), decltype(r_iter)>{}, "");
        static_assert(std::is_same<decltype(r_iter--), decltype(r_iter)>{}, "");

        if(iter != container.begin())
        {
            auto & result = ++ r_iter;
            -- iter;
            REQUIRE(r_iter.base() == iter);
            REQUIRE(std::addressof(result) == std::addressof(r_iter));
        }

        if(iter != container.end())
        {
            auto const & result = -- r_iter;
            ++ iter;
            REQUIRE(r_iter.base() == iter);
            REQUIRE(std::addressof(result) == std::addressof(r_iter));
        }

        if(iter != container.begin())
        {
            auto r_iter_old = r_iter++;

            REQUIRE(r_iter_old.base() == iter);

            -- iter;
            REQUIRE(r_iter.base() == iter);
        }

        if(iter != container.end())
        {
            auto r_iter_old = r_iter--;

            REQUIRE(r_iter_old.base() == iter);

            ++ iter;
            REQUIRE(r_iter.base() == iter);
        }
    }

    template <class Container>
    void check_reverse_iterator_random_access_operations(Container container)
    {
        auto const pos = saga_test::random_position_of(container);
        auto const iter = std::next(container.begin(), pos);
        auto const r_iter = saga::make_reverse_iterator(iter);

        auto const n_plus = saga_test::random_uniform(0, pos);
        auto const n_minus = saga_test::random_uniform(pos, container.size()) - pos;

        REQUIRE(r_iter + n_plus == saga::make_reverse_iterator(iter - n_plus));
        REQUIRE(r_iter - n_minus == saga::make_reverse_iterator(iter + n_minus));
        REQUIRE(n_plus + r_iter == saga::make_reverse_iterator(iter - n_plus));

        {
            auto r = r_iter;
            auto & ptr = (r += n_plus);

            REQUIRE(r.base() == r_iter.base() - n_plus);
            REQUIRE(std::addressof(ptr) == std::addressof(r));
        }

        {
            auto r = r_iter;
            auto & ptr = (r -= n_minus);

            REQUIRE(r.base() == r_iter.base() + n_minus);
            REQUIRE(std::addressof(ptr) == std::addressof(r));
        }

        if(!container.empty())
        {
            using Difference = typename Container::difference_type;

            auto const n = Difference(pos) - 1 - saga_test::random_index_of(container);
            REQUIRE(std::addressof(r_iter[n]) == std::addressof(iter[-n-1]));
        }

        auto const iter2 = saga_test::random_iterator_of(container);
        auto const r_iter2 = saga::make_reverse_iterator(iter2);

        REQUIRE(r_iter2 - r_iter == (iter - iter2));
    }
}

TEST_CASE("reverse_iterator : types")
{
    check_reverse_iterator_typedefs<std::list<int>::iterator>();
    check_reverse_iterator_typedefs<std::list<int>::const_iterator>();
    check_reverse_iterator_typedefs<std::deque<int>::iterator>();
    check_reverse_iterator_typedefs<std::deque<int>::const_iterator>();
    check_reverse_iterator_typedefs<std::vector<int>::iterator>();
    check_reverse_iterator_typedefs<std::vector<int>::const_iterator>();
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
        constexpr saga::iota_iterator<int> it1(13);
        constexpr saga::iota_iterator<long> it1_a(13);
        constexpr saga::iota_iterator<long> it2(42);

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
        using Iterator = saga::iota_iterator<int>;

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
        constexpr saga::iota_iterator<int> const it1(42);

        constexpr auto const r_it1 = saga::make_reverse_iterator(it1);
        constexpr saga::reverse_iterator<saga::iota_iterator<long>> r_it2(r_it1);

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
        constexpr saga::iota_iterator<int> const it1(13);
        constexpr saga::iota_iterator<long> const it2(42);

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

namespace
{
    template <class T, std::size_t N>
    constexpr bool check_reverse_iterator_dereference_constexpr(T(&arr)[N])
    {
        auto iter = saga::begin(arr) + N / 2;

        auto const r_iter = saga::make_reverse_iterator(iter);

        return (*r_iter == *iter - 1);

        // Нужен constexpr для std::addressof: static_assert(r_iter.operator->() == &(*r_iter), "");
    }
}

TEST_CASE("reverse_iterator : dereference")
{
    {
        constexpr int arr[] = {1, 2, 3, 4, 5};

        static_assert(::check_reverse_iterator_dereference_constexpr(arr), "");
    }

    saga_test::property_checker
        << ::check_reverse_iterator_dereference_for_container<std::list<int>>
        << ::check_reverse_iterator_dereference_for_container<std::deque<int>>
        << ::check_reverse_iterator_dereference_for_container<std::vector<int>>;
}

namespace
{
    template <class Iterator>
    constexpr Iterator constexpr_increment_decrement(Iterator iter)
    {
        ++iter;
        --iter;
        iter++;
        iter--;

        return iter;
    }
}

TEST_CASE("reverse_iterator : increment and decrement")
{
    {
        constexpr saga::iota_iterator<long> iter(42);
        constexpr auto r_iter = saga::make_reverse_iterator(iter);

        static_assert(::constexpr_increment_decrement(r_iter) == r_iter, "");
    }

    saga_test::property_checker
        << ::check_reverse_iterator_increment_and_decrement_for_container<std::list<int>>
        << ::check_reverse_iterator_increment_and_decrement_for_container<std::deque<int>>
        << ::check_reverse_iterator_increment_and_decrement_for_container<std::vector<int>>;
}

namespace
{
    template <class Iterator, class Size>
    constexpr Iterator constexpr_plus_assign_minus_assign(Iterator iter, Size n)
    {
        iter += n;
        iter -= n;
        return iter;
    }
}

TEST_CASE("reverse_iterator : random access operations")
{
    {
        constexpr saga::iota_iterator<long> iter(42);
        constexpr auto n = 13;

        constexpr auto r_iter = saga::make_reverse_iterator(iter);

        static_assert(r_iter + n == saga::make_reverse_iterator(iter - n), "");
        static_assert(r_iter - n == saga::make_reverse_iterator(iter + n), "");
        static_assert(r_iter[n] == iter[-n-1], "");
        static_assert(r_iter - r_iter == 0, "");
        static_assert(n + r_iter == r_iter + n, "");

        static_assert(::constexpr_plus_assign_minus_assign(r_iter, n) == r_iter, "");
    }

    saga_test::property_checker
        << ::check_reverse_iterator_random_access_operations<std::deque<int>>
        << ::check_reverse_iterator_random_access_operations<std::vector<long>>;
}

TEST_CASE("reverse_iterator : compare")
{
    {
        constexpr saga::iota_iterator<int> it1(13);
        constexpr saga::iota_iterator<long> it1_a(13);
        constexpr saga::iota_iterator<long> it2(42);

        constexpr auto const r_it1 = saga::make_reverse_iterator(it1);
        constexpr auto const r_it1_a = saga::make_reverse_iterator(it1_a);
        constexpr auto const r_it2 = saga::make_reverse_iterator(it2);

        static_assert(it1 < it2, "");
        static_assert(it1 == it1_a, "");

        static_assert(r_it2 < r_it1, "");
        static_assert(r_it1 > r_it2, "");
        static_assert(r_it2 <= r_it1, "");
        static_assert(r_it1 >= r_it2, "");
        static_assert(r_it1 >= r_it1_a, "");
        static_assert(r_it1 <= r_it1_a, "");
    }

    saga_test::property_checker
        << ::check_reverse_iterator_comparison_for_container<std::deque<int>>
        << ::check_reverse_iterator_comparison_for_container<std::vector<int>>;
}

TEST_CASE("reverse_iterator : make_reverse_iterator is inverse of itself")
{
    {
        constexpr auto iter = saga::iota_iterator<long>(42);
        constexpr auto r_iter = saga::make_reverse_iterator(iter);
        constexpr auto rr_iter = saga::make_reverse_iterator(r_iter);

        static_assert(std::is_same<decltype(rr_iter), decltype(iter)>{}, "");
        static_assert(rr_iter == iter, "");
    }

    saga_test::property_checker
        << ::check_make_reverse_iterator_is_inverse_of_itself<std::list<int>>
        << ::check_make_reverse_iterator_is_inverse_of_itself<std::deque<long>>
        << ::check_make_reverse_iterator_is_inverse_of_itself<std::vector<int>>;
}

TEST_CASE("reverse_iterator : equality with std::reverse_iterator")
{
    using Container = std::list<int>;

    saga_test::property_checker << [](Container const & container)
    {
        auto const ri_std = std::make_reverse_iterator(saga_test::random_iterator_of(container));
        auto const ri_saga = saga::make_reverse_iterator(saga_test::random_iterator_of(container));

        REQUIRE((ri_std == ri_saga) == (ri_std.base() == ri_saga.base()));
        REQUIRE((ri_saga == ri_std) == (ri_std == ri_saga));

        REQUIRE((ri_std != ri_saga) == (ri_std.base() != ri_saga.base()));
        REQUIRE((ri_saga != ri_std) == (ri_std != ri_saga));
    };
}

TEST_CASE("reverse_iterator : comparison with std::reverse_iterator")
{
    using Container = std::vector<int>;

    saga_test::property_checker << [](Container const & container)
    {
        auto const ri_std = std::make_reverse_iterator(saga_test::random_iterator_of(container));
        auto const ri_saga = saga::make_reverse_iterator(saga_test::random_iterator_of(container));

        REQUIRE((ri_std == ri_saga) == (ri_std.base() == ri_saga.base()));
        REQUIRE((ri_saga == ri_std) == (ri_std == ri_saga));

        REQUIRE((ri_std != ri_saga) == (ri_std.base() != ri_saga.base()));
        REQUIRE((ri_saga != ri_std) == (ri_std != ri_saga));

        REQUIRE((ri_std < ri_saga) == (ri_std.base() > ri_saga.base()));
        REQUIRE((ri_saga < ri_std) == (ri_std > ri_saga));

        REQUIRE((ri_std <= ri_saga) == (ri_std.base() >= ri_saga.base()));
        REQUIRE((ri_saga <= ri_std) == (ri_std >= ri_saga));

        REQUIRE((ri_std > ri_saga) == (ri_std.base() < ri_saga.base()));
        REQUIRE((ri_saga > ri_std) == (ri_std < ri_saga));

        REQUIRE((ri_std >= ri_saga) == (ri_std.base() <= ri_saga.base()));
        REQUIRE((ri_saga >= ri_std) == (ri_std <= ri_saga));
    };
}

TEST_CASE("reverse_iterator : ctor from std::reverse_iterator<Iterator>")
{
    using Container = std::list<int>;

    saga_test::property_checker << [](Container container)
    {
        using Iterator = Container::iterator;
        using CIterator = Container::const_iterator;

        static_assert(std::is_constructible<saga::reverse_iterator<Iterator>,
                                             std::reverse_iterator<Iterator>>{}, "");
        static_assert(std::is_constructible<saga::reverse_iterator<CIterator>,
                                             std::reverse_iterator<CIterator>>{}, "");
        static_assert(std::is_constructible<saga::reverse_iterator<CIterator>,
                                             std::reverse_iterator<Iterator>>{}, "");

        static_assert(!std::is_constructible<Iterator, CIterator>{}, "");
        static_assert(!std::is_constructible<saga::reverse_iterator<Iterator>,
                                              std::reverse_iterator<CIterator>>{}, "");

        static_assert(std::is_convertible<saga::reverse_iterator<Iterator>,
                                           std::reverse_iterator<Iterator>>{}, "");
        static_assert(std::is_convertible<saga::reverse_iterator<CIterator>,
                                           std::reverse_iterator<CIterator>>{}, "");
        static_assert(std::is_convertible<saga::reverse_iterator<Iterator>,
                                           std::reverse_iterator<CIterator>>{}, "");
        static_assert(!std::is_convertible<saga::reverse_iterator<CIterator>,
                                            std::reverse_iterator<Iterator>>{}, "");

        static_assert(std::is_convertible<std::reverse_iterator<Iterator>,
                                           saga::reverse_iterator<Iterator>>{}, "");
        static_assert(std::is_convertible<std::reverse_iterator<CIterator>,
                                           saga::reverse_iterator<CIterator>>{}, "");
        static_assert(std::is_convertible<std::reverse_iterator<Iterator>,
                                           saga::reverse_iterator<CIterator>>{}, "");
        static_assert(!std::is_convertible<std::reverse_iterator<CIterator>,
                                            saga::reverse_iterator<Iterator>>{}, "");

        auto const iter = saga_test::random_iterator_of(container);

        auto const std_r_iter = std::make_reverse_iterator(iter);

        saga::reverse_iterator<CIterator> saga_r_iter(std_r_iter);

        REQUIRE(saga_r_iter == std_r_iter);
    };
}

TEST_CASE("reverse_iterator : assign from std::reverse_iterator<Iterator>")
{
    using Container = std::list<int>;

    saga_test::property_checker << [](Container container)
    {
        using Iterator = Container::iterator;
        using CIterator = Container::const_iterator;

        static_assert(std::is_assignable<saga::reverse_iterator<Iterator> &,
                                         std::reverse_iterator<Iterator> const &>{}, "");
        static_assert(std::is_assignable<saga::reverse_iterator<CIterator> &,
                                         std::reverse_iterator<Iterator> const &>{}, "");

        static_assert(!std::is_assignable<saga::reverse_iterator<Iterator> &,
                                              std::reverse_iterator<CIterator> const &>{}, "");

        auto const std_iter = std::make_reverse_iterator(saga_test::random_iterator_of(container));
        auto saga_iter = saga::make_reverse_iterator(saga_test::random_const_iterator_of(container));

        saga_iter = std_iter;

        REQUIRE(saga_iter == std_iter);
    };
}

TEST_CASE("reverse_iterator difference with std::reverse_iterator")
{
    using Container = std::vector<int>;

    saga_test::property_checker << [](Container container)
    {
        auto const std_iter = std::make_reverse_iterator(saga_test::random_iterator_of(container));
        auto const saga_iter = saga::make_reverse_iterator(saga_test::random_const_iterator_of(container));

        REQUIRE((std_iter - saga_iter) == (saga_iter.base() - std_iter.base()));
        REQUIRE((saga_iter - std_iter) == (std_iter.base() - saga_iter.base()));
    };
}

TEST_CASE("make_reverse_iterator(std::reverse_iterator<Iterator>) -> Iterator")
{
    using Container = std::list<int>;

    saga_test::property_checker << [](Container const & container)
    {
        auto const iter = saga_test::random_iterator_of(container);

        auto const r_iter = std::make_reverse_iterator(iter);

        auto const rr_iter = saga::make_reverse_iterator(r_iter);

        static_assert(std::is_same<decltype(iter), decltype(rr_iter)>{}, "");

        REQUIRE(iter == rr_iter);
    };
}
