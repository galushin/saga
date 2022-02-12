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

// invoker_result
namespace
{
    struct Base
    {
        int var;

        int size();
    };

    struct Derived : Base{};
}

// 1. Обработка случая: указатель на функцию-член
static_assert(std::is_same<saga::invoke_result_t<int (Base::*)(), Derived&>
                          , int>{}, "");
static_assert(std::is_same<saga::invoke_result_t<int const & (Base::*)() const, Derived &>
                          , int const&>{}, "");
static_assert(std::is_same<saga::invoke_result_t<int && (Base::*)() &&, Derived&&>
                          , int &&>{}, "");
static_assert(std::is_same<saga::invoke_result_t<int const && (Base::*)() const &&, Derived const &&>
                          , int const &&>{}, "");

// 1.2 Аргумент - специализация std::reference_wrapper
static_assert(std::is_same<saga::invoke_result_t<int (Base::*)(), std::reference_wrapper<Derived>>
                          , int>{}, "");
static_assert(std::is_same<saga::invoke_result_t<int & (Base::*)()
                                                , std::reference_wrapper<Derived> const>
                          , int &>{}, "");
static_assert(std::is_same<saga::invoke_result_t<int const& (Base::*)() const
                                                , std::reference_wrapper<Derived const>>
                          , int const &>{}, "");
static_assert(std::is_same<saga::invoke_result_t<int const & (Base::*)() const
                                                ,std::reference_wrapper<Derived const> const &>
                          , int const &>{}, "");

// 1.3 Остальное (считаем, что аргумент - указатель или умный указатель)
static_assert(std::is_same<saga::invoke_result_t<int & (Base::*)(double), Derived*, double>
                          , int &>{}, "");
static_assert(std::is_same<saga::invoke_result_t<int const & (Base::*)(double) const
                                                , Derived const*, double>
                          , int const&>{}, "");
static_assert(std::is_same<saga::invoke_result_t<int & (Base::*)(double),
                                                 std::unique_ptr<Derived>, double>
                          , int&>{}, "");
static_assert(std::is_same<saga::invoke_result_t<int const & (Base::*)(double) const
                                                , std::unique_ptr<Derived const>, double>
                          , int const&>{}, "");

// 2. Обработка случая: указатель на переменную-член
// 2.1 Аргумент - ссылка
static_assert(std::is_same<saga::invoke_result_t<int Base::*, Derived&>
                          , int&>{}, "");
static_assert(std::is_same<saga::invoke_result_t<int Base::*, Derived const &>
                          , int const&>{}, "");
static_assert(std::is_same<saga::invoke_result_t<int Base::*, Derived&&>
                          , int &&>{}, "");
static_assert(std::is_same<saga::invoke_result_t<int Base::*, Derived const &&>
                          , int const &&>{}, "");

// 2.2 Аргумент - специализация std::reference_wrapper
static_assert(std::is_same<saga::invoke_result_t<int Base::*, std::reference_wrapper<Derived>&>
                          , int&>{}, "");
static_assert(std::is_same<saga::invoke_result_t<int Base::*, std::reference_wrapper<Derived> const &>
                          , int&>{}, "");
static_assert(std::is_same<saga::invoke_result_t<int Base::*, std::reference_wrapper<Derived const> &>
                          , int const &>{}, "");
static_assert(std::is_same<saga::invoke_result_t<int Base::*, std::reference_wrapper<Derived const> const &>
                          , int const &>{}, "");

// 2.3 Остальное (считаем, что аргумент - указатель или умный указатель)
static_assert(std::is_same<saga::invoke_result_t<int Base::*, Derived*>
                          , int&>{}, "");
static_assert(std::is_same<saga::invoke_result_t<int Base::*, Derived const *>
                          , int const&>{}, "");
static_assert(std::is_same<saga::invoke_result_t<int Base::*, std::unique_ptr<Derived>>
                          , int&>{}, "");
static_assert(std::is_same<saga::invoke_result_t<int Base::*, std::unique_ptr<Derived const>>
                          , int const&>{}, "");

// 3. Обработка случая: остальное (функциональный объект)
namespace
{
    struct Functor
    {
        double operator()(char, int&);
        float operator()(int);
    };
}

static_assert(std::is_same<saga::invoke_result_t<Functor, int>, float>{}, "");
static_assert(std::is_same<saga::invoke_result_t<Functor, char, int &>, double>{}, "");

static_assert(std::is_same<saga::invoke_result_t<void(double, char), double, char>, void>{}, "");
