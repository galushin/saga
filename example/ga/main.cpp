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

    template <class Genotype, class Objective>
    auto make_objective_compare(Objective const & objective, std::string const & goal)
    {
        using Objective_value = decltype(objective(std::declval<Genotype const &>()));

        std::function<bool(Objective_value const &, Objective_value const &)> cmp;

        if(goal == "min")
        {
            cmp = std::less<>{};
        }
        else if(goal == "max")
        {
            cmp = std::greater<>{};
        }
        else
        {
            throw std::domain_error("Unknown optimization goal " + goal);
        }

        return cmp;
    }

    double parse_mutation_strength(std::string const & str)
    {
        if(str == "weak")
        {
            return 0.5;
        }
        else if(str == "normal")
        {
            return 1.0;
        }
        else if(str == "strong")
        {
            return 2.0;
        }
        else
        {
            return std::stod(str);
        }
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

        auto objective_cmp = make_objective_compare<Genotype>(objective, problem_kvps.at("goal"));

        auto problem = saga::make_optimization_problem_boolean(objective, dim, objective_cmp);

        // Запустить генератор случайных чисел
        // @todo Добавить возможность задать зерно генератора случайных чисел через аргумент программы
        std::default_random_engine rnd_engine(std::time(nullptr));

        // Настройка генетического алгоритма
        auto const settings_kvps = open_and_parse_ini_file(cmd_args.at(2));

        saga::GA_settings<Genotype, saga::ga_boolean_crossover_one_point_fn,
                          saga::selection_ranking> settings;

        settings.population_size = std::stol(settings_kvps.at("population_size"));
        settings.max_iterations = std::stol(settings_kvps.at("max_iterations"));
        settings.mutation_strength = parse_mutation_strength(settings_kvps.at("mutation strength"));

        // @todo Настроить операторы на основе файла настроек

        // Выполняем оптимизацию
        auto const result = saga::genetic_algorithm(problem, settings, rnd_engine);

        // Вывести результаты
        auto const best_pos
            = std::min_element(std::begin(result), std::end(result),
                               saga::compare_by_objective_value(objective_cmp));
        assert(!result.empty());
        auto const best_objective_value = best_pos->objective_value;

        for(auto const & xy : result)
        {
            if(!objective_cmp(best_objective_value, xy.objective_value))
            {
                std::copy(saga::begin(xy.solution), saga::end(xy.solution),
                          std::ostream_iterator<bool>(std::cout));
                std::cout << "\t" << xy.objective_value << "\n";
            }
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
