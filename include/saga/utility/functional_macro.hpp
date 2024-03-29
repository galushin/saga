/* (c) 2022 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_SAGA_FUNCTIONAL_MACRO_HPP_INCLUDED
#define Z_SAGA_FUNCTIONAL_MACRO_HPP_INCLUDED

/** @file saga/utility/functional_macro.hpp
 @brief Макросы, облегчающие работу с функциями (особенно с перегруженными и обобщёнными)
*/

#include <utility>

#define SAGA_RETURNS(...)\
    noexcept(noexcept(__VA_ARGS__)) -> decltype(__VA_ARGS__) { return __VA_ARGS__; }

#define SAGA_FWD(...) std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)

#define SAGA_OVERLOAD_SET(fun) [](auto &&... args) SAGA_RETURNS(fun(SAGA_FWD(args)...))

#endif
// Z_SAGA_FUNCTIONAL_MACRO_HPP_INCLUDED
