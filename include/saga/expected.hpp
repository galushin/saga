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

#ifndef Z_SAGA_EXPECTED_HPP_INCLUDED
#define Z_SAGA_EXPECTED_HPP_INCLUDED

/** @file saga/expected.hpp
 @brief Класс для представления ожидаемого объекта. Объект типа expected<T, E> содержит объект
 типа T или объект типа unexpected<E> и управляет временем жизни содержащихся в нём объектов.
 @todo Ограничения на типы аргументов шаблона
 @todo Проверить размер объектов, довести до минимума

 Реализация основана на http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0323r9.html
*/

#include <saga/expected/bad_expected_access.hpp>
#include <saga/expected/detail.hpp>
#include <saga/expected/unexpected.hpp>

#include <saga/utility/in_place.hpp>
#include <saga/utility/operators.hpp>

#include <utility>

namespace saga
{
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
     : public detail::expected_base<saga::conditional_t<std::is_void<Value>{}, void, Value>, Error>
     , detail::copy_ctor_enabler<detail::expected_is_copyable<Value, Error>()>
     , detail::move_ctor_enabler<detail::expected_is_moveable<Value, Error>()>
    {
        using Base
            = detail::expected_base<saga::conditional_t<std::is_void<Value>{}, void, Value>, Error>;

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
        expected(expected &&) = default;

        template <class OtherValue, class OtherError
                  , std::enable_if_t<detail::expected_has_ctor_from_other<Value, Error, OtherValue, OtherError>()> * = nullptr
                  , std::enable_if_t<detail::expected_explicit_from_other<Value, Error, OtherValue, OtherError>()> * = nullptr>
        // @todo Ограничения типа
        // @todo constexpr
        explicit expected(expected<OtherValue, OtherError> const & rhs)
         : Base(rhs)
        {}

        template <class OtherValue, class OtherError
                  , std::enable_if_t<detail::expected_has_ctor_from_other<Value, Error, OtherValue, OtherError>()> * = nullptr
                  , std::enable_if_t<!detail::expected_explicit_from_other<Value, Error, OtherValue, OtherError>()> * = nullptr>
        // @todo Ограничения типа
        // @todo constexpr
        expected(expected<OtherValue, OtherError> const & rhs)
         : Base(rhs)
        {}

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

        // Присваивание
        /** @brief Копирующее присваивание
        @todo If any exception is thrown, bool(*this) and bool(rhs) remain unchanged.
        @todo If an exception is thrown during the call to T's or unexpected<E>'s copy constructor, no effect.
        @todo If an exception is thrown during the call to T's or unexpected<E>'s copy assignment, the state of its contained value is as defined by the exception safety guarantee of T's or unexpected<E>'s copy assignment.
        @todo This operator shall be defined as deleted unless: T is cv void and is_copy_assignable_v<E> is true and is_copy_constructible_v<E> is true; or
        T is not cv void and is_copy_assignable_v<T> is true and is_copy_constructible_v<T> is true and is_copy_assignable_v<E> is true and is_copy_constructible_v<E> is true and (is_nothrow_move_constructible_v<E> is true or is_nothrow_move_constructible_v<T> is true).
        */
        expected & operator=(expected const & rhs) = default;

        /** @brief Присваивание с перемещением
        @todo Спецификация noexcept
        @todo If any exception is thrown, bool(*this) and bool(rhs) remain unchanged.
        @todo If an exception is thrown during the call to T's move constructor, no effect.
        @todo If an exception is thrown during the call to T's move assignment, the state of its contained value is as defined by the exception safety guarantee of T's copy assignment.
        @todo If an exception is thrown during the call to E's move assignment, the state of its contained unex is as defined by the exception safety guarantee of E's copy assignment.
        @todo This operator is defined as deleted unless: T is cv void and is_nothrow_move_constructible_v<E> is true and is_nothrow_move_assignable_v<E> is true; or
        T is not cv void and is_move_constructible_v<T> is true and is_move_assignable_v<T> is true and is_nothrow_move_constructible_v<E> is true and is_nothrow_move_assignable_v<E> is true.
        */
        expected & operator=(expected && rhs) = default;

        /** @brief Присваивание значения
        @todo Значение по умолчанию для шаблонного параметра
        @todo Добавить ограничения типа std::is_void<Value>{} == false;
        @todo Добавить ограничение типов: is_same_v<expected<T,E>, remove_cvref_t<U>> is false
        @todo Добавить ограничение типов: conjunction_v<is_scalar<T>, is_same<T, decay_t<U>>> is false
        @todo Добавить ограничение типов: is_assignable_v<T&, U> is true
        @todo Добавить ограничение типов: is_nothrow_move_constructible_v<E> is true.
        @todo If any exception is thrown, bool(*this) remains unchanged.
        @todo If an exception is thrown during the call to T's constructor, no effect.
        @todo If an exception is thrown during the call to T's copy assignment, the state of its contained value is as defined by the exception safety guarantee of T's copy assignment.
        */
        template <class Arg,
                  class = std::enable_if_t<std::is_constructible<Value, Arg>{}>>
        expected & operator=(Arg && arg)
        {
            // @todo Спецификация operator= отличается от emplace. Изменить?
            this->emplace(std::forward<Arg>(arg));

            return *this;
        }

        /** Присваивание unexpected
        @todo Значение по умолчанию для шаблонного параметра
        @todo Ограничения типа: is_nothrow_copy_constructible_v<E> и is_copy_assignable_v<E>
        @todo Доказать, что если возникает исключение, то bool(*this) не изменяется
        */
        template <class Arg>
        expected & operator=(unexpected<Arg> const & unex)
        {
            this->assign_error(unex.value());

            return *this;
        }

        /** @brief Присваивание временного unexpected
        @todo Значение по умолчанию для шаблонного параметра
        @todo Ограничения типа: is_nothrow_move_constructible_v<E> и is_move_assignable_v<E>.
        @todo Доказать, что если возникает исключение, то bool(*this) не изменяется
        */
        template <class Arg>
        expected & operator=(unexpected<Arg> && unex)
        {
            this->assign_error(std::move(unex.value()));

            return *this;
        }

        // emplace
        using Base::emplace;

        // Обмен
        /**
        @todo Нормально ли так задать ограничения?
        */
        void swap(expected & rhs) noexcept(detail::is_expected_nothrow_swappable<Value, Error>())
        {
            Base::swap(rhs);
        }

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

    /**
    @todo Можно ли сделать скрытым другом?
    */
    template <class Value, class Error>
    std::enable_if_t<detail::expected_is_swappable<Value, Error>()>
    swap(expected<Value, Error> & lhs, expected<Value, Error> & rhs)
        noexcept(noexcept(lhs.swap(rhs)))
    {
        return lhs.swap(rhs);
    }

    template <class Value, class Error>
    std::enable_if_t<!detail::expected_is_swappable<Value, Error>()>
    swap(expected<Value, Error> & lhs, expected<Value, Error> & rhs) = delete;
}
// namespace saga

#endif
// Z_SAGA_EXPECTED_HPP_INCLUDED
