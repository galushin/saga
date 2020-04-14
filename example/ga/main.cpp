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

    std::map<std::string, std::string>
    open_and_parse_ini_file(std::string const & path)
    {
        std::ifstream file(path);

        if(!file)
        {
            throw std::runtime_error("Failed to open file " + path);
        }

        return parse_ini_file(file);
    }

    int do_main(std::vector<std::string> const & cmd_args)
    {
        // Открываем и разбираем файл с описанием задачи
        auto const problem_kvps = open_and_parse_ini_file(cmd_args.at(1));

        // Создаём целевую функцию на основе exe-файла
        saga_example::exe_objective server(problem_kvps.at("exe"), problem_kvps.at("objective"));

        using Genotype = saga_example::Genotype;
        auto objective = [&server](Genotype const & arg) { return server(arg); };

        // Настроить задачу оптимизации
        auto const dim = std::stol(problem_kvps.at("dim"));

        auto problem = saga::make_optimization_problem_boolean(objective, dim);

        // Запустить генератор случайных чисел
        // @todo Добавить возможность задать зерно генератора случайных чисел через аргумент программы
        std::default_random_engine rnd_engine(std::time(nullptr));

        // Настройка генетического алгоритма
        auto const settings_kvps = open_and_parse_ini_file(cmd_args.at(2));

        saga::GA_settings<Genotype, saga::ga_boolean_crossover_one_point_fn,
                          saga::selection_ranking> settings;

        settings.population_size = std::stol(settings_kvps.at("population_size"));
        settings.max_iterations = std::stol(settings_kvps.at("max_iterations"));

        // @todo Настроить операторы и вероятность мутации на основе файла настроек

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
}
// namespace

int main(int argc, char * argv[])
{
    // Проверка аргументов командной строки
    if(argc < 3)
    {
        std::cout << "This programm takes atleast two arguments:\n"
                  << "1. Path to the file with optimization problem\n"
                  << "2. Path to the file with genetic algorithm settings\n";

        return 0;
    }

    try
    {
        std::vector<std::string> cmd_args(argv, argv + argc);

        return do_main(cmd_args);
    }
    catch(std::exception & e)
    {
        std::cerr << e.what() << "\n";
    }
}
