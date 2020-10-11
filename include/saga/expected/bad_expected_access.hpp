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

#ifndef Z_SAGA_EXPECTED_BAD_EXPECTED_ACCESS_HPP_INCLUDED
#define Z_SAGA_EXPECTED_BAD_EXPECTED_ACCESS_HPP_INCLUDED

/** @file saga/expected/bad_expected_access.hpp
 @brief Тип исключения для сообщения о попытке доступа к ожидаемому значению, когда оно не доступно
*/

#include <stdexcept>
#include <utility>

namespace saga
{
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
}
// namespace saga


#endif
// Z_SAGA_EXPECTED_BAD_EXPECTED_ACCESS_HPP_INCLUDED
