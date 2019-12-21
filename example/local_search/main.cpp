/* (c) 2019 СибЮИ МВД России.
Данный файл -- часть программы "Локальный поиск псевдо-булевой оптимзиации"
Никакие гарантии не предоставляются, в том числе гарантия пригодности для любой конкретной цели.
Автор: Галушин Павел Викторович, galushin@gmail.com
*/

#include <boost/dll/import.hpp>

#include <iostream>
#include <string>

/*
argv[1] Строка: имя динамической библиотеки, которую нужно загрузить
argv[2] Строка: имя целевой функции из библиотеки argv[1]
argv[3] Целое положительное: размерность вектора аргумента,
argv[4] Строка: цель оптимзиации (должно быть min или max)
*/
// @todo Локализация

int main(int argc, char * argv[])
{
    // @todo Задокументировать "SAGA function plugin"
    if(argc < 4)
    {
        std::cout << "This program needs at least 3 parameters:\n"
            << "1. path to dynamic lybrary, implementing SAGA function plugin\n"
            << "2. objective function name\n"
            << "3. dimension (positive integer)\n";
        return 0;
    }

    // @todo Создать менеджер для работы с плагинами, содержащими функций

    boost::dll::fs::path const lib_path(argv[1]);

    // @todo Проверить, что библиотека существует и загрузить её
    std::string const objective_name(argv[2]);

    // @todo Убедиться, что функция с заданным именем существует

    // @todo Перехватить исключения
    auto const dim = std::atoi(argv[3]);

    if(dim < 0)
    {
        std::cout << "Dimension must be positive integer, but it's equal to " << dim << "\n";
        return 0;
    }

    // @todo Если цель не задана, то использовать минимизацию

    return 0;
}
