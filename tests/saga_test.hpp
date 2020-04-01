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
#include <saga/detail/static_empty_const.hpp>

#include <type_traits>

namespace saga_test
{
    using generation_t = int;

    // @todo выводить ошибку компиляции при попытке использовать или задокументировать неполноту
    template <class T, class SFINAE = void>
    struct arbitrary;

    namespace detail
    {
        template <class T>
        struct arbitrary_integer
        {
        public:
            using value_type = T;

            template <class UniformRandomBitGenerator>
            static T generate(generation_t generation, UniformRandomBitGenerator & urbg)
            {
                assert(generation >= 0);

                switch(generation)
                {
                case 0:
                    return value_type(0);

                case 1:
                    return std::numeric_limits<value_type>::min();

                case 2:
                    return std::numeric_limits<value_type>::max();

                default:
                    std::uniform_int_distribution<value_type>
                        distr(std::numeric_limits<value_type>::min(),
                              std::numeric_limits<value_type>::max());
                    return distr(urbg);
                }
            }
        };

        template <class T>
        struct arbitrary_real
        {
        public:
            using value_type = T;

            template <class UniformRandomBitGenerator>
            static T generate(generation_t generation, UniformRandomBitGenerator & urbg)
            {
                assert(generation >= 0);

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

        template <class Container>
        struct arbitrary_container
        {
        public:
            using value_type = Container;

            template <class UniformRandomBitGenerator>
            static value_type generate(generation_t generation, UniformRandomBitGenerator & urbg)
            {
                assert(generation >= 0);

                if(generation == 0)
                {
                    return {};
                }

                // @todo Через generate_iterator?
                value_type result;

                using Size = typename Container::size_type;
                using Element = typename Container::value_type;

                std::uniform_int_distribution<Size> distr(0, generation);

                std::generate_n(std::back_inserter(result), generation,
                                [&](){ return arbitrary<Element>::generate(distr(urbg), urbg); });

                return result;
            }
        };
    }
    // namespace detail

    template <>
    struct arbitrary<bool>;

    template <>
    struct arbitrary<char>;

    template <class T>
    struct arbitrary<T, std::enable_if_t<std::is_integral<T>::value>>
     : detail::arbitrary_integer<T>
    {};

    template <class T>
    struct arbitrary<T, std::enable_if_t<std::is_floating_point<T>::value>>
     : detail::arbitrary_real<T>
    {};

    template <class T, class A>
    struct arbitrary<std::vector<T, A>>
     : detail::arbitrary_container<std::vector<T, A>>
    {};

    namespace detail
    {
        template <class Arg>
        void check_property(void(*property)(Arg))
        {
            // @todo Возможность настраивать это значение
            auto const max_generation = generation_t{100};

            using Value = std::remove_cv_t<std::remove_reference_t<Arg>>;

            for(auto gen = generation_t{0}; gen < max_generation; ++ gen)
            {
                property(saga_test::arbitrary<Value>::generate(gen, saga_test::random_engine()));
            }
        }

        struct property_checker_t
        {
        public:
            template <class StatelessLambda>
            property_checker_t const &
            operator<<(StatelessLambda lambda) const
            {
                ::saga_test::detail::check_property(+lambda);
                return *this;
            }
        };
    }
    // namespace detail

    template <class StatelessLambda>
    void check_property(StatelessLambda lambda)
    {
        return ::saga_test::detail::check_property(+lambda);
    }

    namespace
    {
        constexpr auto const & property_checker
            = saga::detail::static_empty_const<detail::property_checker_t>::value;
    }
}
// namespace saga_test

#endif
// Z_SAGA_TEST_HPP_INCLUDED
