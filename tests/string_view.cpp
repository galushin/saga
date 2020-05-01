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

// Тесты
using StringView = saga::basic_string_view<char, std::char_traits<char>>;

static_assert(std::is_same<saga::basic_string_view<char>,
                           saga::basic_string_view<char, std::char_traits<char>>>::value, "");

// @todo 24.4.3 Операторы сравнения
// @todo 24.4.4 Операторы вставки в поток и извлечения из потока

static_assert(std::is_same<saga::string_view, saga::basic_string_view<char>>::value, "");
static_assert(std::is_same<saga::u16string_view, saga::basic_string_view<char16_t>>::value, "");
static_assert(std::is_same<saga::u32string_view, saga::basic_string_view<char32_t>>::value, "");
static_assert(std::is_same<saga::wstring_view, saga::basic_string_view<wchar_t>>::value, "");

// @todo 24.4.5 Поддержка hash
// @todo 24.4.6 Литералы

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
                           std::reverse_iterator<saga::string_view::const_iterator>>{},"");
static_assert(std::is_same<saga::string_view::reverse_iterator,
                           saga::string_view::const_reverse_iterator>{}, "");
static_assert(std::is_same<saga::string_view::size_type, std::size_t>{}, "");
static_assert(std::is_same<saga::string_view::difference_type, std::ptrdiff_t>{}, "");
static_assert(saga::string_view::npos == saga::string_view::size_type(-1), "");

static_assert(std::is_nothrow_default_constructible<saga::string_view>{}, "");

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
    // @todo Должен быть constexpr (пока невозможно реализовать, так как char_traits не constexpr)

    saga_test::property_checker << [](std::string const & str)
    {
        char const * z_str = str.c_str();

        saga::string_view const sv(z_str);

        REQUIRE(sv.size() == saga::string_view::traits_type::length(z_str));
        REQUIRE(sv.data() == z_str);
    };
}

TEST_CASE("string_view : ctor from pointer and size")
{
    constexpr char const * z_str = "This is a test string";
    constexpr auto const num = 7;

    constexpr saga::string_view const sv(z_str, num);

    static_assert(sv.size() == num, "");
    static_assert(sv.data() == z_str, "");

    saga_test::property_checker << [](std::string const & str)
    {
        auto const num = saga_test::random_uniform(0*str.size(), str.size());

        saga::string_view const sv(str.c_str(), num);

        REQUIRE(sv.size() == num);
        REQUIRE(sv.data() == str.data());
    };
}

// @todo Конструктор копий
// @todo Оператор присваивания

// @todo Закончить раздел 24.4
