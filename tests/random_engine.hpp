#ifndef Z_SAGA_TEST_RANDOM_ENGINE_HPP_INCLUDED
#define Z_SAGA_TEST_RANDOM_ENGINE_HPP_INCLUDED

#include <random>

namespace saga_test
{
    using random_engine_type = std::mt19937;

    random_engine_type & random_engine();
}
// namespace saga_test

#endif
// Z_SAGA_TEST_RANDOM_ENGINE_HPP_INCLUDED
