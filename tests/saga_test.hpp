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

#include <saga/type_traits.hpp>
#include <saga/detail/static_empty_const.hpp>

#include <cassert>

#include <array>
#include <algorithm>
#include <iterator>
#include <tuple>
#include <type_traits>

namespace saga_test
{
    using generation_t = int;

    template <class IntType = std::size_t>
    struct container_size
    {
    public:
        operator IntType const & () const
        {
            return this->value;
        }

        IntType value{};
    };

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

        template <class Generator, class Incrementable>
        class function_input_iterator
        {
            friend bool operator==(function_input_iterator const & lhs,
                                   function_input_iterator const & rhs)
            {
                return lhs.pos_ == rhs.pos_;
            }

            friend bool operator!=(function_input_iterator const & lhs,
                                   function_input_iterator const & rhs)
            {
                return !(lhs == rhs);
            }

        public:
            // Типы
            using iterator_category = std::input_iterator_tag;
            using value_type = std::decay_t<decltype(std::declval<Generator&>()())>;
            using difference_type = Incrementable;
            using reference = value_type const &;
            using pointer = value_type const *;

            // Создание и копирование
            function_input_iterator(Generator gen, Incrementable pos)
             : gen_(std::move(gen))
             , pos_(std::move(pos))
            {}

            // Итератор
            reference operator*()
            {
                if(!this->has_value_)
                {
                    this->value_ = (this->gen_)();
                    this->has_value_ = true;
                }

                return this->value_;
            }

            function_input_iterator & operator++()
            {
                if(this->has_value_)
                {
                    this->has_value_ = false;
                }
                else
                {
                    (this->gen_)();
                }

                ++ this->pos_;

                return *this;
            }

        private:
            Generator gen_{};
            Incrementable pos_{};

            // @todo optional
            bool has_value_ = false;
            value_type value_{};
        };

        template <class Generator, class Incrementable>
        function_input_iterator<Generator, Incrementable>
        make_function_input_iterator(Generator gen, Incrementable pos)
        {
            return {std::move(gen), std::move(pos)};
        }

        template <class Container>
        struct arbitrary_sequence_container
        {
        public:
            using value_type = Container;

            template <class UniformRandomBitGenerator>
            static value_type generate(generation_t generation, UniformRandomBitGenerator & urbg)
            {
                assert(generation >= 0);

                using Size = typename Container::size_type;
                auto const num = Size{arbitrary<container_size<Size>>::generate(generation, urbg)};

                if(num == 0)
                {
                    return {};
                }

                std::uniform_int_distribution<generation_t> distr(0, generation);

                using Element = typename Container::value_type;
                auto element_gen = [&](){ return arbitrary<Element>::generate(distr(urbg), urbg); };

                auto first = ::saga_test::detail::make_function_input_iterator(element_gen, 0*num);
                auto last = ::saga_test::detail::make_function_input_iterator(element_gen, num);

                return value_type(first, last);
            }
        };

        template <typename T, class SFINAE = void>
        struct is_sequence_container
         : std::false_type
        {};

        template <typename T>
        struct is_sequence_container<T, saga::void_t<typename T::value_type>>
         : std::is_constructible<T, typename T::value_type const *, typename T::value_type const *>
        {};
    }
    // namespace detail

    template <>
    struct arbitrary<bool>
    {
    public:
        template <class UniformRandomBitGenerator>
        static bool generate(generation_t generation, UniformRandomBitGenerator & urbg)
        {
            return std::bernoulli_distribution(0.5)(urbg);
        }
    };

    template <>
    struct arbitrary<char>
    {
    public:
        template <class UniformRandomBitGenerator>
        static char generate(generation_t generation, UniformRandomBitGenerator & urbg)
        {
            if(generation == 0)
            {
                return 0;
            }
            else
            {
                return std::uniform_int_distribution<char>(32, 126)(urbg);
            }
        }
    };

    template <class T>
    struct arbitrary<T, std::enable_if_t<std::is_integral<T>::value>>
     : detail::arbitrary_integer<T>
    {};

    template <class T>
    struct arbitrary<T, std::enable_if_t<std::is_floating_point<T>::value>>
     : detail::arbitrary_real<T>
    {};

    template <class T>
    struct arbitrary<T, std::enable_if_t<detail::is_sequence_container<T>::value>>
     : detail::arbitrary_sequence_container<T>
    {};

    template <class T, std::size_t N>
    struct arbitrary<std::array<T, N>>
    {
    public:
        using value_type = std::array<T, N>;

        template <class UniformRandomBitGenerator>
        static value_type generate(generation_t generation, UniformRandomBitGenerator & urbg)
        {
            assert(generation >= 0);

            value_type result;

            using Element = typename value_type::value_type;
            std::uniform_int_distribution<generation_t> distr(0, generation);

            std::generate(result.begin(), result.end(),
                          [&](){ return arbitrary<Element>::generate(distr(urbg), urbg); });

            return result;
        }
    };

    template <class... Types>
    struct arbitrary<std::tuple<Types...>>
    {
    public:
        using value_type = std::tuple<Types...>;

        template <class UniformRandomBitGenerator>
        static value_type generate(generation_t generation, UniformRandomBitGenerator & urbg)
        {
            return std::make_tuple(arbitrary<Types>::generate(std::move(generation), urbg)...);
        }
    };

    template <class IntType>
    struct arbitrary<container_size<IntType>>
    {
    public:
        using value_type = container_size<IntType>;

        template <class UniformRandomBitGenerator>
        static value_type generate(generation_t generation, UniformRandomBitGenerator & urbg)
        {
            if(generation == 0)
            {
                return {0};
            }

            std::uniform_int_distribution<IntType> distr(0, generation);

            return {distr(urbg)};
        }
    };

    namespace detail
    {
        template <class F, class Tuple, std::size_t... Indices>
        void apply_impl(F&& f, Tuple&& t, std::index_sequence<Indices...>)
        {
            return std::forward<F>(f)(std::get<Indices>(std::forward<Tuple>(t))...);
        }

        template <class F, class Tuple>
        void apply(F&& f, Tuple&& t)
        {
            constexpr auto const n = std::tuple_size<std::remove_reference_t<Tuple>>::value;

            return saga_test::detail::apply_impl(std::forward<F>(f), std::forward<Tuple>(t),
                                                 std::make_index_sequence<n>{});
        }

        template <class... Args>
        void check_property(void(*property)(Args...))
        {
            auto const max_generation = generation_t{100};

            using Value = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;

            for(auto gen = generation_t{0}; gen < max_generation; ++ gen)
            {
                auto args = saga_test::arbitrary<Value>::generate(gen, saga_test::random_engine());
                ::saga_test::detail::apply(property, std::move(args));
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
