#ifndef PLUGIN_HPP_INCLUDED
#define PLUGIN_HPP_INCLUDED

#include <cstddef>
#include <string>

namespace saga
{
    using pseudo_boolean_objective_signature = double(bool const *, std::size_t);

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
