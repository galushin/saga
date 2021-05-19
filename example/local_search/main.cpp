/* (c) 2019-2021 СибЮИ МВД России.
Данный файл -- часть программы "Локальный поиск псевдо-булевой оптимзиации"
Никакие гарантии не предоставляются, в том числе гарантия пригодности для любой конкретной цели.
Автор: Галушин Павел Викторович, galushin@gmail.com
*/

#include "plugin.hpp"
#include <saga/optimization/local_search.hpp>
#include <saga/random/iid_distribution.hpp>

#include <boost/dll/import.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <iostream>
#include <map>
#include <random>
#include <string>
#include <valarray>

// @todo Проверка дублирования имён функций. Могут ли быть функции с разными сигнатурами, но одним именем?
class my_function_manager
 : public saga::function_manager
{
public:
    using BooleanSignature = saga::pseudo_boolean_objective_signature;
    using IntegerSignature = saga::integer_objective_signature;

    using Name = std::string;

    std::function<BooleanSignature> get_boolean(Name const & name) const
    {
        auto ptr = this->boolean_functions_.find(name);

        if(ptr != boolean_functions_.end())
        {
            return {ptr->second};
        }
        else
        {
            return {};
        }
    }

    std::function<IntegerSignature> get_integer(Name const & name) const
    {
        // @todo Устранить дублирование
        auto ptr = this->integer_functions_.find(name);

        if(ptr != integer_functions_.end())
        {
            return {ptr->second};
        }
        else
        {
            return {};
        }
    }

protected:
    void do_add(BooleanSignature * fun, Name const & name) override
    {
        boolean_functions_[name] = fun;
    }

    void do_add(IntegerSignature * fun, Name const & name) override
    {
        integer_functions_[name] = fun;
    }

private:
    std::map<Name, BooleanSignature *> boolean_functions_;
    std::map<Name, IntegerSignature *> integer_functions_;
};

template <class OStream, class InputRange, class RealType>
void print_optimization_state(OStream & out, InputRange const & solution, RealType objective_value)
{
    using Value = typename std::iterator_traits<decltype(std::begin(solution))>::value_type;
    using OutType = std::conditional_t<std::is_same<Value, bool>{}, int, Value>;

    std::copy(std::begin(solution), std::end(solution), std::ostream_iterator<OutType>(out, " "));
    out << "\t" << objective_value << "\n";
}

namespace pt = boost::property_tree;

int main(int argc, char * argv[])
{
    // Чтение аргументов
    // @todo Для целочисленной задачи оптимизации нужно задавать границы изменения переменных
    if(argc < 2)
    {
        std::cout << "This program needs parameter:\n"
            << "path to file with problem description\n";
        return 0;
    }

    // @todo Проверить, что файл существует
    pt::ptree problem_tree;
    pt::read_xml(argv[1], problem_tree);

    boost::dll::fs::path const lib_path(problem_tree.get<std::string>("problem.plugin"));

    auto const objective_name(problem_tree.get<std::string>("problem.objective"));

    auto const bool_dim = problem_tree.get("problem.boolean_dimension", 0);

    if(bool_dim < 0)
    {
        std::cout << "Dimension must be non negative, but it was: " << bool_dim << "\n";
    }

    auto const goal = problem_tree.get("problem.goal", std::string("min"));

    if(goal != "min" && goal != "max")
    {
        std::cout << "Optimization goal must be 'min' or 'max'\n"
                  << "but it was: '" << goal << "'\n";
        return 1;
    }

    // @todo Считать свойства целочисленных переменных
    auto maybe_ints_subtree = problem_tree.get_child_optional("problem.integer_variables");

    saga::search_space_integer<int> int_space;

    if(!!maybe_ints_subtree)
    {
        for(auto const & node : *maybe_ints_subtree)
        {
            assert(node.first == "variable");
            int_space.add(node.second.get<int>("min"), node.second.get<int>("max"));
        }
    }

    // Реальная работа
    my_function_manager manager;

    boost::shared_ptr<saga::function_regitrar> plugin
        = boost::dll::import<saga::function_regitrar>(lib_path, "registrar");

    plugin->init(manager);

    std::seed_seq seed{std::time(nullptr)};
    std::minstd_rand random_engine(seed);

    if(bool_dim > 0 && int_space.dim() == 0)
    {
    // @todo Вынести в отдельную функцию
        auto objective_impl = manager.get_boolean(objective_name);

        if(!objective_impl)
        {
            std::cout << "Unknown objective name: " << objective_name << "\n";
            return 1;
        }

        auto objective = [&](std::valarray<bool> const & arg)
        {
            saga::boolean_const_span s(arg.size() == 0 ? nullptr : std::addressof(arg[0]), arg.size());

            return objective_impl(s);
        };

        saga::iid_distribution<std::bernoulli_distribution, std::valarray<bool>> init_distr(bool_dim);
        auto const x_init = init_distr(random_engine);

        print_optimization_state(std::cout, x_init, objective(x_init));

        // @todo type-erase функцию сравнения
        auto const result =
            (goal == "min") ? saga::local_search_boolean(objective, x_init)
                            : saga::local_search_boolean(objective, x_init, std::greater<>{});

        print_optimization_state(std::cout, result.solution, result.objective_value);
    }
    else if(bool_dim == 0 && int_space.dim() > 0)
    {
        // @todo Вынести в отдельную функцию
        auto objective_impl = manager.get_integer(objective_name);

        if(!objective_impl)
        {
            std::cout << "Unknown objective name: " << objective_name << "\n";
            return 1;
        }

        using Vector = std::vector<int>;
        auto objective = [&](Vector const & arg)
        {
            saga::integer_const_span s(arg.size() == 0 ? nullptr : std::addressof(arg[0]), arg.size());

            return objective_impl(s);
        };

        Vector x_init;
        for(auto const & var : int_space)
        {
            std::uniform_int_distribution<Vector::value_type> distr(var.min, var.max);
            x_init.push_back(distr(random_engine));
        }

        print_optimization_state(std::cout, x_init, objective(x_init));

        // @todo type-erase функцию сравнения
        auto const result =
            (goal == "min") ? saga::local_search_integer(int_space, objective, x_init)
                            : saga::local_search_integer(int_space, objective, x_init, std::greater<>{});

        print_optimization_state(std::cout, result.solution, result.objective_value);
    }
    else
    {
        std::cout << "Unknown problem type\n";
        return 1;
    }

    return 0;
}
