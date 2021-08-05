/* (c) 2021 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_TEST_REGULAR_TRACER_HPP_INCLUDED
#define Z_SAGA_TEST_REGULAR_TRACER_HPP_INCLUDED

/** @file saga/test/regular_tracer.hpp
 @brief Трассировщик для регулярных объектов
*/

#include <utility>
#include <atomic>

namespace saga
{
    template <class T, class Tag = void>
    class regular_tracer
    {
    public:
        // Типы
        using value_type = T;
        using tag_type = Tag;
        using count_type = std::ptrdiff_t;

        // Счётчики операций
        static count_type constructed()
        {
            return regular_tracer::constructed_ref();
        }

        static count_type destructed()
        {
            return regular_tracer::destructed_ref();
        }

        static count_type copy_constructed()
        {
            return regular_tracer::copy_constructed_ref();
        }

        static count_type move_constructed()
        {
            return regular_tracer::move_constructed_ref();
        }

        static count_type copy_assignments()
        {
            return regular_tracer::copy_assignments_ref();
        }

        static count_type move_assignments()
        {
            return regular_tracer::move_assignments_ref();
        }

        static count_type equality_comparisons()
        {
            return regular_tracer::equality_comparisons_ref();
        }

        // Создание, присваивание и уничтожение
        template <class... Args, class = std::enable_if_t<std::is_constructible<T, Args...>{}>>
        explicit regular_tracer(Args &&... args)
         : value_(std::forward<Args>(args)...)
        {
            ++ regular_tracer::constructed_ref();
        }

        template <class U, class... Args
                  , class = std::enable_if_t<std::is_constructible<T, std::initializer_list<U>&, Args...>{}>>
        explicit regular_tracer(std::initializer_list<U> inits, Args&&... args)
         : value_(inits, std::forward<Args>(args)...)
        {
            ++ regular_tracer::constructed_ref();
        }

        regular_tracer(regular_tracer const & rhs)
         : value_(rhs.value_)
        {
            ++ regular_tracer::copy_constructed_ref();
            ++ regular_tracer::constructed_ref();
        }

        regular_tracer(regular_tracer && rhs) noexcept(std::is_nothrow_move_constructible<T>{})
         : value_(std::move(rhs.value_))
        {
            ++ regular_tracer::move_constructed_ref();
            ++ regular_tracer::constructed_ref();
        }

        regular_tracer &
        operator=(regular_tracer const & rhs)
        {
            this->value_ = rhs.value();

            ++ regular_tracer::copy_assignments_ref();

            return *this;
        }

        regular_tracer &
        operator=(regular_tracer && rhs) noexcept(std::is_nothrow_move_assignable<T>{})
        {
            this->value_ = std::move(rhs.value_);

            ++ regular_tracer::move_assignments_ref();

            return *this;
        }

        ~regular_tracer()
        {
            ++ regular_tracer::destructed_ref();
        }

        // Сравнения
        friend bool operator==(regular_tracer const & lhs, regular_tracer const & rhs)
        {
            ++ regular_tracer::equality_comparisons_ref();

            return lhs.value() == rhs.value();
        }

        friend bool operator!=(regular_tracer const & lhs, regular_tracer const & rhs)
        {
            return !(lhs == rhs);
        }

        // Свойства
        value_type const & value() const
        {
            return this->value_;
        }

    private:
        T value_;

        using count_impl_type = std::atomic<count_type>;

        static count_impl_type & constructed_ref()
        {
            static count_impl_type instance{0};
            return instance;
        }

        static count_impl_type & destructed_ref()
        {
            static count_impl_type instance{0};
            return instance;
        }

        static count_impl_type & copy_constructed_ref()
        {
            static count_impl_type instance{0};
            return instance;
        }

        static count_impl_type & move_constructed_ref()
        {
            static count_impl_type instance{0};
            return instance;
        }

        static count_impl_type & copy_assignments_ref()
        {
            static count_impl_type instance{0};
            return instance;
        }

        static count_impl_type & move_assignments_ref()
        {
            static count_impl_type instance{0};
            return instance;
        }

        static count_impl_type & equality_comparisons_ref()
        {
            static count_impl_type instance{0};
            return instance;
        }
    };
}
// namespace saga

#endif
// Z_SAGA_TEST_REGULAR_TRACER_HPP_INCLUDED
