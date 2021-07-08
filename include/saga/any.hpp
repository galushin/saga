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

#ifndef Z_SAGA_ANY_HPP_INCLUDED
#define Z_SAGA_ANY_HPP_INCLUDED

/** @file saga/any.hpp
 @brief Объект, который позволяет хранить значение любого типа и получать к нему доступ.
*/

#include <cassert>
#include <typeinfo>
#include <type_traits>
#include <utility>

namespace saga
{
    class any;

    namespace detail
    {
        template <class T>
        T * any_cast_impl(saga::any const & operand);
    }

    class any
    {
    public:
        template <class T>
        friend T * saga::detail::any_cast_impl(any const &);

        // Создание и уничтожение
        // @todo Как протестировать, что этот конструктор constexpr, если деструктор не тривиальный?
        constexpr any() noexcept = default;

        any(any const &) = delete;
        any(any &&) = delete;

        template <class T
                  , std::enable_if_t<std::is_copy_constructible<std::decay_t<T>>{}> * = nullptr>
        // @todo Ограничения
        // @todo explicit?
        any(T && value)
         : type_(&typeid(std::decay_t<T>))
         , destroy_(&any::destroy_heap<std::decay_t<T>>)
         , data_(new std::decay_t<T>(std::forward<T>(value)))
        {}

        ~any()
        {
            this->reset();
        }

        // Присваивание
        any & operator=(any const &) = delete;
        any & operator=(any &&) = delete;

        // Свойства
        bool has_value() const noexcept
        {
            return this->type() != typeid(void);
        }

        std::type_info const & type() const noexcept
        {
            assert(this->type_ != nullptr);

            return *this->type_;
        }

    private:
        void reset()
        {
            this->destroy_(this->data_);

            this->data_ = nullptr;
            this->type_ = &typeid(void);
            this->destroy_ = &any::destroy_empty;
        }

        using Destroy = void (*)(void *);

        static void destroy_empty(void *)
        {
            return;
        }

        template <class T>
        static void destroy_heap(void * data)
        {
            delete static_cast<T*>(data);
        }

        std::type_info const * type_ = &typeid(void);
        Destroy destroy_ = &any::destroy_empty;
        void * data_ = nullptr;
    };

    namespace detail
    {
        template <class T>
        T * any_cast_impl(saga::any const & operand)
        {
            if(operand.type() == typeid(T))
            {
                return static_cast<T*>(operand.data_);
            }
            else
            {
                return nullptr;
            }
        }
    }

    template <class T>
    T const * any_cast(any const * operand) noexcept
    {
        if(operand == nullptr)
        {
            return nullptr;
        }

        return saga::detail::any_cast_impl<T>(*operand);
    }

    template <class T>
    T * any_cast(any *) noexcept
    {
        return nullptr;
    }
}
// namespace saga

#endif
// Z_SAGA_ANY_HPP_INCLUDED
