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

// Тестируемый заголовочный файл
#include <saga/type_traits.hpp>

// Тестовая инфраструктура

// Вспомогательные файлы
#include <functional>
#include <string>

// Тесты
// conditional_t
static_assert(std::is_same<saga::conditional_t<true, int, double>, int>{}, "");
static_assert(std::is_same<saga::conditional_t<false, int, double>, double>{}, "");

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

        friend void swap(struct_1 &, struct_2 &) noexcept(false);
        friend void swap(struct_2 &, struct_1 &) noexcept(false);
    };

    struct struct_3
    {
        friend void swap(struct_2 &, struct_3 &) noexcept(false);
        friend void swap(struct_3 &, struct_2 &) = delete;
    };

    struct struct_4
    {
        friend void swap(struct_1 &, struct_4 &) noexcept;
        friend void swap(struct_4 &, struct_1 &) noexcept;

        friend void swap(struct_2 &, struct_4 &) noexcept;
        friend void swap(struct_4 &, struct_2 &) noexcept(false);
    };



    struct rvalue_swap
    {
        friend void swap(rvalue_swap &, rvalue_swap &) = delete;
        friend void swap(rvalue_swap &&, rvalue_swap &&) noexcept(false);
    };

    struct rvalue_swap_noexcept
    {
        friend void swap(rvalue_swap_noexcept &, rvalue_swap_noexcept &) = delete;
        friend void swap(rvalue_swap_noexcept &&, rvalue_swap_noexcept &&) noexcept;
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
        void swap(T &, T &) noexcept;
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

// nonesuch
static_assert(!std::is_destructible<saga::nonesuch>{}, "");
static_assert(!std::is_default_constructible<saga::nonesuch>{}, "");
static_assert(!std::is_copy_constructible<saga::nonesuch>{}, "");
static_assert(!std::is_move_constructible<saga::nonesuch>{}, "");
static_assert(!std::is_copy_assignable<saga::nonesuch>{}, "");
static_assert(!std::is_move_assignable<saga::nonesuch>{}, "");

// @todo Проверить, что saga::nonesuch - не является аггрегатом

// detected_or
namespace
{
    template <class T>
    using get_size_type = typename T::size_type;
}

#include <memory>
#include <vector>

static_assert(std::is_same<std::false_type, saga::detected_or<void, ::get_size_type, long>::value_t>{}, "");
static_assert(std::is_same<void, saga::detected_or<void, ::get_size_type, long>::type>{}, "");

static_assert(std::is_same<std::false_type,
              saga::detected_or<std::size_t, ::get_size_type, std::unique_ptr<bool>>::value_t>{}, "");
static_assert(std::is_same<std::size_t, saga::detected_or<std::size_t, ::get_size_type, long>::type>{}, "");

static_assert(std::is_same<std::true_type, saga::detected_or<double, ::get_size_type, std::vector<int>>::value_t>{}, "");
static_assert(std::is_same<typename std::vector<int>::size_type,
                           saga::detected_or<double, ::get_size_type, std::vector<int>>::type>{}, "");

// is_detected
static_assert(!saga::is_detected<::get_size_type, long>{}, "");
static_assert(!saga::is_detected<::get_size_type, std::unique_ptr<bool>>{}, "");
static_assert(saga::is_detected<::get_size_type, std::vector<int>>{}, "");

// is_detected_v
static_assert(!saga::is_detected_v<::get_size_type, long>, "");
static_assert(!saga::is_detected_v<::get_size_type, std::unique_ptr<bool>>, "");
static_assert(saga::is_detected_v<::get_size_type, std::vector<int>>, "");

// detected_t
static_assert(std::is_same<saga::nonesuch, saga::detected_t<::get_size_type, double>>{}, "");
static_assert(std::is_same<saga::nonesuch, saga::detected_t<::get_size_type, std::unique_ptr<bool>>>{}, "");
static_assert(std::is_same<typename std::vector<int>::size_type,
                           saga::detected_t<::get_size_type, std::vector<int>>>{}, "");

// detected_or_t
static_assert(std::is_same<void, saga::detected_or_t<void, ::get_size_type, long>>{}, "");
static_assert(std::is_same<std::size_t, saga::detected_or_t<std::size_t, ::get_size_type, long>>{}, "");
static_assert(std::is_same<typename std::vector<int>::size_type,
                           saga::detected_or_t<double, ::get_size_type, std::vector<int>>>{}, "");

namespace
{
    template <class T>
    using diff_t = typename T::difference_type;

    struct no_diff
    {};

    struct int_diff
    {
        using difference_type = int;
    };

    struct long_diff
    {
        using difference_type = long;
    };

    struct void_diff
    {
        using difference_type = void;
    };
}

// is_detected_exact
static_assert(!saga::is_detected_exact<int, diff_t, long>{}, "");
static_assert(!saga::is_detected_exact<int, diff_t, no_diff>{}, "");
static_assert(!saga::is_detected_exact<int, diff_t, long_diff>{}, "");
static_assert(saga::is_detected_exact<int, diff_t, int_diff>{}, "");

// is_detected_exact_v
#if __cpp_variable_templates >= 201304
static_assert(!saga::is_detected_exact_v<int, diff_t, long>, "");
static_assert(!saga::is_detected_exact_v<int, diff_t, no_diff>, "");
static_assert(!saga::is_detected_exact_v<int, diff_t, long_diff>, "");
static_assert(saga::is_detected_exact_v<int, diff_t, int_diff>, "");
#endif
//__cpp_variable_templates

// is_detected_convertible
static_assert(!saga::is_detected_convertible<int, diff_t, long>{}, "");
static_assert(!saga::is_detected_convertible<int, diff_t, no_diff>{}, "");
static_assert(saga::is_detected_convertible<long, diff_t, long_diff>{}, "");
static_assert(saga::is_detected_convertible<int, diff_t, int_diff>{}, "");
static_assert(!saga::is_detected_convertible<int, diff_t, void_diff>{}, "");

// is_detected_convertible_v
#if __cpp_variable_templates >= 201304
static_assert(!saga::is_detected_convertible_v<int, diff_t, long>, "");
static_assert(!saga::is_detected_convertible_v<int, diff_t, no_diff>, "");
static_assert(saga::is_detected_convertible_v<long, diff_t, long_diff>, "");
static_assert(saga::is_detected_convertible_v<int, diff_t, int_diff>, "");
static_assert(!saga::is_detected_convertible_v<int, diff_t, void_diff>, "");
#endif
//__cpp_variable_templates

// is_specialization
static_assert(saga::is_specialization_of<std::vector<int>, std::vector>{}, "");
static_assert(!saga::is_specialization_of<std::unique_ptr<int>, std::vector>{}, "");
static_assert(!saga::is_specialization_of<int, std::vector>{}, "");
