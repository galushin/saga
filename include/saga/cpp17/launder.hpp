/* (c) 2021 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_CPP17_LAUNDER_HPP_INCLUDED
#define Z_SAGA_CPP17_LAUNDER_HPP_INCLUDED

/** @file saga/cpp17/launder.hpp
 @brief Обёртка, вызывающая std::launder там, где она доступна.
*/

#include <new>

namespace saga
{
#if __cpp_lib_launder >= 201606
    using std::launder;
#else
    template <class T>
    constexpr T * launder(T * ptr) noexcept
    {
        // @tood Можно ли реализовать правильно, но оставить constexpr в С++14?
        return ptr;
    }

    // Стандарт явно запрещает следующие варианты:
    void launder(void*) = delete;
    void launder(void const*) = delete;
    void launder(void volatile*) = delete;
    void launder(void const volatile*) = delete;
    template <typename T, typename... Args>
    void launder(T (*)(Args...)) = delete;
#endif
}
// namespace saga

#endif
// Z_SAGA_CPP17_LAUNDER_HPP_INCLUDED
