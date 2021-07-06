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

#ifndef Z_SAGA_STRING_VIEW_HPP_INCLUDED
#define Z_SAGA_STRING_VIEW_HPP_INCLUDED

/** @file string_view.hpp
 @brief Классы, объекты которых могут ссылаться на константную непрерывную последовательность
 символьно-подобных объектов, первый элемент которой имеет индекс ноль.

 Эти классы можно использовать как нешаблонные параметры там, где требуется последовательность
 символов.
 Реализация основана на www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/n4727.pdf
*/

#include <saga/iterator/reverse.hpp>
#include <saga/utility/operators.hpp>

#include <cassert>

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>

namespace saga
{
    /** @brief Шаблон класса, объекты которых могут ссылаться на константную непрерывную
    последовательность символьно-подобных объектов, первый элемент которой имеет индекс ноль.

    Сложность операций является константной, если явно не указано иное.
    */
    template <class charT, class traits = std::char_traits<charT>>
    class basic_string_view
     : saga::operators::totally_ordered<basic_string_view<charT, traits>>
    {
        // Функции сравнения
        friend bool operator==(basic_string_view lhs, basic_string_view rhs) noexcept
        {
            return lhs.compare(rhs) == 0;
        }

        friend bool operator<(basic_string_view lhs, basic_string_view rhs) noexcept
        {
            return lhs.compare(rhs) < 0;
        }

        // Вставка в поток
        friend std::basic_ostream<charT, traits> &
        operator<<(std::basic_ostream<charT, traits> & os, basic_string_view sv)
        {
            typename std::basic_ostream<charT, traits>::sentry sentry(os);

            if(sentry)
            {
                size_type l_pad = 0;
                size_type r_pad = 0;

                if(static_cast<size_type>(os.width()) > sv.size())
                {
                    auto const pad = os.width() - sv.size();

                    if((os.flags() & os.adjustfield) == os.left)
                    {
                        r_pad = pad;
                    }
                    else
                    {
                        l_pad = pad;
                    }
                }

                for(; l_pad > 0; -- l_pad)
                {
                    os.rdbuf()->sputc(os.fill());
                }

                os.rdbuf()->sputn(sv.data(), sv.size());

                for(; r_pad > 0; -- r_pad)
                {
                    os.rdbuf()->sputc(os.fill());
                }

                os.width(0);
            }

            return os;
        }

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
        using const_reverse_iterator = saga::reverse_iterator<const_iterator>;
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

        constexpr basic_string_view(basic_string_view const & ) noexcept = default;
        constexpr basic_string_view & operator=(basic_string_view const & ) noexcept = default;

        /** @brief Конструктор на основе строки, заверщающейся нулевым символом
        @param str Указатель на начало строки, завершающейся нулевым символом
        @pre <tt>[str, str+traits::length(str))</tt> является корректным интервалом
        @post <tt>this->data() == str</tt>
        @post <tt>this->size() == traits::length(str)</tt>

        Сложность пропорциональна <tt>traits::length(str)</tt>
        */
        basic_string_view(charT const * str)
         : basic_string_view(str, str == nullptr ? 0 : traits_type::length(str))
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

        template <class Allocator>
        basic_string_view(std::basic_string<charT, traits_type, Allocator> const & str) noexcept
         : basic_string_view(str.data(), str.size())
        {}

        // Неявное преобразование в строку
        operator std::basic_string<charT, traits>() const
        {
            return std::basic_string<charT, traits>(this->data(), this->size());
        }

        // Итераторы
        constexpr const_iterator begin() const
        {
            return this->data();
        }

        constexpr const_iterator cbegin() const
        {
            return this->begin();
        }

        constexpr const_iterator end() const
        {
            return this->begin() + this->size();
        }

        constexpr const_iterator cend() const
        {
            return this->end();
        }

        constexpr const_reverse_iterator rbegin() const
        {
            return reverse_iterator(this->end());
        }

        constexpr const_reverse_iterator crbegin() const
        {
            return this->rbegin();
        }

        constexpr const_reverse_iterator rend() const
        {
            return reverse_iterator(this->begin());
        }

        constexpr const_reverse_iterator crend() const
        {
            return this->rend();
        }

        // Ёмкость
        constexpr size_type size() const noexcept
        {
            return this->size_;
        }

        constexpr size_type length() const noexcept
        {
            return this->size();
        }

        constexpr size_type max_size() const noexcept
        {
            return std::numeric_limits<size_type>::max() / sizeof(value_type);
        }

        constexpr bool empty() const
        {
            return 0 == this->size();
        }

        // Доступ к элементам
        constexpr const_reference operator[](size_type pos) const
        {
            assert(pos < this->size());

            return this->data()[pos];
        }

        constexpr const_reference at(size_type pos) const
        {
            return (pos < this->size())
                   ? (*this)[pos]
                   : throw std::out_of_range("saga::string_view::at"), (*this)[pos];
        }

        constexpr const_reference front() const
        {
            assert(!this->empty());
            return (*this)[0];
        }

        constexpr const_reference back() const
        {
            assert(!this->empty());
            return (*this)[this->size() - 1];
        }

        constexpr const_pointer data() const
        {
            return this->data_;
        }

        // Модификаторы
        void remove_prefix(size_type n)
        {
            assert(n <= this->size());
            this->data_ += n;
            this->size_ -= n;
        }

        void remove_suffix(size_type n)
        {
            assert(n <= this->size());
            this->size_ -= n;
        }

        void swap(basic_string_view & that) noexcept
        {
            using std::swap;
            swap(this->data_, that.data_);
            swap(this->size_, that.size_);
        }

        // Строковые операции
        /**
        @pre <tt>[s, s+n)</tt> является корректным интервалом
        */
        size_type copy(charT * s, size_type n, size_type pos = 0) const
        {
            if(pos > this->size())
            {
                throw std::out_of_range("saga::string_view::copy");
            }

            auto const rlen = std::min(n, this->size() - pos);

            traits_type::copy(s, this->data() + pos, rlen);

            return rlen;
        }

        basic_string_view substr(size_type pos = 0, size_type n = npos) const
        {
            if(pos > this->size())
            {
                throw std::out_of_range("saga::string_view::substr");
            }

            auto const rlen = std::min(n, this->size() - pos);
            return basic_string_view(this->data() + pos, rlen);
        }

        /**
        Сложность является линейной по rlen
        */
        int compare(basic_string_view str) const noexcept
        {
            auto const rlen = std::min(this->size(), str.size());

            auto const cmp_common = traits_type::compare(this->data(), str.data(), rlen);

            if(cmp_common != 0)
            {
                return cmp_common;
            }
            else if(this->size() < str.size())
            {
                return -1;
            }
            else
            {
                return this->size() - str.size();
            }
        }

        int compare(size_type pos1, size_type n1, basic_string_view str) const
        {
            return this->substr(pos1, n1).compare(str);
        }

        int compare(size_type pos1, size_type n1,
                    basic_string_view str, size_type pos2, size_type n2) const
        {
            return this->substr(pos1, n1).compare(str.substr(pos2, n2));
        }

        int compare(char const * s) const
        {
            return this->compare(basic_string_view(s));
        }

        int compare(size_type pos1, size_type n1, char const * s) const
        {
            return this->substr(pos1, n1).compare(basic_string_view(s));
        }

        int compare(size_type pos1, size_type n1, char const * s, size_type n2) const
        {
            return this->substr(pos1, n1).compare(basic_string_view(s, n2));
        }

        bool starts_with(basic_string_view str) const noexcept
        {
            return this->substr(0, str.size()) == str;
        }

        bool starts_with(charT c) const noexcept
        {
            return !this->empty() && traits_type::eq(this->front(), c);
        }

        bool starts_with(charT const * s) const
        {
            return this->starts_with(basic_string_view(s));
        }

        bool ends_with(basic_string_view str) const noexcept
        {
            return this->size() >= str.size()
                    && this->compare(this->size() - str.size(), str.size(), str) == 0;
        }

        bool ends_with(charT c) const noexcept
        {
            return !this->empty() && traits_type::eq(this->back(), c);
        }

        bool ends_with(charT const * s) const
        {
            return this->ends_with(basic_string_view(s));
        }

        // Поиск
        size_type find(basic_string_view str, size_type pos = 0) const noexcept
        {
            if(pos > this->size())
            {
                return npos;
            }

            if(str.empty())
            {
                return pos;
            }

            auto const first = this->begin() + pos;
            auto const last = this->end();

            auto const result = std::search(first, last, str.begin(), str.end(), traits_type::eq);

            return result == last ? npos : result - first;
        }

        size_type find(const charT * str, size_type pos = 0) const noexcept
        {
            return this->find(basic_string_view(str), pos);
        }

        size_type find(const charT * str, size_type pos, size_type n) const noexcept
        {
            return this->find(basic_string_view(str, n), pos);
        }

        size_type find(charT c, size_type pos = 0) const noexcept
        {
            return this->find(basic_string_view(&c, 1), pos);
        }

        size_type rfind(basic_string_view str, size_type pos = npos) const noexcept
        {
            if(this->size() < str.size())
            {
                return npos;
            }

            if(str.empty())
            {
                return std::min(pos, this->size());
            }

            pos = std::min(pos, this->size());

            auto const first = this->begin();
            auto const last = this->begin() + str.size() + std::min(pos, this->size() - str.size());

            auto const result = std::find_end(first, last, str.begin(), str.end(), traits_type::eq);

            return (result == last) ? npos : result - first;
        }

        size_type rfind(charT const * str, size_type pos = npos) const noexcept
        {
            return this->rfind(basic_string_view(str), pos);
        }

        size_type rfind(const charT * str, size_type pos, size_type n) const noexcept
        {
            return this->rfind(basic_string_view(str, n), pos);
        }

        size_type rfind(charT c, size_type pos = npos) const noexcept
        {
            return this->rfind(basic_string_view(&c, 1), pos);
        }

        size_type find_first_of(basic_string_view str, size_type pos = 0) const noexcept
        {
            if(str.empty() || pos >= this->size())
            {
                return npos;
            }

            auto const first = this->begin() + pos;
            auto const last = this->end();

            auto const result
                = std::find_first_of(first, last, str.begin(), str.end(), traits_type::eq);

            return (result == last) ? npos : result - first;
        }

        size_type find_first_of(charT const * str, size_type pos = 0) const noexcept
        {
            return this->find_first_of(basic_string_view(str), pos);
        }

        size_type find_first_of(const charT * str, size_type pos, size_type n) const noexcept
        {
            return this->find_first_of(basic_string_view(str, n), pos);
        }

        size_type find_first_of(charT c, size_type pos = 0) const noexcept
        {
            return this->find_first_of(basic_string_view(&c, 1), pos);
        }

        size_type find_last_of(basic_string_view str, size_type pos = npos) const noexcept
        {
            if(this->empty())
            {
                return npos;
            }

            pos = std::min(pos, this->size()-1);

            auto const first = reverse_iterator(this->begin() + pos + 1);
            auto const last = this->rend();

            auto const result
                = std::find_first_of(first, last, str.begin(), str.end(), traits_type::eq);

            if(result == last)
            {
                return npos;
            }

            return size_type(last - result - 1);
        }

        size_type find_last_of(charT const * str, size_type pos = npos) const noexcept
        {
            return this->find_last_of(basic_string_view(str), pos);
        }

        size_type find_last_of(charT const * str, size_type pos, size_type n) const noexcept
        {
            return this->find_last_of(basic_string_view(str, n), pos);
        }

        size_type find_last_of(charT c, size_type pos = npos) const noexcept
        {
            return this->find_last_of(basic_string_view(&c, 1), pos);
        }

        size_type find_first_not_of(basic_string_view str, size_type pos = 0) const noexcept
        {
            if(pos >= this->size())
            {
                return npos;
            }

            auto not_in_str = [&](charT c)
            {
                return std::find_if(str.begin(), str.end(),
                                    [&](charT x) { return traits_type::eq(c, x); }) == str.end();
            };

            auto const first = this->begin();
            auto const last = this->end();

            auto const result = std::find_if(first, last, not_in_str);

            return result == last ? npos : result - first;
        }

        size_type find_first_not_of(charT const * str, size_type pos = 0) const noexcept
        {
            return this->find_first_not_of(basic_string_view(str), pos);
        }

        size_type find_first_not_of(charT const * str, size_type pos, size_type n) const noexcept
        {
            return this->find_first_not_of(basic_string_view(str, n), pos);
        }

        size_type find_first_not_of(charT c, size_type pos = 0) const noexcept
        {
            return this->find_first_not_of(basic_string_view(&c, 1), pos);
        }

        size_type find_last_not_of(basic_string_view str, size_type pos = npos) const noexcept
        {
            if(this->empty())
            {
                return npos;
            }

            auto not_in_str = [&](charT c)
            {
                return std::find_if(str.begin(), str.end(),
                                    [&](charT x) { return traits_type::eq(c, x); }) == str.end();
            };

            pos = std::min(pos, this->size() - 1);

            auto const first = const_reverse_iterator(this->begin() + pos + 1);
            auto const last = this->rend();

            auto const result = std::find_if(first, last, not_in_str);

            return this->size() - (result - first) - 1;
        }

        size_type find_last_not_of(charT const * str, size_type pos = npos) const noexcept
        {
            return this->find_last_not_of(basic_string_view(str), pos);
        }

        size_type find_last_not_of(charT const * str, size_type pos, size_type n) const noexcept
        {
            return this->find_last_not_of(basic_string_view(str, n), pos);
        }

        size_type find_last_not_of(charT c, size_type pos = npos) const noexcept
        {
            return this->find_last_not_of(basic_string_view(&c, 1), pos);
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

    // Суффиксы для литералов basic_string_view
    inline namespace literals {
    inline namespace string_view_literals
    {
        constexpr string_view operator""_sv(char const * str, std::size_t length) noexcept
        {
            return string_view{str, length};
        }

        constexpr u16string_view operator""_sv(char16_t const * str, std::size_t length) noexcept
        {
            return u16string_view{str, length};
        }

        constexpr u32string_view operator""_sv(char32_t const * str, std::size_t length) noexcept
        {
            return u32string_view{str, length};
        }

        constexpr wstring_view operator""_sv(wchar_t const * str, std::size_t length) noexcept
        {
            return wstring_view{str, length};
        }
    }
    }
}
//namespace saga


namespace std
{
    template <class charT, class traits>
    struct hash<saga::basic_string_view<charT, traits>>
    {
        std::size_t operator()(saga::basic_string_view<charT, traits> sv) const noexcept
        {
            using String = std::basic_string<charT, traits>;
            return std::hash<String>{}(String(sv));
        }
    };
}

#endif
// Z_SAGA_STRING_VIEW_HPP_INCLUDED
