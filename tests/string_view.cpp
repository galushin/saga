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
#include <saga/string_view.hpp>

// Тестовая инфраструктура
#include "saga_test.hpp"
#include <catch/catch.hpp>

// Вспомогательные файлы
#include <iomanip>
#include <sstream>

// Тесты
using StringView = saga::basic_string_view<char, std::char_traits<char>>;

static_assert(std::is_same<saga::basic_string_view<char>,
                           saga::basic_string_view<char, std::char_traits<char>>>::value, "");

static_assert(std::is_same<saga::string_view, saga::basic_string_view<char>>::value, "");
static_assert(std::is_same<saga::u16string_view, saga::basic_string_view<char16_t>>::value, "");
static_assert(std::is_same<saga::u32string_view, saga::basic_string_view<char32_t>>::value, "");
static_assert(std::is_same<saga::wstring_view, saga::basic_string_view<wchar_t>>::value, "");

// Типы
static_assert(std::is_same<saga::string_view::traits_type, std::char_traits<char>>::value, "");
static_assert(std::is_same<saga::string_view::value_type, char>::value, "");
static_assert(std::is_same<saga::string_view::pointer, saga::string_view::value_type *>::value, "");
static_assert(std::is_same<saga::string_view::const_pointer,
                           saga::string_view::value_type const *>::value, "");
static_assert(std::is_same<saga::string_view::reference, saga::string_view::value_type &>::value, "");
static_assert(std::is_same<saga::string_view::const_reference,
                           saga::string_view::value_type const &>::value, "");

using SV_const_iterator = saga::string_view::const_iterator;
static_assert(std::is_convertible<std::iterator_traits<SV_const_iterator>::iterator_category,
                                  std::random_access_iterator_tag>::value,"");

static_assert(std::is_same<saga::string_view::iterator, saga::string_view::const_iterator>{}, "");
static_assert(std::is_same<saga::string_view::const_reverse_iterator,
                           saga::reverse_iterator<saga::string_view::const_iterator>>{},"");
static_assert(std::is_same<saga::string_view::reverse_iterator,
                           saga::string_view::const_reverse_iterator>{}, "");
static_assert(std::is_same<saga::string_view::size_type, std::size_t>{}, "");
static_assert(std::is_same<saga::string_view::difference_type, std::ptrdiff_t>{}, "");
static_assert(saga::string_view::npos == saga::string_view::size_type(-1), "");

static_assert(std::is_nothrow_default_constructible<saga::string_view>{}, "");

namespace
{
    struct string_view_struct
    {
        saga::string_view::pointer ptr;
        saga::string_view::size_type size;
    };
}

static_assert(sizeof(saga::string_view) <= sizeof(string_view_struct), "");

TEST_CASE("string_view : default ctor")
{
    static_assert(noexcept(saga::string_view{}), "");

    constexpr saga::string_view const sv0{};

    static_assert(sv0.empty(), "");
    static_assert(sv0.size() == 0, "");
    static_assert(sv0.data() == nullptr, "");
}

// Конструктор копий и присваивание нельзя тестировать полноценно без других конструкторов
static_assert(std::is_nothrow_copy_constructible<saga::string_view>{}, "");
static_assert(std::is_nothrow_copy_assignable<saga::string_view>{}, "");

TEST_CASE("string_view : ctor from null-terminated string")
{
    saga_test::property_checker << [](std::string const & str)
    {
        char const * z_str = str.c_str();

        saga::string_view const sv(z_str);

        REQUIRE(sv.size() == saga::string_view::traits_type::length(z_str));
        REQUIRE(sv.data() == z_str);
    };
}

TEST_CASE("string_view : construct from nullptr")
{
    saga::string_view const sv(nullptr);

    REQUIRE(sv.size() == 0);
    REQUIRE(sv.data() == nullptr);
}

TEST_CASE("string_view : ctor from pointer and size")
{
    saga_test::property_checker << [](std::string const & str)
    {
        auto const num = saga_test::random_position_of(str);

        saga::string_view const sv(str.c_str(), num);

        REQUIRE(sv.size() == num);
        REQUIRE(sv.length() == sv.size());
        REQUIRE(sv.data() == str.data());
    };
}

TEST_CASE("string_view : ctor from pointer and size; constexpr")
{
    constexpr char const * z_str = "This is a test string";
    constexpr auto const num = 7;

    constexpr saga::string_view const sv(z_str, num);

    static_assert(sv.size() == num, "");
    static_assert(sv.length() == sv.size(), "");
    static_assert(sv.data() == z_str, "");
}

TEST_CASE("string_view : ctor from std::string")
{
    static_assert(std::is_convertible<std::string, saga::string_view>::value, "");

    saga_test::property_checker << [](std::string const & str)
    {
        saga::string_view const sv(str);

        static_assert(noexcept(saga::string_view(str)), "");

        REQUIRE(sv.data() == str.data());
        REQUIRE(sv.size() == str.size());
    };
}

TEST_CASE("string_view : implicit coversion to string_view")
{
    static_assert(std::is_convertible<saga::string_view, std::string>::value, "");

    saga_test::property_checker << [](std::string const & str)
    {
        saga::string_view const sv(str);

        std::string const str2 = sv;

        REQUIRE(str == str2);
    };
}

TEST_CASE("string_veiw : iterators are random access")
{
    saga_test::property_checker << [](std::string const & str)
    {
        saga::string_view const sv(str);

        REQUIRE(std::is_heap(sv.begin(), sv.end()) == std::is_heap(str.begin(), str.end()));
    };
}

TEST_CASE("string_veiw : iterators are сontiguous")
{
    saga_test::property_checker << [](std::string const & str)
    {
        saga::string_view const sv(str);

        auto const n = sv.size();
        auto const first = sv.begin();

        for(auto index = 0*n; index < n; ++ index)
        {
            REQUIRE(std::addressof(first[index]) == std::addressof(*first) + index);
        }
    };
}


TEST_CASE("string_view : iterators of empty")
{
    constexpr saga::string_view const sv0{};

    static_assert(sv0.empty(), "");

    static_assert(sv0.begin() == sv0.end(), "");
    static_assert(sv0.cbegin() == sv0.begin(), "");
    static_assert(sv0.cbegin() == sv0.cend(), "");

    static_assert(sv0.rbegin() == sv0.rend(), "");
    static_assert(sv0.crbegin() == sv0.rbegin(), "");
    static_assert(sv0.crbegin() == sv0.crend(), "");
}

TEST_CASE("string_view : iterators, common case")
{
    saga_test::property_checker << [](std::string const & str)
    {
        saga::string_view const sv(str.c_str(), str.size());

        CAPTURE(str);
        REQUIRE(sv.empty() == str.empty());

        if(!sv.empty())
        {
            REQUIRE(std::addressof(*sv.begin()) == sv.data());
            REQUIRE(sv.cbegin() == sv.begin());
        }

        REQUIRE(sv.end() == sv.begin() + sv.size());
        REQUIRE(sv.cend() == sv.end());

        REQUIRE(sv.rbegin() == saga::string_view::reverse_iterator(sv.end()));
        REQUIRE(sv.crbegin() == sv.rbegin());
        REQUIRE(sv.rend() == saga::string_view::reverse_iterator(sv.begin()));
        REQUIRE(sv.crend() == sv.rend());
    };
}

TEST_CASE("string_view : max_size")
{
    constexpr saga::string_view const sv{};

    static_assert(noexcept(sv.max_size()), "");
    static_assert(std::is_same<decltype(sv.max_size()), saga::string_view::size_type>{}, "");

    constexpr auto const ms = sv.max_size();

    REQUIRE(ms >= std::string{}.max_size());
}

TEST_CASE("string_view : operator[], front, back")
{
    saga_test::property_checker << [](std::string const & str)
    {
        auto const num = saga_test::random_position_of(str);

        saga::string_view const sv(str.c_str(), num);

        for(auto index = 0*sv.size(); index < sv.size(); ++ index)
        {
            REQUIRE(sv[index] == str.at(index));
            REQUIRE(std::addressof(sv[index]) == sv.data() + index);
        }

        if(!sv.empty())
        {
            REQUIRE(sv.front() == sv[0]);
            REQUIRE(sv.back() == sv[sv.size() - 1]);
        }
    };
}

TEST_CASE("string_view : operator[], front, back; constexpr")
{
    constexpr char const * z_str = "This is a test string";
    constexpr auto const num = 7;

    constexpr saga::string_view const sv(z_str, num);

    static_assert(!sv.empty(), "");
    static_assert(sv.front() == sv[0], "");
    static_assert(sv.back() == sv[sv.size() - 1], "");
}

TEST_CASE("string_view : at")
{
    saga_test::property_checker << [](std::string const & str, std::size_t index)
    {
        auto const num = saga_test::random_position_of(str);

        saga::string_view const sv(str.c_str(), num);

        if(index < sv.size())
        {
            REQUIRE(std::addressof(sv.at(index)) == std::addressof(sv[index]));
        }
        else
        {
            REQUIRE_THROWS_AS(sv.at(index), std::out_of_range);
        }
    };
}

TEST_CASE("string_view : at; constexpr")
{
    constexpr char const * z_str = "This is a test string";
    constexpr auto const num = 7;

    constexpr saga::string_view const sv(z_str, num);

    static_assert(sv.at(0) == sv[0], "");
    static_assert(sv.at(1) == sv[1], "");
    static_assert(sv.at(2) == sv[2], "");
    static_assert(sv.at(3) == sv[3], "");
    static_assert(sv.at(4) == sv[4], "");
    static_assert(sv.at(5) == sv[5], "");
    static_assert(sv.at(6) == sv[6], "");
}

// data покрыта тестами выше

TEST_CASE("string_view : remove_prefix")
{

    saga_test::property_checker << [](std::string const & str)
    {
        auto const num = saga_test::random_position_of(str);

        saga::string_view sv(str.c_str(), str.size());

        auto const sv_old = sv;

        sv.remove_prefix(num);

        REQUIRE(sv.data() == sv_old.data() + num);
        REQUIRE(sv.size() == sv_old.size() - num);
    };
}

TEST_CASE("string_view : remove_suffix")
{
    saga_test::property_checker << [](std::string const & str)
    {
        auto const num = saga_test::random_position_of(str);

        saga::string_view sv(str.c_str(), str.size());

        auto const sv_old = sv;

        sv.remove_suffix(num);

        REQUIRE(sv.data() == sv_old.data());
        REQUIRE(sv.size() == sv_old.size() - num);
    };
}

TEST_CASE("string_view : swap")
{
    saga_test::property_checker << [](std::string const & str1, std::string const & str2)
    {
        saga::string_view sv1(str1.data(), str1.size());
        saga::string_view sv2(str2.data(), str2.size());

        static_assert(noexcept(sv1.swap(sv2)), "");

        sv1.swap(sv2);

        REQUIRE(sv1.data() == str2.data());
        REQUIRE(sv1.size() == str2.size());

        REQUIRE(sv2.data() == str1.data());
        REQUIRE(sv2.size() == str1.size());
    };
}

TEST_CASE("string_view : copy")
{
    saga_test::property_checker << [](std::string const & str_src, std::string const & str_dest_old)
    {
        saga::string_view const sv(str_src);

        std::string str_dest = str_dest_old;

        auto const num = saga_test::random_position_of(str_dest);
        auto const pos = saga_test::random_uniform(0*str_src.size(), 2*str_src.size());

        if(pos <= str_src.size())
        {
            auto const result = sv.copy(std::addressof(str_dest[0]), num, pos);

            REQUIRE(result == std::min(num, sv.size() - pos));

            static_assert(std::is_same<decltype(result), std::size_t const>{}, "");

            REQUIRE(str_dest.substr(0, result) == str_src.substr(pos, result));
            REQUIRE(str_dest.substr(result) == str_dest_old.substr(result));
        }
        else
        {
            REQUIRE_THROWS_AS(sv.copy(std::addressof(str_dest[0]), num, pos), std::out_of_range);
        }
    };
}

TEST_CASE("string_view : copy, default pos = 0")
{
    saga_test::property_checker << [](std::string const & str_src, std::string const & str_dest_old)
    {
        saga::string_view const sv(str_src);

        std::string str_dest_1 = str_dest_old;
        std::string str_dest_2 = str_dest_old;

        auto const num = saga_test::random_position_of(str_dest_1);

        auto const r1 = sv.copy(std::addressof(str_dest_1[0]), num);
        auto const r2 = sv.copy(std::addressof(str_dest_2[0]), num, 0);

        REQUIRE(r1 == r2);
        REQUIRE(str_dest_1 == str_dest_2);
    };
}

TEST_CASE("string_view : substr")
{
    saga_test::property_checker << [](std::string const & str)
    {
        saga::string_view const sv(str);

        auto const pos = saga_test::random_uniform(0*sv.size(), 2*sv.size());
        auto const num = saga_test::random_uniform(0*sv.size(), 2*sv.size());

        if(pos > sv.size())
        {
            REQUIRE_THROWS_AS(sv.substr(pos, num), std::out_of_range);
        }
        else
        {
            auto const rlen = std::min(num, sv.size() - pos);
            auto const expected = saga::string_view(sv.data() + pos, rlen);

            auto const actual = sv.substr(pos, num);

            static_assert(std::is_same<decltype(sv.substr(pos, num)), saga::string_view>{}, "");

            REQUIRE(actual.data() == expected.data());
            REQUIRE(actual.size() == expected.size());
        }
    };
}

TEST_CASE("string_view : default n")
{
    saga_test::property_checker << [](std::string const & str)
    {
        saga::string_view const sv(str);

        auto const pos = saga_test::random_uniform(0*sv.size(), 2*sv.size());

        if(pos > sv.size())
        {
            REQUIRE_THROWS_AS(sv.substr(pos), std::out_of_range);
        }
        else
        {
            auto const expected = sv.substr(pos, saga::string_view::npos);

            auto const actual = sv.substr(pos);

            REQUIRE(actual.data() == expected.data());
            REQUIRE(actual.size() == expected.size());
        }
    };
}

TEST_CASE("string_view : default n, default pos")
{
    saga_test::property_checker << [](std::string const & str)
    {
        saga::string_view const sv(str);

        auto const actual = sv.substr();

        REQUIRE(actual.data() == sv.data());
        REQUIRE(actual.size() == sv.size());
    };
}

namespace
{
    template <class Arithmetic>
    bool same_sign(Arithmetic const & x, Arithmetic const & y)
    {
        if(x < 0)
        {
            return y < 0;
        }
        else if(x > 0)
        {
            return y > 0;
        }
        else
        {
            return y == 0;
        }
    }
}

TEST_CASE("string_view : compare with string_view")
{
    saga_test::property_checker << [](std::string const & str1, std::string const & str2)
    {
        saga::string_view const sv1(str1);
        saga::string_view const sv2(str2);

        static_assert(std::is_same<decltype(sv1.compare(sv2)), int>{}, "");
        static_assert(noexcept(sv1.compare(sv2)), "");

        auto const expected = str1.compare(str2);
        auto const actual = sv1.compare(sv2);

        REQUIRE(::same_sign(expected, actual));
    };
}

TEST_CASE("string_view : compare substring with string_view")
{
    saga_test::property_checker << [](std::string const & str1, std::string const & str2)
    {
        saga::string_view const sv1(str1);
        saga::string_view const sv2(str2);

        auto const pos1 = saga_test::random_uniform(0*sv1.size(), 2*sv1.size());
        auto const num1 = saga_test::random_uniform(0*sv1.size(), 2*sv1.size());

        static_assert(std::is_same<decltype(sv1.compare(pos1, num1, sv2)), int>{}, "");

        if(pos1 > sv1.size())
        {
            REQUIRE_THROWS_AS(sv1.compare(pos1, num1, sv2), std::out_of_range);
        }
        else
        {
            auto const expected = sv1.substr(pos1, num1).compare(str2);
            auto const actual = sv1.compare(pos1, num1, sv2);

            REQUIRE(expected == actual);
        }
    };
}

TEST_CASE("string_view : compare substring with string_view substr")
{
    saga_test::property_checker << [](std::string const & str1, std::string const & str2)
    {
        saga::string_view const sv1(str1);
        saga::string_view const sv2(str2);

        auto const pos1 = saga_test::random_uniform(0*sv1.size(), 2*sv1.size());
        auto const num1 = saga_test::random_uniform(0*sv1.size(), 2*sv1.size());

        auto const pos2 = saga_test::random_uniform(0*sv2.size(), 2*sv2.size());
        auto const num2 = saga_test::random_uniform(0*sv2.size(), 2*sv2.size());

        static_assert(std::is_same<decltype(sv1.compare(pos1, num1, sv2, pos2, num2)), int>{}, "");

        if(pos1 > sv1.size() || pos2 > sv2.size())
        {
            REQUIRE_THROWS_AS(sv1.compare(pos1, num1, sv2, pos2, num2), std::out_of_range);
        }
        else
        {
            auto const expected = sv1.substr(pos1, num1).compare(str2.substr(pos2, num2));
            auto const actual = sv1.compare(pos1, num1, sv2, pos2, num2);

            REQUIRE(expected == actual);
        }
    };
}

TEST_CASE("string_view : compare with c-string")
{
    saga_test::property_checker << [](std::string const & str1, std::string const & str2)
    {
        saga::string_view const sv(str1);
        char const * c_str = str2.c_str();

        static_assert(std::is_same<decltype(sv.compare(c_str)), int>{}, "");

        auto const expected = sv.compare(saga::string_view(c_str));
        auto const actual = sv.compare(c_str);

        REQUIRE(::same_sign(expected, actual));
    };
}

TEST_CASE("string_view : compare substring with c-string")
{
    saga_test::property_checker << [](std::string const & str1, std::string const & str2)
    {
        saga::string_view const sv(str1);
        char const * c_str = str2.c_str();

        auto const pos1 = saga_test::random_uniform(0*sv.size(), 2*sv.size());
        auto const num1 = saga_test::random_uniform(0*sv.size(), 2*sv.size());

        static_assert(std::is_same<decltype(sv.compare(pos1, num1, c_str)), int>{}, "");

        if(pos1 > sv.size())
        {
            REQUIRE_THROWS_AS(sv.compare(pos1, num1, c_str), std::out_of_range);
        }
        else
        {
            auto const expected = sv.substr(pos1, num1).compare(saga::string_view(c_str));
            auto const actual = sv.compare(pos1, num1, c_str);

            REQUIRE(expected == actual);
        }
    };
}

TEST_CASE("string_view : compare substring with {pointr, size}")
{
    saga_test::property_checker << [](std::string const & str1, std::string const & str2)
    {
        saga::string_view const sv(str1);
        char const * c_str = str2.c_str();
        auto const num2 = str2.size();

        auto const pos1 = saga_test::random_uniform(0*sv.size(), 2*sv.size());
        auto const num1 = saga_test::random_uniform(0*sv.size(), 2*sv.size());

        static_assert(std::is_same<decltype(sv.compare(pos1, num1, c_str)), int>{}, "");

        if(pos1 > sv.size())
        {
            REQUIRE_THROWS_AS(sv.compare(pos1, num1, c_str, num2), std::out_of_range);
        }
        else
        {
            auto const expected = sv.substr(pos1, num1).compare(saga::string_view(c_str, num2));
            auto const actual = sv.compare(pos1, num1, c_str, num2);

            REQUIRE(expected == actual);
        }
    };
}

TEST_CASE("string_view : starts_with string_view")
{
    saga_test::property_checker << [](std::string const & str1, std::string const & str2)
    {
        saga::string_view const sv1(str1);
        saga::string_view const sv2(str2);

        static_assert(std::is_same<decltype(sv1.starts_with(sv2)), bool>{}, "");
        static_assert(noexcept(sv1.starts_with(sv2)), "");

        auto const expected = sv1.size() >= sv2.size()
                            && sv1.compare(0, sv2.size(), sv2) == 0;

        REQUIRE(sv1.starts_with(sv2) == expected);
        REQUIRE(sv1.starts_with(sv1));
        REQUIRE(sv1.starts_with(sv1.substr(0, sv1.size()/2)));
    };
}

TEST_CASE("string_view : starts_with char")
{
    saga_test::property_checker << [](std::string const & str1, char ch2)
    {
        saga::string_view const sv1(str1);

        static_assert(std::is_same<decltype(sv1.starts_with(ch2)), bool>{}, "");
        static_assert(noexcept(sv1.starts_with(ch2)), "");

        CAPTURE(str1, ch2);

        REQUIRE(sv1.starts_with(ch2) == sv1.starts_with(saga::string_view(&ch2, 1)));
    };
}

TEST_CASE("string_view : starts_with c-string")
{
    saga_test::property_checker << [](std::string const & str1, std::string const & str2)
    {
        saga::string_view const sv1(str1);
        char const * c_str = str2.c_str();

        static_assert(std::is_same<decltype(sv1.starts_with(c_str)), bool>{}, "");

        REQUIRE(sv1.starts_with(c_str) == sv1.starts_with(saga::string_view(c_str)));
    };
}

TEST_CASE("string_view : ends_with string_view")
{
    saga_test::property_checker << [](std::string const & str1, std::string const & str2)
    {
        saga::string_view const sv1(str1);
        saga::string_view const sv2(str2);

        static_assert(std::is_same<decltype(sv1.ends_with(sv2)), bool>{}, "");
        static_assert(noexcept(sv1.ends_with(sv2)), "");

        auto const expected = (sv1.size() >= sv2.size())
                            && (sv1.compare(sv1.size() - sv2.size(), sv2.size(), sv2) == 0);

        CAPTURE(str1, str2);
        REQUIRE(sv1.ends_with(sv2) == expected);
    };
}

TEST_CASE("string_view : ends_with char")
{
    saga_test::property_checker << [](std::string const & str1, char ch2)
    {
        saga::string_view const sv1(str1);

        static_assert(std::is_same<decltype(sv1.ends_with(ch2)), bool>{}, "");
        static_assert(noexcept(sv1.ends_with(ch2)), "");

        CAPTURE(str1, ch2);
        REQUIRE(sv1.ends_with(ch2) == sv1.ends_with(saga::string_view(&ch2, 1)));
    };
}

TEST_CASE("string_view : ends_with c-string")
{
    saga_test::property_checker << [](std::string const & str1, std::string const & str2)
    {
        saga::string_view const sv1(str1);
        char const * c_str = str2.c_str();

        static_assert(std::is_same<decltype(sv1.ends_with(c_str)), bool>{}, "");

        REQUIRE(sv1.ends_with(c_str) == sv1.ends_with(saga::string_view(c_str)));
    };
}

TEST_CASE("string_view : find")
{
    saga_test::property_checker << [](std::string const & str1, std::string const & str2,
                                      std::size_t const pos, char const ch)
    {
        saga::string_view const sv1(str1);
        saga::string_view const sv2(str2);

        auto const num = saga_test::random_position_of(str2);

        static_assert(noexcept(sv1.find(sv2)), "");
        static_assert(noexcept(sv1.find(sv2, pos)), "");
        static_assert(noexcept(sv1.find(str2.c_str())), "");
        static_assert(noexcept(sv1.find(str2.c_str(), pos)), "");
        static_assert(noexcept(sv1.find(str2.c_str(), pos, num)), "");
        static_assert(noexcept(sv1.find(ch)), "");
        static_assert(noexcept(sv1.find(ch, pos)), "");

        CAPTURE(str1, str2);

        REQUIRE(sv1.find(sv1) == str1.find(str1));
        REQUIRE(sv1.find(sv2) == str1.find(str2));
        REQUIRE(sv2.find(sv1) == str2.find(str1));

        REQUIRE(sv1.find(sv2, pos) == str1.find(str2, pos));
        REQUIRE(sv2.find(sv1, pos) == str2.find(str1, pos));

        REQUIRE(sv1.find(str2.c_str()) == sv1.find(saga::string_view(str2.c_str())));
        REQUIRE(sv1.find(str2.c_str(), pos) == sv1.find(saga::string_view(str2.c_str()), pos));
        REQUIRE(sv1.find(str2.c_str(), pos, num)
                == sv1.find(saga::string_view(str2.c_str(), num), pos));
        REQUIRE(sv1.find(ch) == sv1.find(saga::string_view(&ch, 1)));
        REQUIRE(sv1.find(ch, pos) == sv1.find(saga::string_view(&ch, 1), pos));
    };
}

TEST_CASE("string_view : find searches first occurence")
{
    saga_test::property_checker << [](std::string const & str1)
    {
        std::string const str2 = str1 + str1;

        saga::string_view const sv1(str1);
        saga::string_view const sv2(str2);

        static_assert(noexcept(sv1.find(sv2)), "");

        CAPTURE(str1, str2);

        REQUIRE(sv1.find(sv2) == str1.find(str2));
    };
}

TEST_CASE("string_view : rfind")
{
    saga_test::property_checker << [](std::string const & str1, std::string const & str2,
                                      std::size_t const pos, char const ch)
    {
        saga::string_view const sv1(str1);
        saga::string_view const sv2(str2);

        auto const num = saga_test::random_position_of(str2);

        static_assert(noexcept(sv1.rfind(sv2)), "");
        static_assert(noexcept(sv1.rfind(sv2, pos)), "");
        static_assert(noexcept(sv1.rfind(str2.c_str())), "");
        static_assert(noexcept(sv1.rfind(str2.c_str(), pos)), "");
        static_assert(noexcept(sv1.rfind(str2.c_str(), pos, num)), "");
        static_assert(noexcept(sv1.rfind(ch)), "");
        static_assert(noexcept(sv1.rfind(ch, pos)), "");

        CAPTURE(str1, str2, pos, num, ch);

        REQUIRE(sv1.rfind(sv1) == str1.rfind(str1));
        REQUIRE(sv1.rfind(sv2) == str1.rfind(str2));
        REQUIRE(sv2.rfind(sv1) == str2.rfind(str1));

        REQUIRE(sv1.rfind(sv2, pos) == str1.rfind(str2, pos));
        REQUIRE(sv2.rfind(sv1, pos) == str2.rfind(str1, pos));

        REQUIRE(sv1.rfind(str2.c_str()) == sv1.rfind(saga::string_view(str2.c_str())));
        REQUIRE(sv1.rfind(str2.c_str(), pos) == sv1.rfind(saga::string_view(str2.c_str()), pos));
        REQUIRE(sv1.rfind(str2.c_str(), pos, num)
                == sv1.rfind(saga::string_view(str2.c_str(), num), pos));
        REQUIRE(sv1.rfind(ch) == sv1.rfind(saga::string_view(&ch, 1)));
        REQUIRE(sv1.rfind(ch, pos) == sv1.rfind(saga::string_view(&ch, 1), pos));
    };
}

TEST_CASE("string_view : find_first_of")
{
    saga_test::property_checker << [](std::string const & str1, std::string const & str2,
                                      std::size_t const pos, char const ch)
    {
        saga::string_view const sv1(str1);
        saga::string_view const sv2(str2);

        auto const num = saga_test::random_position_of(str2);

        static_assert(noexcept(sv1.find_first_of(sv2)), "");
        static_assert(noexcept(sv1.find_first_of(sv2, pos)), "");
        static_assert(noexcept(sv1.find_first_of(str2.c_str())), "");
        static_assert(noexcept(sv1.find_first_of(str2.c_str(), pos)), "");
        static_assert(noexcept(sv1.find_first_of(str2.c_str(), pos, num)), "");
        static_assert(noexcept(sv1.find_first_of(ch)), "");
        static_assert(noexcept(sv1.find_first_of(ch, pos)), "");

        CAPTURE(str1, str2, pos, num, ch);

        REQUIRE(sv1.find_first_of(sv1) == str1.find_first_of(str1));
        REQUIRE(sv1.find_first_of(sv2) == str1.find_first_of(str2));
        REQUIRE(sv2.find_first_of(sv1) == str2.find_first_of(str1));

        REQUIRE(sv1.find_first_of(sv2, pos) == str1.find_first_of(str2, pos));
        REQUIRE(sv2.find_first_of(sv1, pos) == str2.find_first_of(str1, pos));

        REQUIRE(sv1.find_first_of(str2.c_str())
                == sv1.find_first_of(saga::string_view(str2.c_str())));
        REQUIRE(sv1.find_first_of(str2.c_str(), pos)
                == sv1.find_first_of(saga::string_view(str2.c_str()), pos));
        REQUIRE(sv1.find_first_of(str2.c_str(), pos, num)
                == sv1.find_first_of(saga::string_view(str2.c_str(), num), pos));
        REQUIRE(sv1.find_first_of(ch) == sv1.find_first_of(saga::string_view(&ch, 1)));
        REQUIRE(sv1.find_first_of(ch, pos) == sv1.find_first_of(saga::string_view(&ch, 1), pos));
    };
}

TEST_CASE("string_view : find_last_of")
{
    saga_test::property_checker << [](std::string const & str1, std::string const & str2,
                                      std::size_t const pos, char const ch)
    {
        saga::string_view const sv1(str1);
        saga::string_view const sv2(str2);

        auto const num = saga_test::random_position_of(str2);

        static_assert(noexcept(sv1.find_last_of(sv2)), "");
        static_assert(noexcept(sv1.find_last_of(sv2, pos)), "");
        static_assert(noexcept(sv1.find_last_of(str2.c_str())), "");
        static_assert(noexcept(sv1.find_last_of(str2.c_str(), pos)), "");
        static_assert(noexcept(sv1.find_last_of(str2.c_str(), pos, num)), "");
        static_assert(noexcept(sv1.find_last_of(ch)), "");
        static_assert(noexcept(sv1.find_last_of(ch, pos)), "");

        CAPTURE(str1, str2, pos, num, ch);

        REQUIRE(sv1.find_last_of(sv1) == str1.find_last_of(str1));
        REQUIRE(sv1.find_last_of(sv2) == str1.find_last_of(str2));
        REQUIRE(sv2.find_last_of(sv1) == str2.find_last_of(str1));

        REQUIRE(sv1.find_last_of(sv2, pos) == str1.find_last_of(str2, pos));
        REQUIRE(sv2.find_last_of(sv1, pos) == str2.find_last_of(str1, pos));

        REQUIRE(sv1.find_last_of(str2.c_str())
                == sv1.find_last_of(saga::string_view(str2.c_str())));
        REQUIRE(sv1.find_last_of(str2.c_str(), pos)
                == sv1.find_last_of(saga::string_view(str2.c_str()), pos));
        REQUIRE(sv1.find_last_of(str2.c_str(), pos, num)
                == sv1.find_last_of(saga::string_view(str2.c_str(), num), pos));
        REQUIRE(sv1.find_last_of(ch) == sv1.find_last_of(saga::string_view(&ch, 1)));
        REQUIRE(sv1.find_last_of(ch, pos) == sv1.find_last_of(saga::string_view(&ch, 1), pos));
    };
}

TEST_CASE("string_view : find_last_of regression")
{
    std::string const str1(" ");
    std::string const str2(" ");
    auto const pos = std::size_t{0};

    saga::string_view const sv1(str1);
    saga::string_view const sv2(str2);

    CAPTURE(str1, str2, pos);

    REQUIRE(sv1.find_last_of(sv2, pos) == str1.find_last_of(str2, pos));
}

TEST_CASE("string_view : find_first_not_of")
{
    saga_test::property_checker << [](std::string const & str1, std::string const & str2,
                                      std::size_t const pos, char const ch)
    {
        saga::string_view const sv1(str1);
        saga::string_view const sv2(str2);

        auto const num = saga_test::random_position_of(str2);

        static_assert(noexcept(sv1.find_first_not_of(sv2)), "");
        static_assert(noexcept(sv1.find_first_not_of(sv2, pos)), "");
        static_assert(noexcept(sv1.find_first_not_of(str2.c_str())), "");
        static_assert(noexcept(sv1.find_first_not_of(str2.c_str(), pos)), "");
        static_assert(noexcept(sv1.find_first_not_of(str2.c_str(), pos, num)), "");
        static_assert(noexcept(sv1.find_first_not_of(ch)), "");
        static_assert(noexcept(sv1.find_first_not_of(ch, pos)), "");

        CAPTURE(str1, str2, pos, num, ch);

        REQUIRE(sv1.find_first_not_of(sv1) == str1.find_first_not_of(str1));
        REQUIRE(sv1.find_first_not_of(sv2) == str1.find_first_not_of(str2));
        REQUIRE(sv2.find_first_not_of(sv1) == str2.find_first_not_of(str1));

        REQUIRE(sv1.find_first_not_of(sv2, pos) == str1.find_first_not_of(str2, pos));
        REQUIRE(sv2.find_first_not_of(sv1, pos) == str2.find_first_not_of(str1, pos));

        REQUIRE(sv1.find_first_not_of(str2.c_str())
                == sv1.find_first_not_of(saga::string_view(str2.c_str())));
        REQUIRE(sv1.find_first_not_of(str2.c_str(), pos)
                == sv1.find_first_not_of(saga::string_view(str2.c_str()), pos));
        REQUIRE(sv1.find_first_not_of(str2.c_str(), pos, num)
                == sv1.find_first_not_of(saga::string_view(str2.c_str(), num), pos));
        REQUIRE(sv1.find_first_not_of(ch) == sv1.find_first_not_of(saga::string_view(&ch, 1)));
        REQUIRE(sv1.find_first_not_of(ch, pos) == sv1.find_first_not_of(saga::string_view(&ch, 1), pos));
    };
}

TEST_CASE("string_view : find_last_not_of")
{
    saga_test::property_checker << [](std::string const & str1, std::string const & str2,
                                      std::size_t const pos, char const ch)
    {
        saga::string_view const sv1(str1);
        saga::string_view const sv2(str2);

        auto const num = saga_test::random_position_of(str2);

        static_assert(noexcept(sv1.find_last_not_of(sv2)), "");
        static_assert(noexcept(sv1.find_last_not_of(sv2, pos)), "");
        static_assert(noexcept(sv1.find_last_not_of(str2.c_str())), "");
        static_assert(noexcept(sv1.find_last_not_of(str2.c_str(), pos)), "");
        static_assert(noexcept(sv1.find_last_not_of(str2.c_str(), pos, num)), "");
        static_assert(noexcept(sv1.find_last_not_of(ch)), "");
        static_assert(noexcept(sv1.find_last_not_of(ch, pos)), "");

        CAPTURE(str1, str2, pos, num, ch);

        REQUIRE(sv1.find_last_not_of(sv1) == str1.find_last_not_of(str1));
        REQUIRE(sv1.find_last_not_of(sv2) == str1.find_last_not_of(str2));
        REQUIRE(sv2.find_last_not_of(sv1) == str2.find_last_not_of(str1));

        REQUIRE(sv1.find_last_not_of(sv2, pos) == str1.find_last_not_of(str2, pos));
        REQUIRE(sv2.find_last_not_of(sv1, pos) == str2.find_last_not_of(str1, pos));

        REQUIRE(sv1.find_last_not_of(str2.c_str())
                == sv1.find_last_not_of(saga::string_view(str2.c_str())));
        REQUIRE(sv1.find_last_not_of(str2.c_str(), pos)
                == sv1.find_last_not_of(saga::string_view(str2.c_str()), pos));
        REQUIRE(sv1.find_last_not_of(str2.c_str(), pos, num)
                == sv1.find_last_not_of(saga::string_view(str2.c_str(), num), pos));
        REQUIRE(sv1.find_last_not_of(ch) == sv1.find_last_not_of(saga::string_view(&ch, 1)));
        REQUIRE(sv1.find_last_not_of(ch, pos) == sv1.find_last_not_of(saga::string_view(&ch, 1), pos));
    };
}

TEST_CASE("string_view : non-member comparison functions")
{
    saga_test::property_checker << [](std::string const & str1, std::string const & str2)
    {
        saga::string_view const sv1(str1);
        saga::string_view const sv2(str2);

        // Равно
        static_assert(noexcept(sv1 == sv2), "");

        REQUIRE(sv1 == sv1);
        REQUIRE(sv2 == sv2);

        REQUIRE((sv1 == sv2) == (str1 == str2));
        REQUIRE((sv1 == sv2) == (sv1.compare(sv2) == 0));

        REQUIRE((str1 == sv2) == (sv1 == sv2));
        REQUIRE((sv1 == str2) == (sv1 == sv2));

        // Не равно
        static_assert(noexcept(sv1 != sv2), "");

        REQUIRE((sv1 != sv1) == false);

        REQUIRE((sv1 != sv2) == !(sv1 == sv2));
        REQUIRE((str1 != sv2) == !(sv1 == sv2));
        REQUIRE((sv1 != str2) == !(sv1 == sv2));

        // Меньше
        static_assert(noexcept(sv1 < sv2), "");

        REQUIRE((sv1 < sv2) == (sv1.compare(sv2) < 0));
        REQUIRE((sv1 < str2) == (sv1 < sv2));
        REQUIRE((str1 < sv2) == (sv1 < sv2));

        // Больше
        static_assert(noexcept(sv1 > sv2), "");

        REQUIRE((sv1 > sv2) == (sv1.compare(sv2) > 0));
        REQUIRE((sv1 > str2) == (sv1 > sv2));
        REQUIRE((str1 > sv2) == (sv1 > sv2));

        // Меньше или равно
        static_assert(noexcept(sv1 <= sv2), "");

        REQUIRE(sv1 <= sv1);

        REQUIRE((sv1 <= sv2) == (sv1.compare(sv2) <= 0));
        REQUIRE((sv1 <= str2) == (sv1 <= sv2));
        REQUIRE((str1 <= sv2) == (sv1 <= sv2));

        // Больше или равно
        static_assert(noexcept(sv1 >= sv2), "");

        REQUIRE(sv1 >= sv1);

        REQUIRE((sv1 >= sv2) == (sv1.compare(sv2) >= 0));
        REQUIRE((sv1 >= str2) == (sv1 >= sv2));
        REQUIRE((str1 >= sv2) == (sv1 >= sv2));
    };
}

namespace
{
    // Отрицательные значения 'width' означают выравнивание по правому краю.
    template <typename T>
    std::string write_padded(const T& s, int width, char fill = 0)
    {
        std::ostringstream oss;
        if (fill != 0)
        {
            oss << std::setfill(fill);
        }
        if (width < 0)
        {
            width = -width;
            oss << std::right;
        }
        oss << std::setw(width) << s;
        return oss.str();
    }
}

TEST_CASE("string_view : ostreaming")
{
    saga_test::property_checker << [](std::string const & str)
    {
        saga::string_view const sv(str);

        std::ostringstream oss;

        static_assert(std::is_same<decltype(oss << sv), std::ostream &>{}, "");

        std::ostream & result = (oss << sv);

        REQUIRE(std::addressof(result) == std::addressof(oss));
        REQUIRE(oss.str() == str);
    };
}

TEST_CASE("string_view : ostreaming, padding")
{
    saga_test::property_checker << [](std::string const & str)
    {
        saga::string_view const sv(str);

        for (int w = -64; w < 64; ++w)
        {
            REQUIRE(::write_padded(str, w) == ::write_padded(sv, w));
            REQUIRE(::write_padded(str, w, '#') == ::write_padded(sv, w, '#'));
        }
    };
}

TEST_CASE("string_view : ostreaming, reset width after write")
{
    saga_test::property_checker << [](std::string const & str)
    {
        saga::string_view const sv(str);

        std::ostringstream oss;
        oss << "[" << std::setfill('#') << std::setw(5) << str << "]";

        std::ostringstream oss_v;
        oss_v << "[" << std::setfill('#') << std::setw(5) << sv << "]";

        REQUIRE(oss.str() == oss_v.str());
    };
}

TEST_CASE("string_view : hash")
{
    saga_test::property_checker << [](std::string const & str)
    {
        saga::string_view const sv(str);

        static_assert(noexcept(std::hash<saga::string_view>{}(sv)), "");

        REQUIRE(std::hash<saga::string_view>{}(sv) == std::hash<std::string>{}(str));
    };
}

TEST_CASE("string_view: char literatls")
{
    using namespace ::saga::literals::string_view_literals;

    static_assert(noexcept("sample string_view literal"_sv), "");

    constexpr saga::string_view const actual = "sample string_view literal"_sv;
    saga::string_view const expected("sample string_view literal");

    REQUIRE(actual == expected);
}

TEST_CASE("string_view: char16_t literatls")
{
    using namespace ::saga::literals::string_view_literals;

    static_assert(noexcept(u"sample string_view literal"_sv), "");

    constexpr saga::u16string_view const actual = u"sample string_view literal"_sv;
    saga::u16string_view const expected(u"sample string_view literal");

    REQUIRE(actual == expected);
}

TEST_CASE("string_view: char32_t literatls")
{
    using namespace ::saga::literals::string_view_literals;

    static_assert(noexcept(U"sample string_view literal"_sv), "");

    constexpr saga::u32string_view const actual = U"sample string_view literal"_sv;
    saga::u32string_view const expected(U"sample string_view literal");

    REQUIRE(actual == expected);
}

TEST_CASE("string_view: wchar_t literatls")
{
    using namespace ::saga::literals::string_view_literals;

    static_assert(noexcept(L"sample string_view literal"_sv), "");

    constexpr saga::wstring_view const actual = L"sample string_view literal"_sv;
    saga::wstring_view const expected(L"sample string_view literal");

    REQUIRE(actual == expected);
}
