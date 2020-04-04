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

//Тесты
TEST_CASE("back_implacer")
{
    using Size = std::size_t;
    using Container = std::vector<int>;

    static_assert(std::is_constructible<Container, Size>{}, "");
    static_assert(!std::is_convertible<Size, Container>{}, "");

    // Конструктор по умолчанию
    saga::back_emplace_iterator<Container> it_0;
    REQUIRE(std::addressof(it_0.container()) == nullptr);

    using Iterator = decltype(it_0);

    static_assert(std::is_same<Iterator::iterator_category, std::output_iterator_tag>{}, "");
    static_assert(std::is_same<Iterator::value_type, void>{}, "");
    static_assert(std::is_same<Iterator::difference_type, void>{}, "");
    static_assert(std::is_same<Iterator::pointer, void>{}, "");
    static_assert(std::is_same<Iterator::reference, void>{}, "");
    static_assert(std::is_same<Iterator::container_type, Container>{}, "");

    // Основной функционал
    saga_test::property_checker
    << [](std::vector<saga_test::container_size<Size>> const & nums)
    {
        std::vector<Container> result;

        auto it = std::copy(nums.begin(), nums.end(), saga::back_emplacer(result));

        REQUIRE(std::addressof(it.container()) == std::addressof(result));

        for(auto i : saga::view::indices_of(nums))
        {
            result.at(i) == Container(nums.at(i));
        }
    };
}
