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

        struct two_phase_ctor_tag{};

        template <class Value, class Error,
                  bool Value_trivially_destructible = std::is_trivially_destructible<Value>::value,
                  bool Error_trivially_destructible = std::is_trivially_destructible<Error>::value>
        struct expected_destructible
        {
        public:
            // Переменные-члены
            using unexpected_type = saga::unexpected<Error>;
            union
            {
                expected_uninitialized no_init_;
                Value value_;
                unexpected_type error_;
            };

            bool has_value_ = false;

            // Конструкторы и деструктор
            explicit expected_destructible(expected_uninitialized)
             : no_init_{}
             , has_value_(false)
            {}

            template <class... Args>
            constexpr explicit expected_destructible(in_place_t, Args &&... args)
             : value_(std::forward<Args>(args)...)
             , has_value_(true)
            {}

            template <class... Args>
            constexpr explicit expected_destructible(unexpect_t, Args &&... args)
             : error_(saga::in_place_t{}, std::forward<Args>(args)...)
             , has_value_(false)
            {}

            ~expected_destructible()
            {
                if(this->has_value_)
                {
                    this->value_.~Value();
                }
                else
                {
                    this->error_.~unexpected();
                }
            }
        };

        template <class Value, class Error>
        struct expected_destructible<Value, Error, true, true>
        {
            static_assert(std::is_trivially_destructible<Value>{}, "");
            static_assert(std::is_trivially_destructible<Error>{}, "");

        public:
            // Переменные-члены
            using unexpected_type = saga::unexpected<Error>;
            union
            {
                expected_uninitialized no_init_;
                Value value_;
                unexpected_type error_;
            };

            bool has_value_ = false;

            // Конструкторы и деструктор
            explicit expected_destructible(expected_uninitialized)
             : no_init_{}
             , has_value_(false)
            {}

            template <class... Args>
            constexpr explicit expected_destructible(in_place_t, Args &&... args)
             : value_(std::forward<Args>(args)...)
             , has_value_(true)
            {}

            template <class... Args>
            constexpr explicit expected_destructible(unexpect_t, Args &&... args)
             : error_(saga::in_place_t{}, std::forward<Args>(args)...)
             , has_value_(false)
            {}

            ~expected_destructible() = default;
        };

        /**
        @todo Уменьшить дублирование между функциями, в частности - инкапсулировать вызовы new и
        декструкторов
        */
        template <class Value, class Error>
        class expected_storage
        {
            using unexpected_type = unexpected<Error>;

        public:
            // Создание и копирование
            template <class... Args>
            constexpr explicit expected_storage(in_place_t, Args &&... args)
             : impl_(in_place_t{}, std::forward<Args>(args)...)
            {}

            template <class... Args>
            constexpr explicit expected_storage(unexpect_t, Args &&... args)
             : impl_(unexpect_t{}, std::forward<Args>(args)...)
            {}

            // Уничтожение
            ~expected_storage() = default;

            // Свойства
            constexpr bool has_value() const
            {
                return this->impl_.has_value_;
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
                    auto tmp = unexpected_type(std::move(this->error()));

                    this->impl_.error_.~unexpected_type();

                    try
                    {
                        new(std::addressof(this->impl_.value_)) Value(std::forward<Args>(args)...);
                        this->impl_.has_value_ = true;
                    }
                    catch(...)
                    {
                        new(std::addressof(this->impl_.error_)) unexpected_type(std::move(tmp));
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
                    this->impl_.error_ = unexpected_type(std::forward<OtherError>(error));
                }
                else
                {
                    // @todo Доказать, что это безопасно при исключениях
                    this->impl_.value_.~Value();

                    new(std::addressof(this->impl_.error_)) unexpected_type(std::forward<OtherError>(error));
                    this->impl_.has_value_ = false;
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
             : impl_(expected_uninitialized{})
            {
                if(rhs.has_value())
                {
                    new(std::addressof(this->impl_.value_)) Value(*rhs);
                }
                else
                {
                    new(std::addressof(this->impl_.error_)) unexpected_type(rhs.error());
                }
                this->impl_.has_value_ = rhs.has_value();
            }

            expected_storage(two_phase_ctor_tag, expected_storage && rhs)
             : impl_(expected_uninitialized{})
            {
                if(rhs.has_value())
                {
                    new(std::addressof(this->impl_.value_)) Value(std::move(*rhs));
                }
                else
                {
                    new(std::addressof(this->impl_.error_)) unexpected_type(std::move(rhs.error()));
                }
                this->impl_.has_value_ = rhs.has_value();
            }

        private:
            void swap_value_error(expected_storage & rhs, std::true_type)
            {
                static_assert(std::is_nothrow_move_constructible<Error>{}, "");

                assert(this->has_value());
                assert(!rhs.has_value());

                // @todo Доказать, что это безопасно при исключениях
                auto tmp_unex = std::move(rhs.error());
                rhs.impl_.error_.~unexpected_type();

                try
                {
                    new(std::addressof(rhs.impl_.value_)) Value(std::move(**this));
                    rhs.impl_.has_value_ = true;

                    this->assign_error(std::move(tmp_unex));
                }
                catch(...)
                {
                    new(std::addressof(rhs.impl_.error_)) unexpected_type(std::move(tmp_unex));
                    throw;
                }
            }

            void swap_value_error(expected_storage & rhs, std::false_type)
            {
                static_assert(std::is_nothrow_move_constructible<Value>{}, "");

                assert(this->has_value());
                assert(!rhs.has_value());

                // @todo Доказать, что это безопасно при исключениях
                auto tmp_value = std::move(**this);
                this->impl_.value_.~Value();

                try
                {
                    new(std::addressof(this->impl_.error_)) unexpected_type(std::move(rhs.error()));
                    this->impl_.has_value_ = false;

                    rhs.emplace(std::move(tmp_value));
                }
                catch(...)
                {
                    new(std::addressof(this->impl_.value_)) Value(std::move(tmp_value));
                    throw;
                }
            }

            expected_destructible<Value, Error> impl_;
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
            constexpr bool compare_value_with(expected<OtherValue, OtherError> const & rhs) const
            {
                return **this == *rhs;
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
            constexpr bool compare_value_with(expected<OtherValue, OtherError> const &) const
            {
                static_assert(std::is_void<OtherValue>{}, "Must be void!");

                return true;
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
         : std::false_type
        {};

        template <class Value, class Error>
        struct is_expected<saga::expected<Value, Error>>
         : std::true_type
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
                        && saga::is_nothrow_swappable<Value>{})                    )
                    && std::is_nothrow_move_constructible<Error>{}
                    && saga::is_nothrow_swappable<Error>{};
        }

        template <class Value, class Error>
        constexpr bool expected_is_swappable()
        {
            /* Error должен быть Swappable всегда, Value - если не является void
            Если is_void<Value>, то достаточно, чтобы у Error был конструктор перемещения,
            если же Value - не void, то либо Value, либо Error должен быть is_nothrow_move_constructible.
            Иначе невозможно определить безопасный при исключениях обмен
            */
            return (std::is_void<Value>{} || saga::is_swappable<Value>{})
                   && saga::is_swappable<Error>{}
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

        template <class T, bool Enable>
        struct swap_adl_enabler
        {
            friend void swap(T & lhs, T & rhs) noexcept(noexcept(lhs.swap(rhs)))
            {
                return lhs.swap(rhs);
            }
        };

        template <class T>
        struct swap_adl_enabler<T, false>
        {
            friend void swap(T & lhs, T & rhs) = delete;
        };
    }
    // namespace detail
}
//namespace saga
/// @endcond

#endif
// Z_SAGA_EXPECTED_DETAIL_HPP_INCLUDED
