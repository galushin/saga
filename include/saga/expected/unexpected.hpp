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

#ifndef Z_SAGA_EXPECTED_UNEXPECTED_HPP_INCLUDED
#define Z_SAGA_EXPECTED_UNEXPECTED_HPP_INCLUDED

/** @file saga/expected/unexpected.hpp
 @brief Шаблон класса для описания неожиданного объекта
*/

#include <saga/utility/in_place.hpp>
#include <saga/type_traits.hpp>

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

    struct unexpect_t
    {
        // @todo Покрыть explicit проверками
        unexpect_t() = default;
    };

    namespace
    {
        constexpr auto const & unexpect = detail::static_empty_const<unexpect_t>::value;
    }
}
// namespace saga

#endif
// Z_SAGA_EXPECTED_UNEXPECTED_HPP_INCLUDED
