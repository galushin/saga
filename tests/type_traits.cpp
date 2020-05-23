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

// Тестируемый заголовочный файл
#include <saga/type_traits.hpp>

// Тестовая инфраструктура

// Вспомогательные файлы
#include <functional>
#include <string>

// Тесты
// remove_cvref
namespace
{
    template <class Type>
    constexpr bool check_remove_cvref_for()
    {
        static_assert(std::is_same<typename saga::remove_cvref<Type>::type, Type>{}, "");
        static_assert(std::is_same<typename saga::remove_cvref<Type const>::type, Type>{}, "");
        static_assert(std::is_same<typename saga::remove_cvref<Type volatile>::type, Type>{}, "");
        static_assert(std::is_same<typename saga::remove_cvref<Type const volatile>::type, Type>{}, "");
        static_assert(std::is_same<typename saga::remove_cvref<Type &>::type, Type>{}, "");
        static_assert(std::is_same<typename saga::remove_cvref<Type const &>::type, Type>{}, "");
        static_assert(std::is_same<typename saga::remove_cvref<Type volatile &>::type, Type>{}, "");
        static_assert(std::is_same<typename saga::remove_cvref<Type const volatile &>::type, Type>{}, "");
        static_assert(std::is_same<typename saga::remove_cvref<Type &&>::type, Type>{}, "");
        static_assert(std::is_same<typename saga::remove_cvref<Type const &&>::type, Type>{}, "");
        static_assert(std::is_same<typename saga::remove_cvref<Type volatile &&>::type, Type>{}, "");
        static_assert(std::is_same<typename saga::remove_cvref<Type const volatile &&>::type, Type>{}, "");

        return true;
    }
}

static_assert(check_remove_cvref_for<std::string>(), "");
static_assert(check_remove_cvref_for<int[2]>(), "");
static_assert(check_remove_cvref_for<double(std::string)>(), "");

// remove_cvref_t
namespace
{
    template <class Type>
    constexpr bool check_remove_cvref_t_for()
    {
        static_assert(std::is_same<saga::remove_cvref_t<Type>, Type>{}, "");
        static_assert(std::is_same<saga::remove_cvref_t<Type const>, Type>{}, "");
        static_assert(std::is_same<saga::remove_cvref_t<Type volatile>, Type>{}, "");
        static_assert(std::is_same<saga::remove_cvref_t<Type const volatile>, Type>{}, "");
        static_assert(std::is_same<saga::remove_cvref_t<Type &>, Type>{}, "");
        static_assert(std::is_same<saga::remove_cvref_t<Type const &>, Type>{}, "");
        static_assert(std::is_same<saga::remove_cvref_t<Type volatile &>, Type>{}, "");
        static_assert(std::is_same<saga::remove_cvref_t<Type const volatile &>, Type>{}, "");
        static_assert(std::is_same<saga::remove_cvref_t<Type &&>, Type>{}, "");
        static_assert(std::is_same<saga::remove_cvref_t<Type const &&>, Type>{}, "");
        static_assert(std::is_same<saga::remove_cvref_t<Type volatile &&>, Type>{}, "");
        static_assert(std::is_same<saga::remove_cvref_t<Type const volatile &&>, Type>{}, "");

        return true;
    }
}

static_assert(check_remove_cvref_t_for<std::string>(), "");
static_assert(check_remove_cvref_t_for<int[2]>(), "");
static_assert(check_remove_cvref_t_for<double(std::string)>(), "");

// семейство is_swappable
namespace
{
    struct struct_1
    {
        struct_1(struct_1 const &) = delete;
        struct_1(struct_1 &&) = delete;
        struct_1 & operator=(struct_1 const &) = delete;
        struct_1 & operator=(struct_1 &&) = delete;

        friend void swap(struct_1 &, struct_1 &) noexcept
        {}
    };

    struct struct_2
    {
        struct_2(struct_2 const &) = delete;
        struct_2(struct_2 &&) = delete;
        struct_2 & operator=(struct_2 const &) = delete;
        struct_2 & operator=(struct_2 &&) = delete;

        friend void swap(struct_2 &, struct_2 &) noexcept(false)
        {}
    };

    struct struct_3 { int value; };
    struct struct_4 {};

    void swap(struct_1 &, struct_2 &) noexcept(false) {}
    void swap(struct_2 &, struct_1 &) noexcept(false) {}

    void swap(struct_2 &, struct_3 &) noexcept(false) {}
    void swap(struct_3 &, struct_2 &) = delete;

    void swap(struct_1 &, struct_4 &) noexcept {};
    void swap(struct_4 &, struct_1 &) noexcept {};

    void swap(struct_2 &, struct_4 &) noexcept {};
    void swap(struct_4 &, struct_2 &) noexcept(false) {};

    struct rvalue_swap
    {
        friend void swap(rvalue_swap &, rvalue_swap &) = delete;
        friend void swap(rvalue_swap &&, rvalue_swap &&) noexcept(false) {}
    };

    struct rvalue_swap_noexcept
    {
        friend void swap(rvalue_swap_noexcept &, rvalue_swap_noexcept &) = delete;
        friend void swap(rvalue_swap_noexcept &&, rvalue_swap_noexcept &&) noexcept {}
    };

    struct deleted_swap
    {
        friend void swap(deleted_swap &, deleted_swap &) noexcept = delete;
    };

    namespace ambiguous_swap_ns
    {
        struct ambiguous_swap
        {};

        template <class T>
        void swap(T &, T &) noexcept
        {}
    }
}

// is_swappable_with не добавляет lvalue-ссылку
static_assert(!saga::is_swappable_with<int, int>::value, "");
static_assert(saga::is_swappable_with<int&, int&>::value, "");
static_assert(saga::is_swappable_with<std::string &, std::string &>::value, "");

static_assert(saga::is_swappable_with<rvalue_swap, rvalue_swap>::value, "");
static_assert(saga::is_swappable_with<rvalue_swap &&, rvalue_swap &&>::value, "");
static_assert(!saga::is_swappable_with<rvalue_swap &, rvalue_swap &>::value, "");

// is_swappable_with правильно обрабатывает гетерогенный обмен
static_assert(saga::is_swappable_with<struct_1&, struct_2&>::value, "");
static_assert(!saga::is_swappable_with<struct_1&, struct_3&>::value, "");
static_assert(saga::is_swappable_with<struct_2&, struct_1&>::value, "");
static_assert(!saga::is_swappable_with<struct_2&, struct_3&>::value, "");
static_assert(!saga::is_swappable_with<struct_3&, struct_1&>::value, "");
static_assert(!saga::is_swappable_with<struct_3&, struct_2&>::value, "");

// is_swappable_with не удаляет cv-квалификатор
static_assert(!saga::is_swappable_with<struct_1 const &, struct_2 &>::value, "");
static_assert(!saga::is_swappable_with<struct_1 &, struct_2 const &>::value, "");

// is_swappable_with защищён он void
static_assert(!saga::is_swappable_with<void, int>{}, "");
static_assert(!saga::is_swappable_with<int, void>{}, "");
static_assert(!saga::is_swappable_with<void const, void const volatile>{}, "");

// is_swappable
// is_swappable для типов, на которые нельзя сформировать ссылки
static_assert(!saga::is_swappable<void>{}, "");
static_assert(!saga::is_swappable<void const>{}, "");
static_assert(!saga::is_swappable<void volatile>{}, "");
static_assert(!saga::is_swappable<void const volatile>{}, "");
static_assert(!saga::is_swappable<int() const>{}, "");
static_assert(!saga::is_swappable<int() volatile>{}, "");
static_assert(!saga::is_swappable<int() const volatile>{}, "");
static_assert(!saga::is_swappable<int() &>{}, "");
static_assert(!saga::is_swappable<int() &&>{}, "");
static_assert(!saga::is_swappable<int() const &>{}, "");
static_assert(!saga::is_swappable<int() const &&>{}, "");
static_assert(!saga::is_swappable<int() volatile &>{}, "");
static_assert(!saga::is_swappable<int() volatile &&>{}, "");
static_assert(!saga::is_swappable<int() const &>{}, "");
static_assert(!saga::is_swappable<int() const &&>{}, "");

// is_swappable добавляет lvalue-ссылку
static_assert(saga::is_swappable<int>{}, "");
static_assert(saga::is_swappable<int &>{}, "");
static_assert(saga::is_swappable<struct_1>{}, "");
static_assert(saga::is_swappable<struct_2>{}, "");
static_assert(!saga::is_swappable<rvalue_swap>{}, "");
static_assert(!saga::is_swappable<rvalue_swap &&>{}, "");

// is_swappable не удаляет cv-квалификатор
static_assert(saga::is_swappable<int const &>{}, "");

// is_swappable - удалённый или неоднозначный swap
static_assert(!saga::is_swappable<deleted_swap>{}, "");
static_assert(!saga::is_swappable<ambiguous_swap_ns::ambiguous_swap>{}, "");

// is_nothrow_swappable_with
// is_nothrow_swappable_with не добавляет lvalue-ссылку
static_assert(!saga::is_nothrow_swappable_with<int, int>::value, "");
static_assert(!saga::is_nothrow_swappable_with<struct_1, struct_1>::value, "");
static_assert(saga::is_nothrow_swappable_with<struct_1 &, struct_1 &>::value, "");

static_assert(!saga::is_nothrow_swappable_with<rvalue_swap, rvalue_swap>::value, "");
static_assert(!saga::is_nothrow_swappable_with<rvalue_swap &&, rvalue_swap &&>::value, "");
static_assert(!saga::is_nothrow_swappable_with<rvalue_swap &, rvalue_swap &>::value, "");

static_assert(saga::is_nothrow_swappable_with<rvalue_swap_noexcept, rvalue_swap_noexcept>::value, "");
static_assert(saga::is_nothrow_swappable_with<rvalue_swap_noexcept &&, rvalue_swap_noexcept &&>::value, "");
static_assert(!saga::is_nothrow_swappable_with<rvalue_swap_noexcept &, rvalue_swap_noexcept &>::value, "");

// is_nothrow_swappable_with правильно обрабатывает гетерогенный обмен
static_assert(saga::is_swappable_with<struct_1&, struct_2&>::value, "");
static_assert(!saga::is_nothrow_swappable_with<struct_1&, struct_2&>::value, "");

static_assert(!saga::is_nothrow_swappable_with<struct_1&, struct_3&>::value, "");
static_assert(!saga::is_nothrow_swappable_with<struct_2&, struct_1&>::value, "");
static_assert(!saga::is_nothrow_swappable_with<struct_2&, struct_3&>::value, "");
static_assert(!saga::is_nothrow_swappable_with<struct_3&, struct_1&>::value, "");
static_assert(!saga::is_nothrow_swappable_with<struct_3&, struct_2&>::value, "");
static_assert(!saga::is_nothrow_swappable_with<struct_2&, struct_4&>::value, "");
static_assert(!saga::is_nothrow_swappable_with<struct_4&, struct_2&>::value, "");

static_assert(saga::is_nothrow_swappable_with<struct_1&, struct_4&>::value, "");
static_assert(saga::is_nothrow_swappable_with<struct_4&, struct_1&>::value, "");

// is_nothrow_swappable_with не удаляет cv-квалификатор
static_assert(!saga::is_nothrow_swappable_with<struct_1 const&, struct_4&>::value, "");
static_assert(!saga::is_nothrow_swappable_with<struct_1&, struct_4 const&>::value, "");

// is_nothrow_swappable_with защищён он void
static_assert(!saga::is_nothrow_swappable_with<void, int>{}, "");
static_assert(!saga::is_nothrow_swappable_with<int, void>{}, "");
static_assert(!saga::is_nothrow_swappable_with<void const, void const volatile>{}, "");

// is_nothrow_swappable
// is_nothrow_swappable для типов, на которые нельзя сформировать ссылки
static_assert(!saga::is_nothrow_swappable<void>{}, "");
static_assert(!saga::is_nothrow_swappable<void const>{}, "");
static_assert(!saga::is_nothrow_swappable<void volatile>{}, "");
static_assert(!saga::is_nothrow_swappable<void const volatile>{}, "");
static_assert(!saga::is_nothrow_swappable<int() const>{}, "");
static_assert(!saga::is_nothrow_swappable<int() volatile>{}, "");
static_assert(!saga::is_nothrow_swappable<int() const volatile>{}, "");
static_assert(!saga::is_nothrow_swappable<int() &>{}, "");
static_assert(!saga::is_nothrow_swappable<int() &&>{}, "");
static_assert(!saga::is_nothrow_swappable<int() const &>{}, "");
static_assert(!saga::is_nothrow_swappable<int() const &&>{}, "");
static_assert(!saga::is_nothrow_swappable<int() volatile &>{}, "");
static_assert(!saga::is_nothrow_swappable<int() volatile &&>{}, "");
static_assert(!saga::is_nothrow_swappable<int() const &>{}, "");
static_assert(!saga::is_nothrow_swappable<int() const &&>{}, "");

// is_nothrow_swappable добавляет lvalue-ссылку
static_assert(saga::is_swappable<struct_1>{}, "");
static_assert(saga::is_nothrow_swappable<struct_1>{}, "");

static_assert(saga::is_swappable<struct_2>{}, "");
static_assert(!saga::is_nothrow_swappable<struct_2>{}, "");

static_assert(!saga::is_nothrow_swappable<rvalue_swap_noexcept>{}, "");
static_assert(!saga::is_nothrow_swappable<rvalue_swap_noexcept &&>{}, "");

// is_nothrow_swappable не удаляет cv-квалификатор
static_assert(!saga::is_nothrow_swappable<struct_1 const>{}, "");

// is_swappable - удалённый или неоднозначный swap
static_assert(!saga::is_nothrow_swappable<deleted_swap>{}, "");
static_assert(!saga::is_nothrow_swappable<ambiguous_swap_ns::ambiguous_swap>{}, "");

// is_equality_comparable
static_assert(saga::is_equality_comparable<int>{}, "Must be true");
static_assert(saga::is_equality_comparable<std::string>{}, "Must be true");
static_assert(!saga::is_equality_comparable<std::greater<int>>{}, "Must be false");
