#include <iostream>
#include <vector>

#include <saga/optimization/test_objectives.hpp>

namespace
{
    // @todo Интерфейс на основе expected?
    std::vector<bool>
    parse_input_string(std::string const & str)
    {
        std::vector<bool> arg;

        for(auto const & c : str)
        {
            if(c != '1' && c != '0')
            {
                throw std::range_error("boolean variable must be 0 or 1");
            }

            arg.push_back(c == '1');
        }

        return arg;
    }

    template <class Objective>
    void process(std::istream & input, std::ostream & output, Objective objective)
    {
        std::string reader;

        while(input)
        {
            std::getline(input, reader);

            if(reader.empty())
            {
                break;
            }

            try
            {
                output << "response: " << objective(parse_input_string(reader)) << "\n";
            }
            catch(std::exception & e)
            {
                std::cerr << e.what();
            }
            catch(...)
            {}
        }
    }
}

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        std::cerr << "This programm requireq argument - name of the objective function";
        // @todo Вывести список доступных функций
        return 1;
    }

    // @todo Настроить целевую функцию
    process(std::cin, std::cout, saga::boolean_manhattan_norm);

    return 0;
}
