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

#include <saga/cpp17/launder.hpp>
#include <saga/type_traits.hpp>
#include <saga/utility/in_place.hpp>

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

        template <class Value>
        constexpr bool any_is_constructible_from()
        {
            return !std::is_same<Value, saga::any>{}
                   && std::is_copy_constructible<Value>{}
                   && !saga::is_specialization_of<Value, saga::in_place_type_t>{};
        }

        template <class Value, class... Args>
        constexpr bool any_has_in_place_ctor()
        {
            return std::is_copy_constructible<Value>{}
                   && std::is_constructible<Value, Args...>{};
        }

        template <class Value, class U, class... Args>
        constexpr bool any_has_in_place_ctor_init_list()
        {
            return std::is_constructible<Value, std::initializer_list<U>&, Args...>{}
                    && std::is_copy_constructible<Value>{};
        }

        template <class Value>
        constexpr bool any_has_assign_from()
        {
            return !std::is_same<Value, saga::any>{} && std::is_copy_constructible<Value>{};
        }
    }

    class any
    {
    public:
        friend void swap(any & lhs, any & rhs) noexcept
        {
            lhs.swap(rhs);
        }

        // Создание и уничтожение
        constexpr any() noexcept = default;

        any(any const & other)
         : any()
        {
            other.vtable_.copy(other.storage_, this->storage_);
            this->vtable_ = other.vtable_;
        }

        any(any && other) noexcept
         : any()
        {
            other.vtable_.move(other.storage_, this->storage_);
            this->vtable_ = std::exchange(other.vtable_, VTable{});
        }

        /// @pre  std::decay<T> удовлетворяет требованиям Cpp17CopyConstructible
        template <class T, class Value = std::decay_t<T>
                  , class = std::enable_if_t<detail::any_is_constructible_from<Value>()>>
        any(T && value)
         : any(saga::in_place_type<Value>, std::forward<T>(value))
        {}

        /// @pre  std::decay<T> удовлетворяет требованиям Cpp17CopyConstructible
        template <class T, class... Args, class Value = std::decay_t<T>
                 , class = std::enable_if_t<detail::any_has_in_place_ctor<Value, Args...>()>>
        explicit any(in_place_type_t<T>, Args &&... args)
         : vtable_(any::manager_for<Value>::make_vtable())
         , storage_()
        {
            any::manager_for<Value>::create(this->storage_, std::forward<Args>(args)...);
        }

        /// @pre  std::decay<T> удовлетворяет требованиям Cpp17CopyConstructible
        template <class T, class U, class... Args, class Value = std::decay_t<T>
                 , class = std::enable_if_t<detail::any_has_in_place_ctor_init_list<Value, U, Args...>()>>
        explicit any(in_place_type_t<T>, std::initializer_list<U> inits, Args &&... args)
         : vtable_(any::manager_for<Value>::make_vtable())
         , storage_()
        {
            any::manager_for<Value>::create(this->storage_, inits, std::forward<Args>(args)...);
        }

        ~any()
        {
            this->reset();
        }

        // Присваивание
        any & operator=(any const & rhs)
        {
            *this = any(rhs);

            return *this;
        }

        any & operator=(any && rhs) noexcept
        {
            if(!rhs.has_value())
            {
                this->reset();
            }
            else if(this != &rhs)
            {
                this->reset();

                rhs.vtable_.move(rhs.storage_, this->storage_);
                this->vtable_ = std::exchange(rhs.vtable_, VTable{});
            }

            return *this;
        }

        /// @pre  std::decay<T> удовлетворяет требованиям Cpp17CopyConstructible
        template <class T>
        auto operator=(T && rhs)
        -> std::enable_if_t<detail::any_has_assign_from<std::decay_t<T>>(), any &>
        {
            *this = any(std::forward<T>(rhs));

            return *this;
        }

        // Модифицирующие операции
        /// @pre  std::decay<T> удовлетворяет требованиям Cpp17CopyConstructible
        template<class T, class... Args>
        auto emplace(Args&&... args)
        -> std::enable_if_t<detail::any_has_in_place_ctor<std::decay_t<T>, Args...>(), std::decay_t<T> &>
        {
            using Value = std::decay_t<T>;

            this->reset();

            using Manager = any::manager_for<Value>;

            auto & result = Manager::create(this->storage_, std::forward<Args>(args)...);
            this->vtable_ = Manager::make_vtable();

            return result;
        }

        /// @pre  std::decay<T> удовлетворяет требованиям Cpp17CopyConstructible
        template <class T, class U, class... Args>
        auto emplace(std::initializer_list<U> inits, Args&&... args)
        -> std::enable_if_t<detail::any_has_in_place_ctor_init_list<std::decay_t<T>, U, Args...>(), std::decay_t<T> &>
        {
            using Value = std::decay_t<T>;

            this->reset();

            using Manager = any::manager_for<Value>;

            auto & result = Manager::create(this->storage_, inits, std::forward<Args>(args)...);
            this->vtable_ = Manager::make_vtable();

            return result;
        }

        void reset() noexcept
        {
            this->vtable_.destroy(this->storage_);
            this->vtable_ = {};
        }

        void swap(any & rhs) noexcept
        {
            any tmp = std::move(*this);
            *this = std::move(rhs);
            rhs = std::move(tmp);
        }

        // Свойства
        bool has_value() const noexcept
        {
            return this->type() != typeid(void);
        }

        std::type_info const & type() const noexcept
        {
            assert(this->vtable_.type != nullptr);

            return *this->vtable_.type;
        }

    private:
        struct Storage
        {
            union
            {
                void * ptr = nullptr;
                alignas(void*) char buffer[sizeof(void*)];
            };

            constexpr Storage() noexcept = default;

            Storage(Storage const &) = delete;
            Storage & operator=(Storage const &) = delete;
        };

        template <class T>
        friend T * saga::detail::any_cast_impl(any const &);

        static void destroy_empty(Storage &) noexcept
        {
            return;
        }

        static void copy_empty(Storage const &, Storage &) noexcept
        {
            return;
        }

        static void move_empty(Storage &, Storage &) noexcept
        {
            return;
        }

        template <class T>
        static void destroy_heap(Storage & storage) noexcept
        {
            delete static_cast<T*>(storage.ptr);
        }

        template <class T>
        static void copy_heap(Storage const & src, Storage & dest)
        {
            assert(src.ptr != nullptr);

            dest.ptr = new T(*static_cast<T*>(src.ptr));
        }

        template <class T>
        static void move_heap(Storage & src, Storage & dest) noexcept
        {
            dest.ptr = std::exchange(src.ptr, nullptr);
        }

        template <class T>
        static void * access_heap(Storage const & storage) noexcept
        {
            return storage.ptr;
        }

        using Destroy_strategy = decltype(&any::destroy_heap<int>);
        using Copy_strategy = decltype(&any::copy_heap<int>);
        using Move_strategy = decltype(&any::move_heap<int>);
        using Access_strategy = decltype(&any::access_heap<int>);

        struct VTable
        {
            std::type_info const * type = &typeid(void);
            Destroy_strategy destroy = &any::destroy_empty;
            Copy_strategy copy = &any::copy_empty;
            Move_strategy move = &any::move_empty;
            Access_strategy access = &any::access_heap<int>;
        };

        template <class T>
        struct manager_heap
        {
            static VTable make_vtable()
            {
                return {std::addressof(typeid(T)), &destroy_heap<T>, &copy_heap<T>
                        , &move_heap<T>, &access_heap<T>};
            }

            template <class... Args>
            static T & create(Storage & storage, Args &&... args)
            {
                auto ptr = new T(std::forward<Args>(args)...);
                storage.ptr = ptr;
                return *ptr;
            }
        };

        template <class T>
        struct manager_small
        {
            static T * get_pointer(Storage & storage) noexcept
            {
                return static_cast<T*>(manager_small::access(storage));
            }

            static T const * get_pointer(Storage const & storage) noexcept
            {
                return static_cast<T const*>(manager_small::access(storage));
            }

            static void destroy(Storage & storage) noexcept
            {
                manager_small::get_pointer(storage)->~T();
            }

            static void copy(Storage const & src, Storage & dest)
            {
                auto const ptr = manager_small::get_pointer(src);
                assert(ptr != nullptr);

                manager_small::create(dest, *ptr);
            }

            static void move(Storage & src, Storage & dest) noexcept
            {
                auto const ptr = manager_small::get_pointer(src);
                assert(ptr != nullptr);

                manager_small::create(dest, std::move(*ptr));
                manager_small::destroy(src);
            }

            static void * access(Storage const & storage) noexcept
            {
                return saga::launder(const_cast<T *>(reinterpret_cast<const T*>(&storage.buffer)));
            }

            static VTable make_vtable()
            {
                return {std::addressof(typeid(T)), &manager_small::destroy
                        , &manager_small::copy, &manager_small::move
                        , &manager_small::access};
            }

            template <class... Args>
            static T & create(Storage & storage, Args &&... args)
            {
                auto ptr = ::new(&storage.buffer) T(std::forward<Args>(args)...);
                return *ptr;
            }
        };

        template <class T>
        constexpr static bool is_fit()
        {
            return std::is_nothrow_move_constructible<T>{}
                   && sizeof(T) <= sizeof(void*)
                   && alignof(T) <= alignof(void*);
        }

        template <class T>
        using manager_for = std::conditional_t<any::is_fit<T>(), manager_small<T>, manager_heap<T>>;

        VTable vtable_;
        Storage storage_;
    };

    template <class T, class... Args>
    saga::any
    make_any(Args&&... args)
    {
        return saga::any(saga::in_place_type<T>, std::forward<Args>(args)...);
    }

    template <class T, class U, class... Args>
    saga::any
    make_any(std::initializer_list<U> inits, Args&&... args)
    {
        return saga::any(saga::in_place_type<T>, inits, std::forward<Args>(args)...);
    }

    namespace detail
    {
        template <class T>
        T * any_cast_impl(saga::any const & operand)
        {
            if(operand.type() == typeid(T))
            {
                return static_cast<T*>(operand.vtable_.access(operand.storage_));
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
