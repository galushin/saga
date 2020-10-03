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

        template <class Value, class Error>
        class expected_holder
        {
        public:
            // Типы
            using unexpected_type = unexpected<Error>;

            // Конструкторы и деструктор
            template <class... Args>
            explicit expected_holder(in_place_t, Args &&... args)
             : has_value_(true)
             , value_(std::forward<Args>(args)...)
            {}

            template <class U, class... Args>
            explicit expected_holder(in_place_t, std::initializer_list<U> inits, Args &&... args)
             : has_value_(true)
             , value_(inits, std::forward<Args>(args)...)
            {}

            expected_holder()
             : expected_holder(in_place_t{})
            {}

            expected_holder(expected_holder const & rhs)
             : has_value_(rhs.has_value())
             , dummy_{}
            {
                if(this->has_value())
                {
                    new(std::addressof(this->value_)) Value(*rhs);
                }
                else
                {
                    new(std::addressof(this->error_)) unexpected_type(rhs.error());
                }
            }

            expected_holder(expected_holder && rhs)
                noexcept(std::is_nothrow_move_constructible<Value>{} && std::is_nothrow_move_constructible<Error>{})
             : has_value_(rhs.has_value())
             , dummy_{}
            {
                if(this->has_value())
                {
                    new(std::addressof(this->value_)) Value(std::move(*rhs));
                }
                else
                {
                    new(std::addressof(this->error_)) unexpected_type(std::move(rhs.error()));
                }
            }

            template <class... Args>
            explicit expected_holder(unexpect_t, Args &&... args)
             : has_value_(false)
             , error_(saga::in_place_t{}, std::forward<Args>(args)...)
            {}

            template <class U, class... Args>
            explicit expected_holder(unexpect_t, std::initializer_list<U> inits, Args &&... args)
             : has_value_(false)
             , error_(saga::in_place_t{}, inits, std::forward<Args>(args)...)
            {}

            ~expected_holder()
            {
                // @todo Не вызывать тривиальные деструкторы
                if(this->has_value())
                {
                    this->value_.~Value();
                }
                else
                {
                    this->error_.~unexpected_type();
                }
            }

            // emplace
            template <class... Args>
            Value & emplace(Args &&... args)
            {
                if(this->has_value())
                {
                    this->value_ = Value(std::forward<Args>(args)...);
                }
                else
                {
                    // @todo Оптимизации для std::is_nothrow_constructible<Value, Args...>
                    // и std::is_nothrow_move_constructible<Value>

                    // @todo Доказать, что это безопасно при наличии исключений
                    static_assert(std::is_nothrow_move_constructible<Error>{}, "");

                    unexpected_type tmp(unexpected_type(this->error()));

                    // @todo Не вызывать тривиальный деструктор?
                    this->error_.~unexpected<Error>();

                    try
                    {
                        new(std::addressof(this->value_)) Value(std::forward<Args>(args)...);
                        this->has_value_ = true;
                    }
                    catch(...)
                    {
                        new(std::addressof(this->error_)) unexpected_type(std::move(tmp));
                        throw;
                    }

                }

                return this->value_;
            }

            template <class U, class... Args>
            Value & emplace(std::initializer_list<U> inits, Args &&... args)
            {
                if(this->has_value())
                {
                    this->value_ = Value(inits, std::forward<Args>(args)...);
                }
                else
                {
                    // @todo Оптимизации для std::is_nothrow_constructible<Value, Args...>
                    // и std::is_nothrow_move_constructible<Value>

                    // @todo Доказать, что это безопасно при наличии исключений
                    static_assert(std::is_nothrow_move_constructible<Error>{}, "");

                    unexpected_type tmp(unexpected_type(this->error()));

                    // @todo Не вызывать тривиальный деструктор?
                    this->error_.~unexpected<Error>();

                    try
                    {
                        new(std::addressof(this->value_)) Value(inits, std::forward<Args>(args)...);
                        this->has_value_ = true;
                    }
                    catch(...)
                    {
                        new(std::addressof(this->error_)) unexpected_type(std::move(tmp));
                        throw;
                    }
                }

                return this->value_;
            }

            // Немодифицирующие операции
            bool has_value() const
            {
                return this->has_value_;
            }

            Value const & operator*() const &
            {
                assert(this->has_value());

                return this->value_;
            }

            Value & operator*() &
            {
                assert(this->has_value());

                return this->value_;
            }

            Value && operator*() &&
            {
                assert(this->has_value());

                return std::move(this->value_);
            }

            Value const && operator*() const &&
            {
                assert(this->has_value());

                return std::move(this->value_);
            }

            Error const & error() const &
            {
                assert(!this->has_value());

                return this->error_.value();
            }

            Error & error() &
            {
                assert(!this->has_value());

                return this->error_.value();
            }

            Error && error() &&
            {
                assert(!this->has_value());

                return std::move(this->error_).value();
            }

            Error const && error() const &&
            {
                assert(!this->has_value());

                return std::move(this->error_).value();
            }

        private:
            bool has_value_ = true;
            union
            {
                char dummy_;
                Value value_;
                unexpected_type error_;
            };
        };

        // @todo Уменьшить дублирование с expected_holder
        template <class Value, class Error>
        class expected_holder_trivial
        {
            static_assert(std::is_trivially_destructible<Value>{}, "");
            static_assert(std::is_trivially_destructible<Error>{}, "");

            using unexpected_type = saga::unexpected<Error>;

        public:
            // Ограничения
            static_assert(std::is_trivially_destructible<Value>{}, "");
            static_assert(std::is_trivially_destructible<Error>{}, "");

            // Конструкторы и деструктор
            template <class... Args>
            constexpr explicit expected_holder_trivial(in_place_t, Args &&... args)
             : has_value_(true)
             , value_(std::forward<Args>(args)...)
            {}

            template <class U, class... Args>
            constexpr explicit expected_holder_trivial(in_place_t, std::initializer_list<U> inits,
                                                       Args &&... args)
             : has_value_(true)
             , value_(inits, std::forward<Args>(args)...)
            {}

            constexpr expected_holder_trivial()
             : expected_holder_trivial(in_place_t{})
            {}

            /* @todo Придумать тест, когда деструктор тривиальный, а конструктор копий - нет
            */
            expected_holder_trivial(expected_holder_trivial const & rhs) = default;

            /* @todo Придумать тест, когда деструктор тривиальный, а конструктор перемещения - нет
            */
            expected_holder_trivial(expected_holder_trivial && rhs) = default;

            template <class... Args>
            constexpr explicit expected_holder_trivial(unexpect_t, Args &&... args)
             : has_value_(false)
             , error_(saga::in_place_t{}, std::forward<Args>(args)...)
            {}

            template <class U, class... Args>
            constexpr explicit expected_holder_trivial(unexpect_t, std::initializer_list<U> inits,
                                                       Args &&... args)
             : has_value_(false)
             , error_(saga::in_place_t{}, inits, std::forward<Args>(args)...)
            {}

            ~expected_holder_trivial() = default;

            // emplace
            template <class... Args>
            Value & emplace(Args &&... args)
            {
                if(this->has_value())
                {
                    this->value_ = Value(std::forward<Args>(args)...);
                }
                else
                {
                    // @todo Оптимизации для std::is_nothrow_constructible<Value, Args...>
                    // и std::is_nothrow_move_constructible<Value>

                    // @todo Доказать, что это безопасно при наличии исключений
                    static_assert(std::is_nothrow_move_constructible<Error>{}, "");

                    unexpected_type tmp(unexpected_type(this->error()));

                    // @todo Не вызывать тривиальный деструктор?
                    this->error_.~unexpected<Error>();

                    try
                    {
                        new(std::addressof(this->value_)) Value(std::forward<Args>(args)...);
                        this->has_value_ = true;
                    }
                    catch(...)
                    {
                        new(std::addressof(this->error_)) unexpected_type(std::move(tmp));
                        throw;
                    }
                }

                return this->value_;
            }

            template <class U, class... Args>
            Value & emplace(std::initializer_list<U> inits, Args &&... args)
            {
                if(this->has_value())
                {
                    this->value_ = Value(inits, std::forward<Args>(args)...);
                }
                else
                {
                    // @todo Оптимизации для std::is_nothrow_constructible<Value, Args...>
                    // и std::is_nothrow_move_constructible<Value>

                    // @todo Доказать, что это безопасно при наличии исключений
                    static_assert(std::is_nothrow_move_constructible<Error>{}, "");

                    unexpected_type tmp(unexpected_type(this->error()));

                    // @todo Не вызывать тривиальный деструктор?
                    this->error_.~unexpected<Error>();

                    try
                    {
                        new(std::addressof(this->value_)) Value(inits, std::forward<Args>(args)...);
                        this->has_value_ = true;
                    }
                    catch(...)
                    {
                        new(std::addressof(this->error_)) unexpected_type(std::move(tmp));
                        throw;
                    }
                }

                return this->value_;
            }

            // Немодифицирующие операции
            constexpr bool has_value() const
            {
                return this->has_value_;
            }

            constexpr Value const & operator*() const &
            {
                assert(this->has_value());

                return this->value_;
            }

            constexpr Value & operator*() &
            {
                assert(this->has_value());

                return this->value_;
            }

            constexpr Value && operator*() &&
            {
                assert(this->has_value());

                return std::move(this->value_);
            }

            constexpr Value const && operator*() const &&
            {
                assert(this->has_value());

                return std::move(this->value_);
            }

            constexpr Error const & error() const &
            {
                assert(!this->has_value());

                return this->error_.value();
            }

            constexpr Error & error() &
            {
                assert(!this->has_value());

                return this->error_.value();
            }

            constexpr Error && error() &&
            {
                assert(!this->has_value());

                return std::move(this->error_).value();
            }

            constexpr Error const && error() const &&
            {
                assert(!this->has_value());

                return std::move(this->error_).value();
            }

        private:
            bool has_value_ = true;
            union
            {
                char no_init_;
                Value value_;
                unexpected<Error> error_;
            };
        };

        // @todo Покрыть тестом инициализацию Enabler во всех конструкторах
        template <class Value, class Error>
        class expected_base
         : std::conditional_t<std::is_trivially_destructible<Value>{}
                              && std::is_trivially_destructible<Error>{},
                              expected_holder_trivial<Value, Error>,
                              expected_holder<Value, Error>>
         , detail::default_ctor_enabler<std::is_default_constructible<Value>{}>
        {
            using Base = std::conditional_t<std::is_trivially_destructible<Value>{}
                                            && std::is_trivially_destructible<Error>{},
                                            expected_holder_trivial<Value, Error>,
                                            expected_holder<Value, Error>>;

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

            // emplace
            template <class... Args>
            Value & emplace(Args &&... args)
            {
                return Base::emplace(std::forward<Args>(args)...);
            }

            template <class U, class... Args>
            Value & emplace(std::initializer_list<U> inits, Args &&... args)
            {
                return Base::emplace(inits, std::forward<Args>(args)...);
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
         : std::conditional_t<std::is_trivially_destructible<Error>{},
                              expected_holder_trivial<void_wrapper, Error>,
                              expected_holder<void_wrapper, Error>>
        {
            using Base = std::conditional_t<std::is_trivially_destructible<Error>{},
                                            expected_holder_trivial<void_wrapper, Error>,
                                            expected_holder<void_wrapper, Error>>;

        public:
            // Конструкторы
            constexpr expected_base()
             : Base(saga::in_place_t{})
            {}

            expected_base(expected_base const &) = default;

            expected_base(expected_base &&) = default;

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

            // Размещение
            void emplace()
            {
                static_cast<void>(Base::emplace());
            }

            // Немодифицирующие операции
            using Base::operator*;

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
    }
    // namespace detail
}
//namespace saga
/// @endcond

#endif
// Z_SAGA_EXPECTED_DETAIL_HPP_INCLUDED
