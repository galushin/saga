/* (c) 2022 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_FLAT_SET_HPP_INCLUDED
#define Z_SAGA_FLAT_SET_HPP_INCLUDED

/** @file saga/flat_set.hpp
 @brief Аналог std::set на основе контейнера с (предположительно) непрерывной памятью

 Основаны на https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p1222r4.pdf
 Однако имеются следующие отличия:
 * Используется saga::reverse_iterator вместо std::reverse_iterator -
 * Функции передаётся в конструкторы по значению, а не по константной ссылке - их всё равно придётся
 копировать

 @todo Подумать как интегрировать курсоры saga
*/

#include <saga/algorithm.hpp>
#include <saga/cursor/subrange.hpp>
#include <saga/iterator/reverse.hpp>

namespace saga
{
    // @todo Проверить, что итераторы с произвольным доступом
    // @todo Проверить требования к контейнерам ([container.reqmts])
    // @todo Проверить требования к обратимым контейнерам ([container.rev.reqmts])
    // @todo Проверить дополнительные требования к контейнерам ([container.opt.reqmts]).
    /* @todo Проверить требования к ассоциативным контейнерам ([associative.reqmts]), кроме
     связанных с узлами, недействительностью итераторов, сложности вставки и удаления одного
     элемента */
    // @todo a_uniq операции из [associative.reqmts]
    // @todo Инвариант: элементы отсортирован и уникальны
    // @todo При исключении интервал должен быть восстановлен, даже если придётся опустошить
    // @todo Аудит в функциях, использующих sorted_unique

    // @todo sorted_unique_t и sorted_unique
    // @todo Возможность задавать key_container
    // @todo erase_if(flat_set, Pred)
    // @todo Специализация uses_allocator

    // @todo Выразить как можно больше функций через CRTP (особенно те, в которых нет доступа к private)
    template <class Key, class Compare = std::less<Key>, class KeyContainer = std::vector<Key>>
    class flat_set
     : private Compare
    {
        static_assert(std::is_same<Key, typename KeyContainer::value_type>{});

    public:
        // Типы
        using key_type               = Key;
        using value_type             = Key;
        using key_compare            = Compare;
        using value_compare          = Compare;
        using reference              = value_type &;
        using const_reference        = value_type const &;
        using size_type              = typename KeyContainer::size_type;
        using difference_type        = typename KeyContainer::difference_type;
        using iterator               = typename KeyContainer::const_iterator;
        using const_iterator         = typename KeyContainer::const_iterator;
        using reverse_iterator       = saga::reverse_iterator<iterator>;
        using const_reverse_iterator = saga::reverse_iterator<const_iterator>;
        using container_type         = KeyContainer;

        // Создание, копирование, уничтожение
        flat_set()
         : flat_set(key_compare())
        {}

        explicit flat_set(container_type src)
         : Compare()
         , data_(std::move(src))
        {
            auto cur = saga::cursor::all(this->data_);
            saga::sort(cur, this->cmp_ref());

            // @todo Оптимизация: мы знаем, что вторая часть конъюнкции всегда истина после sort
            auto equiv = [&](const_reference lhs, const_reference rhs)
            {
                return !this->cmp_ref()(lhs, rhs) && !this->cmp_ref()(rhs, lhs);
            };

            cur = saga::unique(cur, equiv);
            this->data_.erase(cur.begin(), cur.end());
        }

        // @todo Конструктор с контейнером и распределителем памяти
        // @todo Конструктор с отсортированным контейнером
        // @todo Конструктор с отсортированным контейнером и распределителем памяти

        // @todo Покрыть тестами
        // @todo Должно быть explicit
        flat_set(key_compare cmp)
         : Compare(std::move(cmp))
         , data_()
        {}

        // @todo Конструктор с функцией сравнения и распределителем памяти
        // @todo Конструктор с распределителем памяти

        // @todo Конструктор из пары итераторов и, возможно, функции сравнения

        template <class InputIterator, class Allocator>
        flat_set(InputIterator first, InputIterator last, key_compare const & cmp
                , Allocator const & alloc)
         : Compare(cmp)
         // @todo использовать std::make_obj_using_allocator<container_type>(alloc) из C++ 20
         , data_(alloc)
        {
            // @todo Должно быть this->insert(std::move(first), std::move(last)); вместо всего кода
            this->data_.insert(this->data_.end(), std::move(first), std::move(last));

            auto cur = saga::cursor::all(this->data_);
            saga::sort(cur, this->cmp_ref());

            // @todo Оптимизация: мы знаем, что вторая часть конъюнкции всегда истина после sort
            auto equiv = [&](const_reference lhs, const_reference rhs)
            {
                return !this->cmp_ref()(lhs, rhs) && !this->cmp_ref()(rhs, lhs);
            };

            cur = saga::unique(cur, equiv);
            this->data_.erase(cur.begin(), cur.end());
        }

        // @todo Конструктор из пары итераторов и распределителя памяти

        // @todo Конструктор из интервала
        // @todo Конструктор из интервала и распределителя памяти
        // @todo Конструктор из интервала и функции сравнения
        // @todo Конструктор из интервала, функции сравнения и распределителя памяти

        // @todo Конструктор из упорядоченного интервала (пары итераторов) и, возможно, функции сравнения
        // @todo Конструктор из упорядоченного интервала (пары итераторов), функции сравнения и распределителя памяти
        // @todo Конструктор из упорядоченного интервала (пары итераторов) и распределителя памяти

        // @todo Конструктор из списка инициализации и, возможно, функции сравнения
        // @todo Конструктор из списка инициализации, функции сравнения и распределителя памяти
        // @todo Конструктор из списка инициализации и распределителя памяти

        // @todo Конструктор из упорядоченного списка инициализации и, возможно, функции сравнения
        // @todo Конструктор из упорядоченного списка инициализации, функции сравнения и распределителя памяти
        // @todo Конструктор из упорядоченного списка инициализации и распределителя памяти

        // @todo Оператор присваивания списка инициализации

        // @todo Удалить? Не входит в предложением по стандартизации
        typename container_type::allocator_type get_allocator() const noexcept
        {
            return this->data_.get_allocator();
        }

        // Итераторы
        // @todo Определить все функции
        // @todo Должно быть noexcept
        const_iterator begin() const
        {
            return this->data_.begin();
        }

        // @todo Должно быть noexcept
        const_iterator end() const
        {
            return this->data_.end();
        }

        // Размер и ёмкость
        [[nodiscard]] bool empty() const noexcept
        {
            return this->data_.empty();
        }

        size_type size() const noexcept
        {
            return this->data_.size();
        }

        // @todo max_size

        // Модифицирующие операции
        iterator insert(const_iterator, value_type && value)
        {
            // Находим место
            auto pos = this->lower_bound(value);

            // Если pos действительный итератор, то *pos >= value
            if(pos == this->end() || this->cmp_ref()(value, *pos))
            {
                return this->data_.insert(pos, std::move(value));
            }
            else
            {
                return pos;
            }
        }

        void swap(flat_set & other) noexcept
        {
            using std::swap;

            swap(this->cmp_ref(), other.cmp_ref());
            swap(this->data_, other.data_);
        }

        // Немодифицирующие операции
        key_compare key_comp() const
        {
            return this->cmp_ref();
        }

        value_compare value_comp() const
        {
            return this->key_comp();
        }

        // Операции множества
        iterator lower_bound(key_type const & key)
        {
            return saga::lower_bound(saga::cursor::all(this->data_)
                                     , key, std::ref(this->cmp_ref())).begin();
        }

        friend bool operator==(flat_set const & lhs, flat_set const & rhs)
        {
            return saga::equal(saga::cursor::all(lhs), saga::cursor::all(rhs));
        }

        // @todo Обойтиьс без явного определения?
        friend bool operator!=(flat_set const & lhs, flat_set const & rhs)
        {
            return !(lhs == rhs);
        }

        friend void swap(flat_set & lhs, flat_set & rhs) noexcept
        {
            lhs.swap(rhs);
        }

    private:
        value_compare const & cmp_ref() const
        {
            return *this;
        }

        value_compare & cmp_ref()
        {
            return *this;
        }

        KeyContainer data_;
    };
}
// namespace saga

#endif
// Z_SAGA_FLAT_SET_HPP_INCLUDED
