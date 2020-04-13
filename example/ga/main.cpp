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

#include <saga/optimization/ga.hpp>
#include <saga/optimization/optimization_problem.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <random>

namespace
{
    std::map<std::string, std::string>
    parse_ini_file(std::istream & is)
    {
        std::map<std::string, std::string> result;

        std::string reader;
        while(is)
        {
            std::getline(is, reader);

            if(reader.empty())
            {
                continue;
            }

            auto const first = reader.begin();
            auto const last = reader.end();

            auto pos = std::find(first, last, '=');

            if(pos == last)
            {
                // @todo обработать ошибку
                assert(false);
                continue;
            }

            std::string key(first, pos);
            std::string value(pos+1, last);

            result[key] = value;
        }

        return result;
    }
}

int main(int argc, char * argv[])
{
    // Проверка аргументов командной строки
    if(argc < 2)
    {
        // @todo Добавить про то, что требуется указать файл настроек ГА
        std::cout << "This programm takes atleast one argument:\n"
                  << "Path to the file with optimization problem\n";

        return 1;
    }

    // Открываем файл с описанием задачи
    std::ifstream problem_file(argv[1]);

    if(!problem_file)
    {
        std::cerr << "Failed to open optimization problem file: " << argv[1] << "\n";
        return 1;
    }

    // Разбор файла
    auto kvps = parse_ini_file(problem_file);

    // Создаём целевую функцию на основе exe-файла
    saga_example::exe_objective server(kvps.at("exe"), kvps.at("objective"));

    using Genotype = saga_example::Genotype;
    auto objective = [&server](Genotype const & arg) { return server(arg); };

    // Настроить задачу оптимизации
    auto const dim = std::stol(kvps.at("dim"));

    auto problem = saga::make_optimization_problem_boolean(objective, dim);

    // Запустить генератор случайных чисел
    // @todo Добавить возможность задать зерно генератора случайных чисел через аргумент программы
    std::default_random_engine rnd_engine(std::time(nullptr));

    // Настройка генетического алгоритма
    // @todo Возможность настраивать операторы через аргументы программы
    saga::GA_settings<Genotype, saga::ga_boolean_crossover_one_point_fn,
                      saga::selection_ranking> settings;

    settings.population_size = 100;
    settings.max_iterations = 100;

    // @todo Выполнить другие необходимые настройки

    // Выполняем оптимизацию
    auto const result = saga::genetic_algorithm(problem, settings, rnd_engine);

    // Вывести результаты
    // @todo Что улучшить в выводе результатов?
    for(auto const & xy : result)
    {
        std::copy(saga::begin(xy.solution), saga::end(xy.solution),
                  std::ostream_iterator<bool>(std::cout));
        std::cout << "\t" << xy.objective_value << "\n";
    }

    return 0;
}
