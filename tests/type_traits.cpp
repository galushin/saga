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

// is_equality_comparable
static_assert(saga::is_equality_comparable<int>{}, "Must be true");
static_assert(saga::is_equality_comparable<std::string>{}, "Must be true");
static_assert(!saga::is_equality_comparable<std::greater<int>>{}, "Must be false");
