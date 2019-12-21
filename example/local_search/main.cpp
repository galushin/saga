/* (c) 2019 СибЮИ МВД России.
Данный файл -- часть программы "Локальный поиск псевдо-булевой оптимзиации"
Никакие гарантии не предоставляются, в том числе гарантия пригодности для любой конкретной цели.
Автор: Галушин Павел Викторович, galushin@gmail.com
*/

#include "plugin.hpp"
#include <saga/optimization/local_search.hpp>
#include <saga/random/iid_distribution.hpp>

#include <boost/dll/import.hpp>

#include <iostream>
#include <map>
#include <random>
#include <string>
#include <valarray>

/*
argv[1] Строка: имя динамической библиотеки, которую нужно загрузить
argv[2] Строка: имя целевой функции из библиотеки argv[1]
argv[3] Целое положительное: размерность вектора аргумента,
argv[4] Строка (необязательный): цель оптимзиации (должно быть min или max)
*/
// @todo Локализация

class my_function_manager
 : public saga::function_manager
{
public:
    using Signature = saga::pseudo_boolean_objective_signature;
    using Name = std::string;

    std::function<Signature> at(Name const & name) const
    {
        return {this->functions_.at(name)};
    }

protected:
    void do_add(Signature * fun, Name const & name) override
    {
        functions_[name] = fun;
    }

private:
    std::map<Name, Signature*> functions_;
};

int main(int argc, char * argv[])
{
    // Чтение аргументов
    // @todo Задокументировать "SAGA function plugin"
    if(argc < 4)
    {
        std::cout << "This program needs at least 3 parameters:\n"
            << "1. path to dynamic lybrary, implementing SAGA function plugin\n"
            << "2. objective function name\n"
            << "3. dimension (positive integer)\n";
        return 0;
    }

    boost::dll::fs::path const lib_path(argv[1]);

    // @todo Проверить, что файл с таким именем действительно существует

    std::string const objective_name(argv[2]);

    // @todo Перехватить исключения
    auto const dim = std::atoi(argv[3]);

    if(dim < 0)
    {
        std::cout << "Dimension must be positive integer, but it's equal to " << dim << "\n";
        return 0;
    }

    // @todo Если цель не задана, то использовать минимизацию

    // Реальная работа
    // @todo Обработка ошибок
    my_function_manager manager;

    boost::shared_ptr<saga::function_regitrar> plugin
        = boost::dll::import<saga::function_regitrar>(lib_path, "registrar");

    plugin->init(manager);

    // @todo Выполнить поиск и вывести результаты
    saga::iid_distribution<std::bernoulli_distribution, std::valarray> init_distr(dim);

    auto objective_impl = manager.at(objective_name);
    auto objective = [&](std::valarray<bool> const & arg)
    {
        return objective_impl(arg.data(), arg.size());
    };

    std::mt19937 random_engine(std::time(nullptr));

    auto const x_init = init_distr(random_engine);
    auto const x_result = saga::local_search_boolean(objective, x_init);

    std::copy(x_result.begin(), x_result.end(), std::ostream_iterator<int>(std::cout));

    return 0;
}
