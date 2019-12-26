/* (c) 2019 СибЮИ МВД России.
Данный файл -- часть программы "Локальный поиск псевдо-булевой оптимзиации"
Никакие гарантии не предоставляются, в том числе гарантия пригодности для любой конкретной цели.
Автор: Галушин Павел Викторович, galushin@gmail.com
*/

#include "./plugin.hpp"

#include <boost/config.hpp>

#include <algorithm>

saga::objective_value_type
manhattan_norm(saga::boolean_const_span xs)
{
    return static_cast<saga::objective_value_type>(std::count(xs.begin(), xs.end(), true));
}

class sample_library_registrar
 : saga::function_regitrar
{
protected:
    void do_init(saga::function_manager & manager) override
    {
        manager.add(manhattan_norm, "manhattan_norm");
    }
};

extern "C" BOOST_SYMBOL_EXPORT sample_library_registrar registrar;

sample_library_registrar registrar;
