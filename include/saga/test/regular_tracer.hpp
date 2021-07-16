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

namespace saga
{
    template <class T, class Tag = void>
    class regular_tracer
    {
    public:
        // Счётчики операций
        using count_type = long;

        static count_type const & constructed()
        {
            return regular_tracer::constructed_ref();
        }

        static count_type const & destructed()
        {
            return regular_tracer::destructed_ref();
        }

        static count_type const & copy_constructed()
        {
            return regular_tracer::copy_constructed_ref();
        }

        static count_type const & move_constructed()
        {
            return regular_tracer::move_constructed_ref();
        }

        static count_type const & copy_assignments()
        {
            return regular_tracer::copy_assignments_ref();
        }

        static count_type const & move_assignments()
        {
            return regular_tracer::move_assignments_ref();
        }

        static count_type const & equality_comparisons()
        {
            return regular_tracer::equality_comparisons_ref();
        }

        // Типы
        using value_type = T;
        using tag_type = Tag;

        // Создание, присваивание и уничтожение
        explicit regular_tracer(T init_value)
         : value_(std::move(init_value))
        {
            ++ regular_tracer::constructed_ref();
        }

        regular_tracer(regular_tracer const & rhs)
         : value_(rhs.value_)
        {
            ++ regular_tracer::copy_constructed_ref();
            ++ regular_tracer::constructed_ref();
        }

        regular_tracer(regular_tracer && rhs)
         : value_(std::move(rhs.value_))
        {
            ++ regular_tracer::move_constructed_ref();
            ++ regular_tracer::constructed_ref();
        }

        regular_tracer & operator=(regular_tracer const & rhs)
        {
            this->value_ = rhs.value();

            ++ regular_tracer::copy_assignments_ref();

            return *this;
        }

        regular_tracer & operator=(regular_tracer && rhs)
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

        static count_type & constructed_ref()
        {
            static count_type instance{0};
            return instance;
        }

        static count_type & destructed_ref()
        {
            static count_type instance{0};
            return instance;
        }

        static count_type & copy_constructed_ref()
        {
            static count_type instance{0};
            return instance;
        }

        static count_type & move_constructed_ref()
        {
            static count_type instance{0};
            return instance;
        }

        static count_type & copy_assignments_ref()
        {
            static count_type instance{0};
            return instance;
        }

        static count_type & move_assignments_ref()
        {
            static count_type instance{0};
            return instance;
        }

        static count_type & equality_comparisons_ref()
        {
            static count_type instance{0};
            return instance;
        }
    };
}
// namespace saga

#endif
// Z_SAGA_TEST_REGULAR_TRACER_HPP_INCLUDED
