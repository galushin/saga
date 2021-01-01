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

#ifndef Z_SAGA_DEFAULT_DEFAULT_CTOR_ENABLER_HPP_INCLUDED
#define Z_SAGA_DEFAULT_DEFAULT_CTOR_ENABLER_HPP_INCLUDED

namespace saga
{
    namespace detail
    {
        template <bool Enable>
        struct default_ctor_enabler
        {
            default_ctor_enabler() = default;

            constexpr explicit default_ctor_enabler(int)
            {}
        };

        template <>
        struct default_ctor_enabler<false>
        {
            default_ctor_enabler() = delete;

            constexpr explicit default_ctor_enabler(int)
            {}
        };
    }
    // namespace details
}
// namespace saga

#endif
// Z_SAGA_DEFAULT_DEFAULT_CTOR_ENABLER_HPP_INCLUDED
