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
#include <saga/iterator.hpp>

// Тестовая инфраструктура
#include "saga_test.hpp"
#include <catch2/catch_amalgamated.hpp>

// Вспомогательные файлы
#include <saga/algorithm.hpp>
#include <saga/cursor/subrange.hpp>
#include <saga/cursor/indices.hpp>

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

    // Требования к типам
    using Iterator = saga::back_emplace_iterator<Container>;

    static_assert(!std::is_default_constructible<Iterator>{}, "");
    check_emplace_iterators_typedefs<Iterator, Container>();

    // Основной функционал
    saga_test::property_checker
    << [](std::vector<saga_test::container_size<Size>> const & nums)
    {
        std::vector<Container> result;

        auto it = std::copy(nums.begin(), nums.end(), saga::back_emplacer(result));

        REQUIRE(it.container() == std::addressof(result));

        REQUIRE(result.size() == nums.size());

        for(auto i : saga::cursor::indices_of(nums))
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

    // Требования к типам
    using Iterator = saga::front_emplace_iterator<Container>;

    static_assert(!std::is_default_constructible<Iterator>{}, "");
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

        for(auto i : saga::cursor::indices_of(nums))
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

    // Требования к типам
    using Iterator = saga::emplace_iterator<Container>;

    static_assert(!std::is_default_constructible<Iterator>{}, "");
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

TEST_CASE("back_insterter: rvalue")
{
    using Value = int;

    saga_test::property_checker << [](Value const & value)
    {
          auto ptr = std::make_unique<Value>(value);
          auto * const addr = ptr.get();

          std::vector<std::unique_ptr<Value>> dest;
          auto out = saga::back_inserter(dest);

          *out = std::move(ptr);
          ++ out;

          REQUIRE(dest.size() == 1);
          REQUIRE(static_cast<bool>(dest.front()));
          REQUIRE(*dest.front() == value);
          REQUIRE(dest.front().get() == addr);
    };
}

TEST_CASE("front_inserter")
{
    using Value = int;
    using Container = std::list<Value>;

    // Требования к типам
    using Iterator = saga::front_insert_iterator<Container>;

    static_assert(!std::is_default_constructible<Iterator>{}, "");
    check_emplace_iterators_typedefs<Iterator, Container>();

    // Основной функционал
    saga_test::property_checker
    << [](std::vector<Value> const & src, std::list<Value> const & dest_old)
    {
        auto dest = dest_old;

        auto it = saga::copy(saga::cursor::all(src), saga::front_inserter(dest)).out;

        REQUIRE(it.container() == std::addressof(dest));

        std::list<Value> dest_expected;
        saga::reverse_copy(saga::cursor::all(src), saga::back_emplacer(dest_expected));
        saga::copy(saga::cursor::all(dest_old), saga::back_emplacer(dest_expected));

        REQUIRE(dest == dest_expected);
    };
}

TEST_CASE("front_insterter: rvalue")
{
    using Value = int;

    saga_test::property_checker << [](Value const & value1, Value const & value2)
    {
          auto ptr1 = std::make_unique<Value>(value1);
          auto * const addr1 = ptr1.get();

          auto ptr2 = std::make_unique<Value>(value2);
          auto * const addr2 = ptr2.get();

          std::list<std::unique_ptr<Value>> dest;
          auto out = saga::front_inserter(dest);

          *out = std::move(ptr1);
          ++ out;

          *out = std::move(ptr2);
          ++ out;

          REQUIRE(dest.size() == 2);
          REQUIRE(static_cast<bool>(dest.front()));
          REQUIRE(static_cast<bool>(dest.back()));

          REQUIRE(*dest.front() == value2);
          REQUIRE(dest.front().get() == addr2);

          REQUIRE(*dest.back() == value1);
          REQUIRE(dest.back().get() == addr1);
    };
}

TEST_CASE("inserter")
{
    using Value = long;
    using Container = std::vector<Value>;

    // Требования к типам
    using Iterator = saga::insert_iterator<Container>;

    static_assert(!std::is_default_constructible<Iterator>{}, "");
    check_emplace_iterators_typedefs<Iterator, Container>();

    // Основной функционал
    saga_test::property_checker
    << [](Container dest, std::list<Value> const & values)
    {
        auto const pos = saga_test::random_iterator_of(dest);

        // Ожидаемое
        Container dest_expected;
        std::copy(dest.begin(), pos, saga::back_inserter(dest_expected));
        saga::copy(saga::cursor::all(values), saga::back_inserter(dest_expected));
        std::copy(pos, dest.end(), saga::back_inserter(dest_expected));

        // inserter
        auto cur = saga::copy(saga::cursor::all(values), saga::inserter(dest, pos)).out;

        REQUIRE(cur.container() == std::addressof(dest));
        REQUIRE(dest == dest_expected);
    };
}

TEST_CASE("insterter: rvalue")
{
    using Value = int;

    saga_test::property_checker
    << [](Value const & value1, Value const & value2, Value const & value3, Value const & value4)
    {
          auto ptr1 = std::make_unique<Value>(value1);
          auto * const addr1 = ptr1.get();

          auto ptr2 = std::make_unique<Value>(value2);
          auto * const addr2 = ptr2.get();

          std::vector<std::unique_ptr<Value>> dest;
          dest.push_back(std::make_unique<Value>(value3));
          dest.push_back(std::make_unique<Value>(value4));

          auto out = saga::inserter(dest, dest.begin() + 1);

          *out = std::move(ptr1);
          ++ out;

          *out = std::move(ptr2);
          ++ out;

          REQUIRE(dest.size() == 4);

          for(auto const & item : dest)
          {
              REQUIRE(static_cast<bool>(item));
          }

          REQUIRE(*dest.at(1) == value1);
          REQUIRE(dest.at(1).get() == addr1);

          REQUIRE(*dest.at(2) == value2);
          REQUIRE(dest.at(2).get() == addr2);
    };
}

// ostream_joiner
// Проверки типов
namespace
{
    using OStream = std::ostringstream;
    using Delim = char[3];

    using OSJoiner = saga::ostream_joiner<Delim, OStream>;

    static_assert(std::is_same<OSJoiner::char_type, OStream::char_type>{}, "");
    static_assert(std::is_same<OSJoiner::traits_type, OStream::traits_type>{}, "");
    static_assert(std::is_same<OSJoiner::ostream_type, OStream>{}, "");
    static_assert(std::is_same<OSJoiner::iterator_category, std::output_iterator_tag>{}, "");
    static_assert(std::is_same<OSJoiner::value_type, void>{}, "");
    static_assert(std::is_same<OSJoiner::difference_type, std::ptrdiff_t>{}, "");
    static_assert(std::is_same<OSJoiner::pointer, void>{}, "");
    static_assert(std::is_same<OSJoiner::reference, void>{}, "");

    static_assert(noexcept(*std::declval<OSJoiner>()), "");
    static_assert(noexcept(!std::declval<OSJoiner>()), "");
    static_assert(noexcept(++std::declval<OSJoiner>()), "");
    static_assert(noexcept(std::declval<OSJoiner>()++), "");
}

TEST_CASE("ostream_joiner")
{
    using Value = long;

    saga_test::property_checker << [](std::vector<Value> const & values)
    {
        auto const delim = std::string(", ");

        // Явное
        std::ostringstream os_for;

        if(!values.empty())
        {
            auto first = values.begin();
            auto const last = values.end();

            os_for << *first;
            ++ first;

            for(; first != last; ++ first)
            {
                os_for << delim << *first;
            }
        }

        // Как итератор
        std::ostringstream os_iter;
        std::copy(values.begin(), values.end(), saga::ostream_joiner(os_iter, delim));

        REQUIRE(os_iter.str() == os_for.str());

        // Как курсор
        std::ostringstream os_cur;
        auto cur = saga::copy(saga::cursor::all(values), saga::ostream_joiner(os_cur, delim)).out;

        REQUIRE(os_cur.str() == os_for.str());

        static_assert(std::is_same<decltype(cur++), decltype(++cur)>{}, "");
        REQUIRE(std::addressof(cur++) == std::addressof(cur));
    };
}
