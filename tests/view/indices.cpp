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
#include <saga/view/indices.hpp>

// Тестовая инфраструктура
#include <catch/catch.hpp>
#include "../saga_test.hpp"

// Вспомогательные файлы
#include <saga/iterator.hpp>

#include <forward_list>
#include <list>

// Тесты
static_assert(sizeof(saga::detail::iota_iterator<int>) == sizeof(int), "");
// @todo Проверить категорию iota_iterator - придумать все
// @todo iota_iterator<void *> - осмысленно?

static_assert(std::is_same<saga::detail::iota_iterator<std::forward_list<int>::iterator>::iterator_category,
                           std::forward_iterator_tag>{}, "");
static_assert(std::is_same<saga::detail::iota_iterator<std::list<int>::iterator>::iterator_category,
                           std::bidirectional_iterator_tag>{}, "");
static_assert(std::is_same<saga::detail::iota_iterator<std::vector<int>::iterator>::iterator_category,
                           std::random_access_iterator_tag>{}, "");
static_assert(std::is_same<saga::detail::iota_iterator<int const *>::iterator_category,
                           std::random_access_iterator_tag>{}, "");
static_assert(std::is_same<saga::detail::iota_iterator<int>::iterator_category,
                           std::random_access_iterator_tag>{}, "");

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
        for(auto const & i : saga::view::indices_of(xs))
        {
            ins.push_back(i);
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

    std::vector<Index> ins;
    for(auto const & i : saga::view::indices_of(xs))
    {
        ins.push_back(i);
    }

    CHECK(ins == ins_obj);
}
