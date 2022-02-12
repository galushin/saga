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

#ifndef Z_SAGA_EXPECTED_DETAIL_HPP_INCLUDED
#define Z_SAGA_EXPECTED_DETAIL_HPP_INCLUDED

#include <saga/expected/unexpected.hpp>
#include <saga/type_traits.hpp>

#include <saga/detail/default_ctor_enabler.hpp>

#include <cassert>

/// @cond false
namespace saga
{
    template <class Value, class Error>
    class expected;

    namespace detail
    {
        struct void_wrapper
        {};

        struct expected_uninitialized
        {};

        enum class expected_storage_state
        {
            empty,
            value,
            error
        };

        struct two_phase_ctor_tag{};

        template <class Value, class Error,
                  bool Value_trivially_destructible = std::is_trivially_destructible<Value>::value,
                  bool Error_trivially_destructible = std::is_trivially_destructible<Error>::value>
        struct expected_destructible
        {
            // Переменные-члены
            using unexpected_type = saga::unexpected<Error>;
            union
            {
                expected_uninitialized no_init_;
                Value value_;
                unexpected_type error_;
            };

            expected_storage_state state_ = expected_storage_state::empty;

            // Конструкторы и деструктор
            explicit expected_destructible(expected_uninitialized)
             : no_init_{}
             , state_(expected_storage_state::empty)
            {}

            template <class... Args>
            constexpr explicit expected_destructible(in_place_t, Args &&... args)
             : value_(std::forward<Args>(args)...)
             , state_(expected_storage_state::value)
            {}

            template <class... Args>
            constexpr explicit expected_destructible(unexpect_t, Args &&... args)
             : error_(saga::in_place_t{}, std::forward<Args>(args)...)
             , state_(expected_storage_state::error)
            {}

            ~expected_destructible()
            {
                if(this->state_ == expected_storage_state::value)
                {
                    this->value_.~Value();
                }
                else if(this->state_ == expected_storage_state::error)
                {
                    this->error_.~unexpected();
                }
                static_assert(std::is_trivially_destructible<expected_uninitialized>{}, "");
            }
        };

        template <class Value, class Error>
        struct expected_destructible<Value, Error, true, true>
        {
            static_assert(std::is_trivially_destructible<Value>{}, "");
            static_assert(std::is_trivially_destructible<Error>{}, "");

            // Переменные-члены
            using unexpected_type = saga::unexpected<Error>;
            union
            {
                expected_uninitialized no_init_;
                Value value_;
                unexpected_type error_;
            };

            expected_storage_state state_ = expected_storage_state::empty;

            // Конструкторы и деструктор
            explicit expected_destructible(expected_uninitialized)
             : no_init_{}
             , state_(expected_storage_state::empty)
            {}

            template <class... Args>
            constexpr explicit expected_destructible(in_place_t, Args &&... args)
             : value_(std::forward<Args>(args)...)
             , state_(expected_storage_state::value)
            {}

            template <class... Args>
            constexpr explicit expected_destructible(unexpect_t, Args &&... args)
             : error_(saga::in_place_t{}, std::forward<Args>(args)...)
             , state_(expected_storage_state::error)
            {}

            ~expected_destructible() = default;
        };

        template <class Value, class Error>
        class expected_storage_basic
        {
            using unexpected_type = saga::unexpected<Error>;

        public:
            // Создание и копирование
            constexpr explicit expected_storage_basic(expected_uninitialized)
             : impl_(expected_uninitialized{})
            {}

            template <class... Args>
            constexpr explicit expected_storage_basic(in_place_t, Args &&... args)
             : impl_(in_place_t{}, std::forward<Args>(args)...)
            {}

            template <class... Args>
            constexpr explicit expected_storage_basic(unexpect_t, Args &&... args)
             : impl_(unexpect_t{}, std::forward<Args>(args)...)
            {}

            // Уничтожение
            ~expected_storage_basic() = default;

            // Модифицирующие операции
            void destroy_value() noexcept
            {
                assert(this->impl_.state_ == expected_storage_state::value);

                this->impl_.value_.~Value();
                this->impl_.state_ = expected_storage_state::empty;
            }

            void destroy_error() noexcept
            {
                assert(this->impl_.state_ == expected_storage_state::error);

                this->impl_.error_.~unexpected_type();
                this->impl_.state_ = expected_storage_state::empty;
            }

            template <class... Args>
            void emplace_value(Args &&... args)
            {
                assert(this->state() == expected_storage_state::empty);

                ::new(std::addressof(this->impl_.value_)) Value(std::forward<Args>(args)...);

                this->impl_.state_ = expected_storage_state::value;
            }

            template <class... Args>
            void emplace_error(Args &&... args)
            {
                assert(this->state() == expected_storage_state::empty);

                 ::new(std::addressof(this->impl_.error_))
                    unexpected_type(saga::in_place, std::forward<Args>(args)...);

                this->impl_.state_ = expected_storage_state::error;
            }

            // Свойства
            constexpr expected_storage_state state() const
            {
                return this->impl_.state_;
            }

            constexpr bool has_value() const
            {
                return this->state() == expected_storage_state::value;
            }

            constexpr Value const & operator*() const
            {
                assert(this->has_value());
                return this->impl_.value_;
            }

            constexpr Value & operator*()
            {
                assert(this->has_value());
                return this->impl_.value_;
            }

            constexpr Error const & error() const
            {
                assert(!this->has_value());
                return this->impl_.error_.value();
            }

            constexpr Error & error()
            {
                assert(!this->has_value());
                return this->impl_.error_.value();
            }

        private:
            // @todo Доступ к value_ и error_ должен быть через saga::launder
            expected_destructible<Value, Error> impl_;
        };

        /**
        @todo Уменьшить дублирование между функциями, в частности - инкапсулировать вызовы new и
        декструкторов
        */
        template <class Value, class Error>
        class expected_storage
         : private expected_storage_basic<Value, Error>
        {
            using Base = expected_storage_basic<Value, Error>;
        public:
            // Создание и копирование
            using Base::Base;

            // Уничтожение
            ~expected_storage() = default;

            // Свойства
            using Base::has_value;

            using Base::operator*;
            using Base::error;

            // Модифицирующие опреации
            template <class... Args>
            Value & emplace(Args &&... args)
            {
                if(this->has_value())
                {
                    **this = Value(std::forward<Args>(args)...);
                }
                else
                {
                    // @todo Оптимизация для случая, когда конструкторы не порождают исключения
                    auto tmp = std::move(this->error());

                    Base::destroy_error();

                    try
                    {
                        Base::emplace_value(std::forward<Args>(args)...);
                    }
                    catch(...)
                    {
                        Base::emplace_error(std::move(tmp));
                        throw;
                    }
                }
                assert(this->has_value());

                return **this;
            }

            template <class OtherError>
            void assign_error(OtherError && error)
            {
                // @todo Оптимизация: не создавать unexpected_type, а передавать исходный?
                if(!this->has_value())
                {
                    this->error() = std::forward<OtherError>(error);
                }
                else
                {
                    // @todo Доказать, что это безопасно при исключениях
                    Base::destroy_value();

                    Base::emplace_error(std::forward<OtherError>(error));
                }

                assert(!this->has_value());
            }

            // Обмен
            void swap(expected_storage & rhs)
            {
                if(rhs.has_value())
                {
                    if(this->has_value())
                    {
                        using std::swap;
                        swap(**this, *rhs);
                    }
                    else
                    {
                        rhs.swap(*this);
                    }
                }
                else
                {
                    if(this->has_value())
                    {
                        this->swap_value_error(rhs, std::is_nothrow_move_constructible<Error>{});
                    }
                    else
                    {
                        using std::swap;
                        swap(this->error(), rhs.error());
                    }
                }
            }

        protected:
            template <class OtherValue, class OtherError>
            expected_storage(two_phase_ctor_tag,
                             expected_storage<OtherValue, OtherError> const & rhs)
             : Base(expected_uninitialized{})
            {
                if(rhs.has_value())
                {
                    Base::emplace_value(*rhs);
                }
                else
                {
                    Base::emplace_error(rhs.error());
                }
            }

            template <class OtherValue, class OtherError>
            expected_storage(two_phase_ctor_tag, expected_storage<OtherValue, OtherError> && rhs)
             : Base(expected_uninitialized{})
            {
                if(rhs.has_value())
                {
                    Base::emplace_value(std::move(*rhs));
                }
                else
                {
                    Base::emplace_error(std::move(rhs.error()));
                }
            }

        private:
            void swap_value_error(Base & rhs, std::true_type)
            {
                static_assert(std::is_nothrow_move_constructible<Error>{}, "");

                assert(this->has_value());
                assert(rhs.state() == expected_storage_state::error);

                // @todo Доказать, что это безопасно при исключениях
                auto tmp_unex = std::move(rhs.error());
                rhs.destroy_error();

                try
                {
                    rhs.emplace_value(std::move(**this));

                    this->assign_error(std::move(tmp_unex));
                }
                catch(...)
                {
                    rhs.emplace_error(std::move(tmp_unex));
                    throw;
                }
            }

            void swap_value_error(Base & rhs, std::false_type)
            {
                static_assert(std::is_nothrow_move_constructible<Value>{}, "");

                assert(this->has_value());
                assert(rhs.state() == expected_storage_state::error);

                // @todo Доказать, что это безопасно при исключениях
                auto tmp_value = std::move(**this);

                this->destroy_value();

                try
                {
                    this->emplace_error(std::move(rhs.error()));

                    rhs.destroy_error();
                    rhs.emplace_value(std::move(tmp_value));
                }
                catch(...)
                {
                    this->emplace_value(std::move(tmp_value));
                    throw;
                }
            }
        };

        template <class Value, class Error,
                  bool trivially_copy_constructible = std::is_trivially_copy_constructible<Value>{}
                                                    && std::is_trivially_copy_constructible<Error>{}>
        class expected_copy_ctor
         : public expected_storage<Value, Error>
        {
            using Base = expected_storage<Value, Error>;

            static_assert(std::is_trivially_copy_constructible<Value>{}, "");
            static_assert(std::is_trivially_copy_constructible<Error>{}, "");

        public:
            using Base::Base;

            expected_copy_ctor(expected_copy_ctor const &) = default;
        };

        template <class Value, class Error>
        class expected_copy_ctor<Value, Error, false>
         : public expected_storage<Value, Error>
        {
            using Base = expected_storage<Value, Error>;
        public:
            using Base::Base;

            expected_copy_ctor(expected_copy_ctor const & rhs)
             : Base(detail::two_phase_ctor_tag{}, rhs)
            {}
        };

        template <class Value, class Error,
                  bool trivially_move_constructible = std::is_trivially_move_constructible<Value>{}
                                                    && std::is_trivially_move_constructible<Error>{}>
        class expected_move_ctor
         : public expected_copy_ctor<Value, Error>
        {
            using Base = expected_copy_ctor<Value, Error>;

            static_assert(std::is_trivially_move_constructible<Value>{}, "");
            static_assert(std::is_trivially_move_constructible<Error>{}, "");

        public:
            using Base::Base;

            expected_move_ctor(expected_move_ctor const &) = default;

            expected_move_ctor(expected_move_ctor &&)
                noexcept(std::is_nothrow_move_constructible<Value>{}
                         && std::is_nothrow_move_constructible<Error>{}) = default;
        };

        template <class Value, class Error>
        class expected_move_ctor<Value, Error, false>
         : public expected_copy_ctor<Value, Error>
        {
            using Base = expected_copy_ctor<Value, Error>;
        public:
            using Base::Base;

            expected_move_ctor(expected_move_ctor const &) = default;

            expected_move_ctor(expected_move_ctor && rhs)
                noexcept(std::is_nothrow_move_constructible<Value>{}
                         && std::is_nothrow_move_constructible<Error>{})
             : Base(detail::two_phase_ctor_tag{}, std::move(rhs))
            {}
        };

        template <class Value, class Error>
        class expected_holder
         : public expected_move_ctor<Value, Error>
        {
            using Base = expected_move_ctor<Value, Error>;

        public:
            // Конструкторы и деструктор
            using Base::Base;

            constexpr expected_holder()
             : Base(in_place_t{})
            {}

            expected_holder(expected_holder const & ) = default;

            expected_holder(expected_holder && rhs) = default;

            ~expected_holder() = default;

            // Присваивание
            expected_holder & operator=(expected_holder const & rhs)
            {
                if(rhs.has_value())
                {
                    this->emplace(*rhs);
                }
                else
                {
                    this->assign_error(rhs.error());
                }

                return *this;
            }

            expected_holder & operator=(expected_holder && rhs)
            {
                if(rhs.has_value())
                {
                    this->emplace(std::move(*rhs));
                }
                else
                {
                    this->assign_error(std::move(rhs.error()));
                }

                return *this;
            }

            // Обмен
            void swap(expected_holder & rhs)
            {
                return Base::swap(rhs);
            }

            // Немодифицирующие операции
            using Base::has_value;

            constexpr Value const & operator*() const &
            {
                assert(this->has_value());

                return Base::operator*();
            }

            constexpr Value & operator*() &
            {
                assert(this->has_value());

                return Base::operator*();
            }

            constexpr Value && operator*() &&
            {
                assert(this->has_value());

                return std::move(Base::operator*());
            }

            constexpr Value const && operator*() const &&
            {
                assert(this->has_value());

                return std::move(Base::operator*());
            }

            constexpr Error const & error() const &
            {
                assert(!this->has_value());

                return Base::error();
            }

            constexpr Error & error() &
            {
                assert(!this->has_value());

                return Base::error();
            }

            constexpr Error && error() &&
            {
                assert(!this->has_value());

                return std::move(Base::error());
            }

            constexpr Error const && error() const &&
            {
                assert(!this->has_value());

                return std::move(Base::error());
            }

            // Модифицирующие операции
            template <class... Args>
            Value & emplace(Args &&... args)
            {
                return Base::emplace(std::forward<Args>(args)...);
            }

            using Base::assign_error;
        };

        // @todo Покрыть тестом инициализацию Enabler во всех конструкторах
        /** @brief Отвечает за учёт различий интерфейсов в общем случае и когда
        <tt> std::is_void<Value>::value == true </tt>
        */
        template <class Value, class Error>
        class expected_base
         : public expected_holder<Value, Error>
         , detail::default_ctor_enabler<std::is_default_constructible<Value>{}>
        {
            using Base = expected_holder<Value, Error>;

            using Enabler = detail::default_ctor_enabler<std::is_default_constructible<Value>{}>;

            static_assert(!std::is_void<Value>{}, "");

        public:
            // Конструкторы
            constexpr expected_base() = default;

            expected_base(expected_base const &) = default;

            expected_base(expected_base &&) = default;

            template <class... Args>
            constexpr explicit expected_base(in_place_t, Args &&... args)
             : Base(in_place_t{}, std::forward<Args>(args)...)
             , Enabler(0)
            {}

            template <class U, class... Args>
            constexpr explicit expected_base(in_place_t,
                                             std::initializer_list<U> inits, Args &&... args)
             : Base(in_place_t{}, inits, std::forward<Args>(args)...)
             , Enabler(0)
            {}

            template <class OtherValue, class OtherError>
            explicit expected_base(expected_base<OtherValue, OtherError> const & rhs)
             : Base(detail::two_phase_ctor_tag{}, rhs)
            {}

            template <class OtherValue, class OtherError>
            explicit expected_base(expected_base<OtherValue, OtherError> && rhs)
             : Base(detail::two_phase_ctor_tag{}, std::move(rhs))
            {}

            template <class... Args>
            constexpr explicit expected_base(unexpect_t, Args &&... args)
             : Base(unexpect_t{}, std::forward<Args>(args)...)
             , Enabler(0)
            {}

            template <class U, class... Args>
            constexpr explicit expected_base(unexpect_t,
                                             std::initializer_list<U> inits, Args &&... args)
             : Base(unexpect_t{}, inits, std::forward<Args>(args)...)
            {}

            // Присваивание
            expected_base & operator=(expected_base const &) = default;
            expected_base & operator=(expected_base &&) = default;

            // emplace
            // @todo Ограничения типа
            template <class... Args>
            Value & emplace(Args &&... args)
            {
                return Base::emplace(std::forward<Args>(args)...);
            }

            // @todo Ограничения типа
            template <class U, class... Args>
            Value & emplace(std::initializer_list<U> inits, Args &&... args)
            {
                return Base::emplace(inits, std::forward<Args>(args)...);
            }

            // Обмен
            void swap(expected_base & rhs)
            {
                return Base::swap(rhs);
            }

            // Немодифицирующие операции
            // @todo constexpr - требуется constexpr для std::addressof
            const Value * operator->() const
            {
                assert(this->has_value());

                return std::addressof(**this);
            }

            Value * operator->()
            {
                assert(this->has_value());

                return std::addressof(**this);
            }

            using Base::operator*;

            constexpr Value const & value() const &
            {
                this->throw_if_has_no_value();

                return **this;
            }

            constexpr Value & value() &
            {
                this->throw_if_has_no_value();

                return **this;
            }

            constexpr Value && value() &&
            {
                std::move(*this).throw_if_has_no_value();

                return *std::move(*this);
            }

            constexpr Value const && value() const &&
            {
                std::move(*this).throw_if_has_no_value();

                return *std::move(*this);
            }

            using Base::has_value;
            using Base::error;

        protected:
            using Base::assign_error;

            template <class OtherValue, class OtherError>
            constexpr bool value_equal_to(expected<OtherValue, OtherError> const & rhs) const
            {
                assert(this->has_value());
                assert(rhs.has_value());

                return **this == *rhs;
            }

            template <class OtherValue, class OtherError>
            constexpr bool value_not_equal_to(expected<OtherValue, OtherError> const & rhs) const
            {
                assert(this->has_value());
                assert(rhs.has_value());

                return **this != *rhs;
            }

        private:
            constexpr void throw_if_has_no_value() &&
            {
                return this->has_value() ? void()
                       : throw saga::bad_expected_access<Error>(std::move(*this).error());
            }

            constexpr void throw_if_has_no_value() const &
            {
                return this->has_value() ? void()
                                         : throw saga::bad_expected_access<Error>(this->error());
            }
        };

        template <class Error>
        class expected_base<void, Error>
         : public expected_holder<void_wrapper, Error>
        {
            using Base = expected_holder<void_wrapper, Error>;

        public:
            // Конструкторы
            constexpr expected_base()
             : Base(saga::in_place_t{})
            {}

            expected_base(expected_base const &) = default;

            expected_base(expected_base &&) = default;

            template <class OtherError>
            explicit expected_base(expected_base<void, OtherError> const & rhs)
             : Base(detail::two_phase_ctor_tag{}, rhs)
            {}

            template <class OtherError>
            explicit expected_base(expected_base<void, OtherError> && rhs)
             : Base(detail::two_phase_ctor_tag{}, std::move(rhs))
            {}

            constexpr explicit expected_base(in_place_t)
             : Base(in_place_t{})
            {}

            template <class... Args>
            constexpr explicit expected_base(unexpect_t, Args &&... args)
             : Base(saga::unexpect_t{}, std::forward<Args>(args)...)
            {}

            template <class U, class... Args>
            constexpr explicit expected_base(unexpect_t,
                                             std::initializer_list<U> inits, Args &&... args)
             : Base(saga::unexpect_t{}, inits, std::forward<Args>(args)...)
            {}

            // Присваивание
            expected_base & operator=(expected_base const &) = default;
            expected_base & operator=(expected_base &&) = default;

            // Размещение
            void emplace()
            {
                static_cast<void>(Base::emplace());
            }

            // Обмен
            void swap(expected_base & rhs)
            {
                // @todo Уменьшить дублирование со случаем не void
                if(rhs.has_value())
                {
                    if(this->has_value())
                    {
                        // Ничего делать не нужно
                    }
                    else
                    {
                        rhs.swap(*this);
                    }
                }
                else
                {
                    if(this->has_value())
                    {
                        // @todo Знает ли компилятор, что внутри этой функции this->has_value() == true?
                        this->assign_error(std::move(rhs.error()));
                        rhs.emplace();
                    }
                    else
                    {
                        using std::swap;
                        swap(this->error(), rhs.error());
                    }
                }
            }

            // Немодифицирующие операции
            void operator*() & = delete;
            void operator*() const & = delete;
            void operator*() && = delete;
            void operator*() const && = delete;

            constexpr bool has_value() const
            {
                return Base::has_value();
            }

            constexpr void value() &&
            {
                return this->has_value() ? void()
                        : throw saga::bad_expected_access<Error>(std::move(*this).error());
            }

            constexpr void value() const &
            {
                return this->has_value() ? void()
                                         : throw saga::bad_expected_access<Error>(this->error());
            }

            using Base::error;

        protected:
            using Base::assign_error;

            template <class OtherValue, class OtherError>
            constexpr bool value_equal_to(expected<OtherValue, OtherError> const &) const
            {
                static_assert(std::is_void<OtherValue>{}, "Must be void!");

                return true;
            }

            template <class OtherValue, class OtherError>
            constexpr bool value_not_equal_to(expected<OtherValue, OtherError> const & rhs) const
            {
                return !this->value_equal_to(rhs);
            }
        };

        // @todo Перенести в более подходящее место, может быть полезно в других классах
        template <bool Enable>
        struct copy_ctor_enabler
        {};

        template <>
        struct copy_ctor_enabler<false>
        {
            copy_ctor_enabler() = default;
            copy_ctor_enabler(copy_ctor_enabler const &) = delete;
            copy_ctor_enabler(copy_ctor_enabler &&) = default;

            copy_ctor_enabler & operator=(copy_ctor_enabler const & ) = default;
        };

        // @todo Перенести в более подходящее место, может быть полезно в других классах
        template <bool Enable>
        struct move_ctor_enabler
        {};

        template <>
        struct move_ctor_enabler<false>
        {
            move_ctor_enabler() = default;
            move_ctor_enabler(move_ctor_enabler const &) = default;
            move_ctor_enabler(move_ctor_enabler &&) = delete;
        };

        template <class T>
        struct is_expected
         : saga::is_specialization_of<T, saga::expected>
        {};

        template <class Value, class Error, class Arg>
        constexpr bool expected_has_ctor_from_arg()
        {
            return !std::is_void<Value>{}
                    && std::is_constructible<Value, Arg&&>{}
                    && !std::is_same<saga::remove_cvref_t<Arg>, in_place_t>{}
                    && !std::is_same<saga::remove_cvref_t<Arg>, saga::expected<Value, Error>>{}
                    && !std::is_same<saga::remove_cvref_t<Arg>, saga::unexpected<Error>>{};
        }

        template <class Value, class Error>
        constexpr bool is_expected_nothrow_swappable()
        {
            return (std::is_void<Value>{}
                    || (std::is_nothrow_move_constructible<Value>{}
                        && std::is_nothrow_swappable<Value>{})                    )
                    && std::is_nothrow_move_constructible<Error>{}
                    && std::is_nothrow_swappable<Error>{};
        }

        template <class Value, class Error>
        constexpr bool expected_is_swappable()
        {
            /* Error должен быть Swappable всегда, Value - если не является void
            Если is_void<Value>, то достаточно, чтобы у Error был конструктор перемещения,
            если же Value - не void, то либо Value, либо Error должен быть is_nothrow_move_constructible.
            Иначе невозможно определить безопасный при исключениях обмен
            */
            return (std::is_void<Value>{} || std::is_swappable<Value>{})
                   && std::is_swappable<Error>{}
                   && ((std::is_void<Value>{} && std::is_move_constructible<Error>{})
                       || std::is_nothrow_move_constructible<Value>{}
                       || std::is_nothrow_move_constructible<Error>{});
        }

        template <class Value, class Error>
        constexpr bool expected_is_copyable()
        {
            return (std::is_void<Value>{} || std::is_copy_constructible<Value>{})
                   && std::is_copy_constructible<Error>{};
        }

        template <class Value, class Error>
        constexpr bool expected_is_moveable()
        {
            return (std::is_void<Value>{} || std::is_move_constructible<Value>{})
                    && std::is_move_constructible<Error>{};
        }

        template <class Value, class Error, class OtherValue, class OtherError>
        constexpr bool expected_has_ctor_from_other()
        {
            return ((std::is_void<Value>{} && std::is_void<OtherValue>{})
                    || std::is_constructible<Value, std::add_lvalue_reference_t<OtherValue const>>{})
                   && std::is_constructible<Error, OtherError const &>{};
        }

        template <class Value, class Error, class OtherValue, class OtherError>
        constexpr bool expected_explicit_from_other()
        {
            // @todo случай void для T и U
            return !std::is_convertible<std::add_lvalue_reference_t<const OtherValue>, Value>{}
                   || !std::is_convertible<OtherError const &, Error>{};
        }

        template <class Value, class Error, class OtherValue, class OtherError>
        constexpr bool expected_has_move_ctor_from_other()
        {
            // @todo Полная реализация
            return ((std::is_void<Value>{} && std::is_void<OtherValue>{})
                    || std::is_constructible<Value, std::add_rvalue_reference_t<OtherValue>>{})
                    && std::is_constructible<Error, OtherError &&>{};
        }

        template <class Value, class Error, class OtherValue, class OtherError>
        constexpr bool expected_explicit_move_from_other()
        {
            // @todo случай void для T и U
            return !std::is_convertible<std::add_rvalue_reference_t<OtherValue>, Value>{}
                   || !std::is_convertible<OtherError &&, Error>{};
        }
    }
    // namespace detail
}
//namespace saga
/// @endcond

#endif
// Z_SAGA_EXPECTED_DETAIL_HPP_INCLUDED
