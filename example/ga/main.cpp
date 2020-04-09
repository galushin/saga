#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>

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

            auto const first = reader.begin();
            auto const last = reader.end();

            auto pos = std::find(first, last, '=');

            if(pos == last)
            {
                // @todo обработать ошибку
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

    // @todo Настроить задачу оптимизации, подключиться к серверу целевых функций
    // @todo Выполнить оптимизацию
    // @todo Вывести результаты

    return 0;
}
