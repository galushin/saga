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

/** @file saga/assert.hpp
 @brief Утверждения
*/

// Для этого файла специально не предусмотрены стражи включения
// Определение SAGA_ASSERT_AUDIT может отличаться для каждого заголовочного файла, в зависимости
// наличия определений макросов SAGA_AUDIT и NDEBUG
#undef SAGA_ASSERT_AUDIT

#include <cassert>

#ifdef SAGA_AUDIT
#define SAGA_ASSERT_AUDIT(Expr) assert(Expr)
#else
#define SAGA_ASSERT_AUDIT(Expr) ((void)0)
#endif
// SAGA_AUDIT