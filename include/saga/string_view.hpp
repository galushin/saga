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

#ifndef Z_SAGA_STRING_VIEW_HPP_INCLUDED
#define Z_SAGA_STRING_VIEW_HPP_INCLUDED

/** @file string_view.hpp
 @brief Классы, объекты которых могут ссылаться на константную непрерывную последовательность
 символьно-подобных объектов, первый элемент которой имеет индекс ноль.

 Эти классы можно использовать как нешаблонные параметры там, где требуется последовательность
 символов.

 @todo Совместимость с std::basic_string
 @todo Совместимость с std::basic_string_view (там где, он есть)
 @todo Обобщить тесты на u16string_view, u32string_view, wstring_view и нестандартные характеристики
*/

#include <string>

namespace saga
{
    /** @brief Шаблон класса, объекты которых могут ссылаться на константную непрерывную
    последовательность символьно-подобных объектов, первый элемент которой имеет индекс ноль.

    Сложность операций является константной, если явно не указано иное.
    */
    template <class charT, class traits = std::char_traits<charT>>
    class basic_string_view
    {
    public:
        // Типы
        using traits_type = traits;
        using value_type = charT;
        using pointer = value_type *;
        using const_pointer = value_type const *;
        using reference = value_type &;
        using const_reference = value_type const &;
        using const_iterator = const_pointer;
        using iterator = const_iterator;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        using reverse_iterator = const_reverse_iterator;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        static constexpr size_type npos = size_type(-1);

        // Конструирование и присваивание
        /** @brief Конструктор без аргументов
        @post <tt>this->data() == nullptr</tt>
        @post <tt>this->size() == 0</tt>
        */
        constexpr basic_string_view() noexcept = default;

        /** @brief Конструктор на основе строки, заверщающейся нулевым символом
        @param str Указатель на начало строки, завершающейся нулевым символом
        @pre <tt>[str, str+traits::length(str))</tt> является корректным интервалом
        @post <tt>this->data() == str</tt>
        @post <tt>this->size() == traits::length(str)</tt>

        Сложность пропорциональна <tt>traits::length(str)</tt>
        */
        basic_string_view(charT const * str)
         : basic_string_view(str, traits_type::length(str))
        {}

        /** @brief Конструктор на основе строки, заверщающейся нулевым символом
        @param str указатель на массив символов
        @param len количество элементов
        @pre <tt>[str, str+len)</tt> является корректным интервалом
        @post <tt>this->data() == str</tt>
        @post <tt>this->size() == len</tt>
        */
        constexpr basic_string_view(charT const * str, size_type len)
         : data_(str)
         , size_(len)
        {}

        // Ёмкость
        constexpr size_type size() const
        {
            return this->size_;
        }

        constexpr bool empty() const
        {
            return true;
        }

        // Доступ к элементам
        constexpr const_pointer data() const
        {
            return this->data_;
        }

    private:
        const_pointer data_ = nullptr;
        size_type size_ = 0;
    };

    // Cинонимы для конкретных экземпляров basic_string_view
    using string_view = basic_string_view<char>;
    using u16string_view = basic_string_view<char16_t>;
    using u32string_view = basic_string_view<char32_t>;
    using wstring_view = basic_string_view<wchar_t>;
}
//namespace saga

#endif
// Z_SAGA_STRING_VIEW_HPP_INCLUDED
