#include <ctime>

#include "random_engine.hpp"

namespace saga_test
{
    random_engine_type & random_engine()
    {
        static random_engine_type
            instance(static_cast<random_engine_type::result_type>(std::time(nullptr)));

        return instance;
    }
}
// namespace saga_test
