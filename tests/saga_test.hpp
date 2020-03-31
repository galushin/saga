/* (c) 2020 Галушин Павел Викторович, galushin@gmail.com

Данный файл -- часть библиотеки SAGA.

SAGA -- это свободной программное обеспечение: вы можете перераспространять ее и/или изменять ее
на условиях Стандартной общественной лицензии GNU в том виде, в каком она была опубликована Фондом
свободного программного обеспечения; либо версии 3 лицензии, либо (по вашему выбору) любой более
поздней версии.

Это программное обеспечение распространяется в надежде, что оно будет полезной, но БЕЗО ВСЯКИХ
ГАРАНТИЙ; даже без неявной гарантии ТОВАРНОГО ВИДА или ПРИГОДНОСТИ ДЛЯ ОПРЕДЕЛЕННЫХ ЦЕЛЕЙ.
Подробнее см. в Стандартной общественной лицензии GNU.

Вы должны были получить копию Стандартной общественной лицензии GNU вместе с этим программным
обеспечение. Если это не так, см. https://www.gnu.org/licenses/.
*/

#ifndef Z_SAGA_TEST_HPP_INCLUDED
#define Z_SAGA_TEST_HPP_INCLUDED

#include "random_engine.hpp"

#include <type_traits>

namespace saga_test
{
    using generation_t = int;

    namespace detail
    {
        template <class T>
        struct arbitrary_real
        {
        public:
            using value_type = T;

            template <class UniformRandomBitGenerator>
            static T generate(generation_t generation, UniformRandomBitGenerator & urbg)
            {
                switch(generation)
                {
                case 0:
                    return value_type(0);

                case 1:
                    return std::numeric_limits<value_type>::min();

                case 2:
                    return std::numeric_limits<value_type>::max();

                case 3:
                    return std::numeric_limits<value_type>::lowest();

                default:
                    std::uniform_real_distribution<value_type>
                        distr(std::numeric_limits<value_type>::min(),
                              std::numeric_limits<value_type>::max());
                    return distr(urbg);
                }
            }
        };
    }
    // namespace detail

    // @todo выводить ошибку компиляции при попытке использовать или задокументировать неполноту
    template <class T, class SFINAE = void>
    struct arbitrary;

    template <class T>
    struct arbitrary<T, std::enable_if_t<std::is_floating_point<T>::value>>
     : detail::arbitrary_real<T>
    {};

    namespace detail
    {
        template <class Arg>
        void check_property(void(*property)(Arg))
        {
            // @todo Возможность настраивать это значение
            auto const max_generation = generation_t{100};

            for(auto gen = generation_t{0}; gen < max_generation; ++ gen)
            {
                property(saga_test::arbitrary<Arg>::generate(gen, saga_test::random_engine()));
            }
        }
    }
    // namespace detail

    template <class StatelessLambda>
    void check_property(StatelessLambda lambda)
    {
        return ::saga_test::detail::check_property(+lambda);
    }
}
// namespace saga_test

#endif
// Z_SAGA_TEST_HPP_INCLUDED
