/* (c) 2024 Галушин Павел Викторович, galushin@gmail.com

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

// Тестируемый заголовочный файл
#include <saga/compare.hpp>

// Инфраструктура тестирования
#include "saga_test.hpp"
#include <catch2/catch_amalgamated.hpp>

// Вспомогательные файлы

// Тесты
static_assert( saga::in_order(1, 2));
static_assert( saga::in_order(1, 1));
static_assert(!saga::in_order(2, 1));

static_assert(!saga::out_of_order(1, 2));
static_assert(!saga::out_of_order(1, 1));
static_assert( saga::out_of_order(2, 1));

static_assert(!saga::in_order(1, 2, std::greater<>{}));
static_assert( saga::in_order(1, 1, std::greater<>{}));
static_assert( saga::in_order(2, 1, std::greater<>{}));

static_assert( saga::out_of_order(1, 2, std::greater<>{}));
static_assert(!saga::out_of_order(1, 1, std::greater<>{}));
static_assert(!saga::out_of_order(2, 1, std::greater<>{}));
