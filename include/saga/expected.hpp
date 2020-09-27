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

#ifndef Z_SAGA_EXPECTED_HPP_INCLUDED
#define Z_SAGA_EXPECTED_HPP_INCLUDED

/** @file saga/expected.hpp
 @brief Класс для представления ожидаемого объекта. Объект типа expected<T, E> содержит объект
 типа T или объект типа unexpected<E> и управляет временем жизни содержащихся в нём объектов.
 @todo Ограничения на типы аргументов шаблона
 @todo Проверить размер объектов, довести до минимума

 Реализация основана на http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0323r9.html
*/

#include <saga/detail/static_empty_const.hpp>
#include <saga/detail/default_ctor_enabler.hpp>
#include <saga/type_traits.hpp>
#include <saga/utility/in_place.hpp>
#include <saga/utility/operators.hpp>

#include <cassert>
#include <utility>
#include <stdexcept>

namespace saga
{
    template <class Value, class Error>
    class expected;

    namespace detail
    {
        template <class From, class To>
        constexpr bool can_convert()
        {
            return std::is_constructible<To, From&>{}
                    or std::is_constructible<To, From>{}
                    or std::is_constructible<To, From const &>{}
                    or std::is_constructible<To, From const>{}
                    or std::is_convertible<From &, To>{}
                    or std::is_convertible<From, To>{}
                    or std::is_convertible<From const &, To>{}
                    or std::is_convertible<From const, To>{};
        }
    }

    template <class Error>
    class unexpected
    {
    public:
        // Конструкторы
        constexpr unexpected(unexpected const &) = default;

        constexpr unexpected(unexpected &&) = default;

        template <class... Args,
                  class = std::enable_if_t<std::is_constructible<Error, Args...>{}>>
        constexpr explicit unexpected(saga::in_place_t, Args&&... args)
         : value_(std::forward<Args>(args)...)
        {}

        template <class U, class... Args,
                  class = std::enable_if_t<std::is_constructible<Error, std::initializer_list<U> &, Args...>{}>>
        // @todo explicit
        constexpr unexpected(saga::in_place_t, std::initializer_list<U> init, Args &&... args)
         : value_(init, std::forward<Args>(args)...)
        {}

        template <class Err = Error,
                  class = std::enable_if_t<std::is_constructible<Error, Err>{}>,
                  class = std::enable_if_t<!std::is_same<saga::remove_cvref_t<Err>, in_place_t>{}>,
                  class = std::enable_if_t<!std::is_same<saga::remove_cvref_t<Err>, unexpected>{}>>
        constexpr explicit unexpected(Err && value)
         : value_(std::forward<Err>(value))
        {}

        // Конструктор на основе unexpected<Other> const &
        template <class Err,
                  std::enable_if_t<std::is_convertible<Err const &, Error>{}, bool> = false,
                  class = std::enable_if_t<std::is_constructible<Error, Err>{}>,
                  class = std::enable_if_t<!detail::can_convert<unexpected<Err>, Error>()>>
        constexpr unexpected(unexpected<Err> const & other)
         : value_(other.value())
        {}

        template <class Err,
                  std::enable_if_t<!std::is_convertible<Err const &, Error>{}, bool> = false,
                  class = std::enable_if_t<std::is_constructible<Error, Err>{}>,
                  class = std::enable_if_t<!detail::can_convert<unexpected<Err>, Error>()>>
        constexpr explicit unexpected(unexpected<Err> const & other)
         : value_(other.value())
        {}

        // Конструктор на основе unexpected<Other> &&
        template <class Err,
                  std::enable_if_t<std::is_convertible<Err, Error>{}, bool> = false,
                  class = std::enable_if_t<std::is_constructible<Error, Err>{}>,
                  class = std::enable_if_t<!detail::can_convert<unexpected<Err>, Error>()>>
        constexpr unexpected(unexpected<Err> && other)
         : value_(std::move(other).value())
        {}

        template <class Err,
                  std::enable_if_t<!std::is_convertible<Err, Error>{}, bool> = false,
                  class = std::enable_if_t<std::is_constructible<Error, Err>{}>,
                  class = std::enable_if_t<!detail::can_convert<unexpected<Err>, Error>()>>
        constexpr explicit unexpected(unexpected<Err> && other)
         : value_(std::move(other).value())
        {}

        constexpr unexpected & operator=(unexpected const &) = default;
        constexpr unexpected & operator=(unexpected &&) = default;

        template <class Err = Error>
        constexpr std::enable_if_t<std::is_assignable<Error &, Err const &>{}, unexpected &>
        operator=(unexpected<Err> const & other)
        {
            this->value_ = other.value();
            return *this;
        }

        template <class Err = Error>
        constexpr std::enable_if_t<std::is_assignable<Error &, Err>{}, unexpected &>
        operator=(unexpected<Err> && other)
        {
            this->value_ = std::move(other).value();
            return *this;
        }

        // Доступ к значению
        constexpr Error const & value() const & noexcept
        {
            return this->value_;
        }

        constexpr Error & value() & noexcept
        {
            return this->value_;
        }

        constexpr Error && value() && noexcept
        {
            return std::move(this->value_);
        }

        constexpr Error const && value() const && noexcept
        {
            return std::move(this->value_);
        }

        void swap(unexpected & that) noexcept(saga::is_nothrow_swappable<Error>::value)
        {
            using std::swap;
            swap(this->value_, that.value_);
        }

        template <class Error2>
        friend
        constexpr bool operator==(unexpected<Error> const & lhs, unexpected<Error2> const & rhs)
        {
            return lhs.value() == rhs.value();
        }

        template <class Error2>
        friend
        constexpr bool operator!=(unexpected<Error> const & lhs, unexpected<Error2> const & rhs)
        {
            return !(lhs == rhs);
        }

    private:
        Error value_;
    };

    // @todo Можно ли превратить в скрытых друзей?
    template <class Error>
    std::enable_if_t<saga::is_swappable<Error>{}, void>
    swap(unexpected<Error> & lhs, unexpected<Error> & rhs) noexcept(noexcept(lhs.swap(rhs)))
    {
        lhs.swap(rhs);
    }

    template <class Error>
    std::enable_if_t<!saga::is_swappable<Error>{}, void>
    swap(unexpected<Error> & lhs, unexpected<Error> & rhs) = delete;

#if __cpp_deduction_guides >= 201703
    template <class Error>
    unexpected(Error) -> unexpected<Error>;
#endif
// __cpp_deduction_guides

    template <class Error>
    constexpr unexpected<std::decay_t<Error>> make_unexpected(Error && error)
    {
        return unexpected<std::decay_t<Error>>(std::forward<Error>(error));
    }

    template <class Error>
    class bad_expected_access;

    template <>
    class bad_expected_access<void>
     : public std::exception
    {
    public:
        bad_expected_access() = default;
    };

    template <class Error>
    class bad_expected_access
     : public bad_expected_access<void>
    {
    public:
        explicit bad_expected_access(Error error)
         : bad_expected_access<void>{}
         , error_(std::move(error))
        {}

        Error & error() &
        {
            return this->error_;
        }

        Error const & error() const &
        {
            return this->error_;
        }

        Error && error() &&
        {
            return std::move(this->error_);
        }

        Error const && error() const &&
        {
            return std::move(this->error_);
        }

        char const * what() const noexcept override
        {
            return "saga::bad_expected_access";
        }

    private:
        Error error_;
    };

    struct unexpect_t
    {
        // @todo Покрыть explicit проверками
        unexpect_t() = default;
    };

    constexpr auto const & unexpect = detail::static_empty_const<unexpect_t>::value;

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

        template <class Value, class Error>
        class expected_holder_trivial
        {
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
            {}

            template <class U, class... Args>
            constexpr explicit expected_base(unexpect_t,
                                             std::initializer_list<U> inits, Args &&... args)
             : Base(unexpect_t{}, inits, std::forward<Args>(args)...)
            {}

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

    /** @brief Вспомогательный класс для представения ожидаемого объекта
    @todo Нет ли здесь потенциальной проблемы с повторяющимися базовыми классами? Вдруг кто-то
    унаследует от двух expected.
    @todo Нет ли здесь проблемы с разрастанием размера класса? Может быть объединить
    move_ctor_enabler и copy_ctor_enabler в один класс? К тому же можно рассмотреть передачу
    базового класса
    @todo expected_base не должен беспокоиться о том, что Value может быть void, выделить это в
    отдельный CRTP-класс
    */
    template <class Value, class Error>
    class expected
     : public detail::expected_base<std::conditional_t<std::is_void<Value>{}, void, Value>, Error>
     , detail::copy_ctor_enabler<(std::is_void<Value>{} || std::is_copy_constructible<Value>{}) && std::is_copy_constructible<Error>{}>
     , detail::move_ctor_enabler<(std::is_void<Value>{} || std::is_move_constructible<Value>{}) && std::is_move_constructible<Error>{}>
    {
        using Base = detail::expected_base<std::conditional_t<std::is_void<Value>{}, void, Value>, Error>;

    public:
        // Типы
        using value_type = Value;
        using error_type = Error;
        using unexpected_type = unexpected<Error>;

        template <class Other>
        using rebind = expected<Other, error_type>;

        // Конструкторы
        expected() = default;
        expected(expected const &) = default;

        // @todo Не покрыт тестами
        // @todo noexcept
        expected(expected &&) = default;

        template <class Arg,
                  std::enable_if_t<detail::expected_has_ctor_from_arg<Value, Error, Arg>()> * = nullptr,
                  std::enable_if_t<std::is_convertible<Arg&&, Value>{}> * = nullptr>
        // @todo Значение для Arg по умолчанию
        // @todo Ограничения типа
        constexpr expected(Arg && arg)
         : expected(saga::in_place_t{}, std::forward<Arg>(arg))
        {}

        template <class Arg,
                  std::enable_if_t<detail::expected_has_ctor_from_arg<Value, Error, Arg>()> * = nullptr,
                  std::enable_if_t<!std::is_convertible<Arg&&, Value>{}> * = nullptr>
        // @todo Значение для Arg по умолчанию
        // @todo Ограничения типа кроме второго не покрыты проверками
        constexpr explicit expected(Arg && arg)
         : expected(saga::in_place_t{}, std::forward<Arg>(arg))
        {}

        // @todo Значение по умолчанию не покрыто тестами
        // @todo Ограничения типа кроме второго не покрыты проверками
        template <class OtherError,
                  std::enable_if_t<std::is_constructible<Error, OtherError const &>{}> * = nullptr,
                  std::enable_if_t<std::is_convertible<OtherError const &, Error>{}> * = nullptr>
        constexpr expected(saga::unexpected<OtherError> const & unex)
         : expected(saga::unexpect, unex.value())
        {}

        // @todo Значение по умолчанию не покрыто тестами
        template <class OtherError,
                  std::enable_if_t<std::is_constructible<Error, OtherError const &>{}> * = nullptr,
                  std::enable_if_t<!std::is_convertible<OtherError const &, Error>{}> * = nullptr>
        constexpr explicit expected(saga::unexpected<OtherError> const & unex)
         : expected(saga::unexpect, unex.value())
        {}

        // @todo Значение по умолчанию для OtherError
        template <class OtherError,
                  std::enable_if_t<std::is_constructible<Error, OtherError&&>{}> * = nullptr,
                  std::enable_if_t<std::is_convertible<OtherError &&, Error>{}> * = nullptr>
        constexpr expected(unexpected<OtherError> && unex)
            noexcept(std::is_nothrow_constructible<Error, OtherError &&>{})
         : Base(saga::unexpect, std::move(unex).value())
        {}

        // @todo Значение по умолчанию для OtherError
        template <class OtherError,
                  std::enable_if_t<std::is_constructible<Error, OtherError&&>{}> * = nullptr,
                  std::enable_if_t<!std::is_convertible<OtherError &&, Error>{}> * = nullptr>
        explicit constexpr expected(unexpected<OtherError> && unex)
            noexcept(std::is_nothrow_constructible<Error, OtherError &&>{})
         : Base(saga::unexpect, std::move(unex).value())
        {}

        template <class... Args,
                  class = std::enable_if_t<(std::is_void<Value>{} && sizeof...(Args) == 0)
                                           || (!std::is_void<Value>{}
                                               && std::is_constructible<Value, Args...>{})>>
        constexpr explicit expected(saga::in_place_t, Args &&... args)
         : Base(saga::in_place_t{}, std::forward<Args>(args)...)
        {}

        template <class U, class... Args,
                  class = std::enable_if_t<!std::is_void<Value>{}
                                            && std::is_constructible<Value, std::initializer_list<U> &, Args...>{}>>
        // @todo explicit
        constexpr expected(in_place_t, std::initializer_list<U> inits, Args &&... args)
         : Base(saga::in_place_t{}, inits, std::forward<Args>(args)...)
        {}

        template <class... Args,
                  class = std::enable_if_t<std::is_constructible<Error, Args...>{}>>
        constexpr explicit expected(unexpect_t, Args &&... args)
         : Base(unexpect_t{}, std::forward<Args>(args)...)
        {}

        template <class U, class... Args,
                  class = std::enable_if_t<std::is_constructible<Error, std::initializer_list<U> &, Args...>{}>>
        // @todo explcit
        constexpr expected(unexpect_t, std::initializer_list<U> inits, Args &&... args)
         : Base(unexpect_t{}, inits, std::forward<Args>(args)...)
        {}

        // Свойства
        using Base::operator*;

        constexpr explicit operator bool() const noexcept
        {
            return this->has_value();
        }

        constexpr bool has_value() const noexcept
        {
            return Base::has_value();
        }

        using Base::value;
        using Base::error;

        template <class Other>
        constexpr Value value_or(Other && other) const &
        {
            static_assert(std::is_convertible<Other, Value>{}, "");

            return this->has_value() ? **this : static_cast<Value>(std::forward<Other>(other));
        }

        template <class Other>
        constexpr Value value_or(Other && other) &&
        {
            static_assert(std::is_convertible<Other, Value>{}, "");

            return this->has_value() ? std::move(**this)
                                     : static_cast<Value>(std::forward<Other>(other));
        }

        // Операторы равно и не равно
        template <class OtherValue, class OtherError>
        friend constexpr
        bool operator==(expected const & lhs, expected<OtherValue, OtherError> const & rhs)
        {
            if(lhs.has_value() != rhs.has_value())
            {
                return false;
            }
            else if(!lhs.has_value())
            {
                return lhs.error() == rhs.error();
            }
            else
            {
                return lhs.compare_value_with(rhs);
            }
        }

        template <class OtherValue, class OtherError>
        friend constexpr
        bool operator!=(expected const & lhs, expected<OtherValue, OtherError> const & rhs)
        {
            return !(lhs == rhs);
        }

        // Сравнение со значением
        template <class OtherValue>
        friend constexpr auto operator==(expected const & obj, OtherValue const & value)
        -> std::enable_if_t<!detail::is_expected<OtherValue>{}, bool>
        {
            return obj.has_value() ? *obj == value : false;
        }

        template <class OtherValue>
        friend constexpr auto operator==(OtherValue const & value, expected const & obj)
        -> std::enable_if_t<!detail::is_expected<OtherValue>{}, bool>
        {
            return obj.has_value() ? value == *obj : false;
        }

        template <class OtherValue>
        friend constexpr auto operator!=(expected const & obj, OtherValue const & value)
        -> std::enable_if_t<!detail::is_expected<OtherValue>{}, bool>
        {
            return !(obj == value);
        }

        template <class OtherValue>
        friend constexpr auto operator!=(OtherValue const & value, expected const & obj)
        -> std::enable_if_t<!detail::is_expected<OtherValue>{}, bool>
        {
            return !(value == obj);
        }

        // Сравнение с unexpected
        template <class OtherError>
        friend constexpr bool operator==(expected const & obj, unexpected<OtherError> const & unex)
        {
            return obj.has_value() ? false : obj.error() == unex.value();
        }

        template <class OtherError>
        friend constexpr bool operator==(unexpected<OtherError> const & unex, expected const & obj)
        {
            return obj.has_value() ? false : obj.error() == unex.value();
        }

        template <class OtherError>
        friend constexpr bool operator!=(expected const & obj, unexpected<OtherError> const & unex)
        {
            return !(obj == unex);
        }

        template <class OtherError>
        friend constexpr bool operator!=(unexpected<OtherError> const & unex, expected const & obj)
        {
            return !(unex == obj);
        }
    };
}
// namespace saga

#endif
// Z_SAGA_EXPECTED_HPP_INCLUDED
