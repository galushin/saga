/* (c) 2019 СибЮИ МВД России.
Данный файл -- часть программы "Локальный поиск псевдо-булевой оптимзиации"
Никакие гарантии не предоставляются, в том числе гарантия пригодности для любой конкретной цели.
Автор: Галушин Павел Викторович, galushin@gmail.com
*/

#ifndef PLUGIN_HPP_INCLUDED
#define PLUGIN_HPP_INCLUDED

#include <saga/cpp20/span.hpp>

#include <cstddef>
#include <string>

namespace saga
{
    using objective_value_type = double;
    using boolean_const_span = saga::span<bool const>;

    using pseudo_boolean_objective_signature = objective_value_type(boolean_const_span);

    class function_manager
    {
    public:
        void add(pseudo_boolean_objective_signature * fun, std::string const & name)
        {
            this->do_add(fun, name);
        }

    protected:
        virtual ~function_manager() {};

        virtual void do_add(pseudo_boolean_objective_signature *, std::string const &) = 0;
    };

    class function_regitrar
    {
    public:
        void init(function_manager & manager)
        {
            return this->do_init(manager);
        }

    protected:
        virtual void do_init(function_manager & manager) = 0;

        virtual ~function_regitrar() {};
    };
}
// namespace saga

#endif
// PLUGIN_HPP_INCLUDED
