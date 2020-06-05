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

 Реализация основана на http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0323r9.html
*/

#include <saga/detail/static_empty_const.hpp>
#include <saga/detail/default_ctor_enabler.hpp>
#include <saga/type_traits.hpp>
#include <saga/utility/operators.hpp>

#include <cassert>
#include <utility>
#include <stdexcept>

namespace saga
{
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

    //@todo Найти лучшее место, так как используется ещё и в optional
    struct in_place_t
    {
        // @todo Почему конструкторе без аргументов должен быть явным?
    };
    // @todo in_place

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

        // @todo Реализовать все остальные ограничения кроме первого
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

            Value const & value() const &
            {
                if(!this->has_value())
                {
                    throw saga::bad_expected_access<Error>(this->error());
                }

                return this->value_;
            }

            Error const & error() const &
            {
                assert(!this->has_value());

                return this->error_.value();
            }

        private:
            bool has_value_ = true;
            union
            {
                Value value_;
                unexpected_type error_;
            };
        };

        template <class Value, class Error>
        class expected_holder_trivial
        {
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

            constexpr Value const & value() const &
            {
                this->throw_if_has_no_value();

                return this->value_;
            }

            constexpr Error const & error() const
            {
                assert(!this->has_value());

                return this->error_.value();
            }

        private:
            constexpr void throw_if_has_no_value() const
            {
                !this->has_value() ? throw saga::bad_expected_access<Error>(this->error()) : 0;
            }

            bool has_value_ = true;
            union
            {
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
            constexpr expected_base() = default;

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

            constexpr bool has_value() const
            {
                return Base::has_value();
            }

            constexpr Value const & value() const &
            {
                return Base::value();
            }

            using Base::error;
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
            constexpr expected_base()
             : Base(saga::in_place_t{})
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

            constexpr bool has_value() const
            {
                return Base::has_value();
            }

            constexpr void value() const
            {
                return void(Base::value());
            }

            using Base::error;
        };
    }

    template <class Value, class Error>
    class expected
     : private detail::expected_base<std::conditional_t<std::is_void<Value>{}, void, Value>, Error>
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

        // @todo Не покрыт тестами
        expected(expected &&);

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

        // Немодифицирующие операции
        constexpr bool has_value() const noexcept
        {
            return Base::has_value();
        }

        using Base::value;

        constexpr error_type const & error() const &
        {
            return Base::error();
        }
    };
}
// namespace saga

#endif
// Z_SAGA_EXPECTED_HPP_INCLUDED
