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

        // @todo unexpected(unexpected &&);

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

        // @todo Конструктор на основе одного аргумента
        // @todo Конструктор на основе unexpected<Other> const &
        // @todo Конструктор на основе unexpected<Other> &&

        // @todo Присваивание

        // @todo Доступ к значению
        constexpr Error const & value() const &
        {
            return this->value_;
        }
        // @todo value() &
        // @todo value() &&
        // @todo value() const &&

        // @todo Обмен - член и друг
        // @todo Проверка на равенство

    private:
        Error value_;
    };
    // @todo Подсказка для вывода типа шаблонных параметров

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
