/* (c) 2019 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_TYPE_TRAITS_HPP_INCLUDED
#define Z_SAGA_TYPE_TRAITS_HPP_INCLUDED

/** @file saga/type_traits.hpp
 @brief Характеристики-типов
*/

#include <utility>

namespace saga
{
    /** @brief Тип-тэг для задания приоритета перегрузок
    @tparam P уровень приоритета: чем больше это значение, тем выше приоритет
    */
    template <std::size_t P>
    struct priority_tag
     : priority_tag<P-1>
    {};

    /// @brief Специализация для наименьшего приоритета
    template <>
    struct priority_tag<0>
    {};


    /** @brief Определение типа для предсставления размера, используемого данным типом, например,
    контейнером

    Определяет вложенный тип @c type, определённый по следующему правилу.
    Если у @c T определён вложенный тип @c size_type, то <tt>type = typename T::size_type</tt>.
    Иначе @c type совпадает с типом, возвращаемым функцией-членом @c size типа @c T

    @tparam T тип (предположительно, тип контейнера)
    */
    template <class T>
    struct size_type
    {
    private:
        template <class U>
        static auto impl(priority_tag<0>) -> decltype(std::declval<U>().size());

        template <class U>
        static auto impl(priority_tag<1>) -> typename U::size_type;

    public:
        using type = decltype(size_type::impl<T>(priority_tag<1>{}));
    };
}
// namespace saga

#endif
// Z_SAGA_TYPE_TRAITS_HPP_INCLUDED
