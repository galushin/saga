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

#ifndef Z_SAGA_EXAMPLE_GA_EXE_OBJECTIVE_H_INCLUDED
#define Z_SAGA_EXAMPLE_GA_EXE_OBJECTIVE_H_INCLUDED

/* Без следующего определения Boost.Process не компилируетя
https://stackoverflow.com/questions/59337197/boostprocess-on-windows-with-mingw
*/
#ifndef __kernel_entry
    #define __kernel_entry
#endif
#include <boost/process.hpp>

#include <string>
#include <valarray>

namespace saga_example
{
    using Genotype = std::valarray<bool>;

    class exe_objective
    {
    public:
        // @todo Перейти на использование std::string_view?
        exe_objective(std::string const & path, std::string const & objective_name);

        ~exe_objective();

        double operator()(Genotype const & arg);

    private:
        boost::process::ipstream output_;
        boost::process::opstream input_;
        boost::process::child server_;
    };
}
// namespace saga_example


#endif
// Z_SAGA_EXAMPLE_GA_EXE_OBJECTIVE_H_INCLUDED
