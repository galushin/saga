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

#include <saga/type_traits.hpp>

#include <cassert>
#include <typeinfo>
#include <type_traits>
#include <utility>

namespace saga
{
    class bad_any_cast
     : public std::bad_cast
    {
    public:
        const char * what() const noexcept override
        {
            return "saga::bad_any_cast";
        }
    };

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

        any(any const & other)
         : type_(other.type_)
         , destroy_(other.destroy_)
         , copy_(other.copy_)
         , data_(other.make_copy())
        {}

        any(any &&) = delete;

        template <class T
                  , std::enable_if_t<std::is_copy_constructible<std::decay_t<T>>{}> * = nullptr>
        // @todo Ограничения
        // @todo explicit?
        any(T && value)
         : type_(&typeid(std::decay_t<T>))
         , destroy_(&any::destroy_heap<std::decay_t<T>>)
         , copy_(&any::copy_heap<std::decay_t<T>>)
         , data_(new std::decay_t<T>(std::forward<T>(value)))
        {}

        ~any()
        {
            this->reset();
        }

        // Присваивание
        any & operator=(any const & rhs)
        {
            any(rhs).swap(*this);

            return *this;
        }

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
        // @todo Должно быть public, покрыть тестами
        void reset()
        {
            this->destroy_(this->data_);

            this->type_ = &typeid(void);
            this->destroy_ = &any::destroy_empty;
            this->copy_ = &any::copy_empty;
            this->data_ = nullptr;
        }

        // @todo Должно быть public, покрыть тестами
        void swap(any & rhs) noexcept
        {
            using std::swap;
            swap(this->type_, rhs.type_);
            swap(this->destroy_, rhs.destroy_);
            swap(this->copy_, rhs.copy_);
            swap(this->data_, rhs.data_);
        }

        void * make_copy() const
        {
            return this->copy_(this->data_);
        }

        using Destroy_strategy = void (*)(void *);
        using Copy_strategy = void * (*)(void *);

        static void destroy_empty(void *)
        {
            return;
        }

        static void * copy_empty(void *)
        {
            return nullptr;
        }

        template <class T>
        static void destroy_heap(void * data)
        {
            delete static_cast<T*>(data);
        }

        template <class T>
        static void * copy_heap(void * data)
        {
            assert(data != nullptr);

            return new T(*static_cast<T*>(data));
        }

        std::type_info const * type_ = &typeid(void);
        Destroy_strategy destroy_ = &any::destroy_empty;
        Copy_strategy copy_ = &any::copy_empty;
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
    T * any_cast(any * operand) noexcept
    {
        if(operand == nullptr)
        {
            return nullptr;
        }

        return saga::detail::any_cast_impl<T>(*operand);
    }

    template <class T>
    T any_cast(any const & operand)
    {
        using U = saga::remove_cvref_t<T>;

        if(auto ptr = saga::any_cast<U>(&operand))
        {
            return static_cast<T>(*ptr);
        }
        else
        {
            throw saga::bad_any_cast{};
        }
    }

    template <class T>
    T any_cast(any & operand)
    {
        using U = saga::remove_cvref_t<T>;

        if(auto ptr = saga::any_cast<U>(&operand))
        {
            return static_cast<T>(*ptr);
        }
        else
        {
            throw saga::bad_any_cast{};
        }
    }

    template <class T>
    T any_cast(any && operand)
    {
        using U = saga::remove_cvref_t<T>;

        if(auto ptr = saga::any_cast<U>(&operand))
        {
            return static_cast<T>(std::move(*ptr));
        }
        else
        {
            throw saga::bad_any_cast{};
        }
    }
}
// namespace saga

#endif
// Z_SAGA_ANY_HPP_INCLUDED
