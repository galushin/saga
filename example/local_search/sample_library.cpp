#include "./plugin.hpp"

#include <boost/config.hpp>

#include <algorithm>

double manhattan_norm(bool const * ptr, std::size_t dim)
{
    return std::count(ptr, ptr+dim, true);
}

class sample_library_registrar
 : saga::function_regitrar
{
protected:
    void do_init(saga::function_manager & manager) override
    {
        manager.add(manhattan_norm, "Manhattan norm");
    }
};

extern "C" BOOST_SYMBOL_EXPORT sample_library_registrar registrar;

sample_library_registrar registrar;
