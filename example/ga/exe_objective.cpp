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

#include "exe_objective.hpp"

#include <sstream>

namespace BP = boost::process;

namespace
{
    double parse_exe_objective_answer(std::string const & answer)
    {
        // @todo Обработка ошибок
        std::istringstream is(answer);
        std::string preamble;
        double value;

        is >> preamble >> value;

        return value;
    }
}

namespace saga_example
{
    exe_objective::exe_objective(std::string const & path, std::string const & objective_name)
    {
        // @todo Проверить сущестование файла
        this->server_ = BP::child(path + " " + objective_name,
                                  BP::std_out > this->output_,
                                  BP::std_in < this->input_);

        assert(this->server_.running());
    }

    double exe_objective::operator()(Genotype const & arg)
    {
        // Выводим булеву строку
        std::string s_arg;
        for(auto const & x : arg)
        {
            s_arg.push_back(x ? '1' : '0');
        }

        this->input_ << s_arg << std::endl;

        // Считываем результат
        std::string reader;
        std::getline(this->output_, reader);

        return parse_exe_objective_answer(reader);
    }

    exe_objective::~exe_objective()
    {
        this->input_ << std::endl;
        this->server_.wait();
    }
}
// namespace saga_example
