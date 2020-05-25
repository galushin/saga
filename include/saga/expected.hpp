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

 Реализация основана на http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0323r8.html
*/

#include <saga/detail/static_empty_const.hpp>
#include <saga/type_traits.hpp>
#include <saga/utility/operators.hpp>

#include <utility>
#include <stdexcept>

namespace saga
{
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

        // @todo explicit
        template <class U, class... Args,
                  class = std::enable_if_t<std::is_constructible<Error, std::initializer_list<U> &, Args...>{}>>
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

        // @todo Конструктор на основе unexpected<Other> const &
        // @todo Конструктор на основе unexpected<Other> &&

        constexpr unexpected & operator=(unexpected const &) = default;
        constexpr unexpected & operator=(unexpected &&) = default;

        // @todo Значение по умолчанию для типа - когда есть неоднозначность?
        template <class Err>
        constexpr std::enable_if_t<std::is_assignable<Error &, Err const &>{}, unexpected &>
        operator=(unexpected<Err> const & other)
        {
            this->value_ = other.value();
            return *this;
        }

        // @todo Значение по умолчанию для типа
        template <class Err>
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
    // @todo Подсказка для вывода типа шаблонных параметров

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
}
// namespace saga

#endif
// Z_SAGA_EXPECTED_HPP_INCLUDED
