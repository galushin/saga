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

// @todo make_reverse_iterator
