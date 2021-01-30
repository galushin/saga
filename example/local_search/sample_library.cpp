/* (c) 2019-2021 СибЮИ МВД России.
Данный файл -- часть программы "Локальный поиск псевдо-булевой оптимзиации"
Никакие гарантии не предоставляются, в том числе гарантия пригодности для любой конкретной цели.
Автор: Галушин Павел Викторович, galushin@gmail.com
*/

#include "./plugin.hpp"

#include <saga/optimization/test_objectives.hpp>

#include <boost/config.hpp>

#include <algorithm>

namespace
{
    saga::objective_value_type
    manhattan_norm_boolean(saga::boolean_const_span xs)
    {
        return saga::boolean_manhattan_norm(xs);
    }

    saga::objective_value_type
    manhattan_norm_integer(saga::integer_const_span xs)
    {
        return saga::manhattan_norm(xs);
    }

    class sample_library_registrar
     : saga::function_regitrar
    {
    protected:
        void do_init(saga::function_manager & manager) override
        {
            manager.add(manhattan_norm_boolean, "manhattan_norm_boolean");
            manager.add(manhattan_norm_integer, "manhattan_norm_integer");
        }
    };

    extern "C" BOOST_SYMBOL_EXPORT sample_library_registrar registrar;

    sample_library_registrar registrar;
}
