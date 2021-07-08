/* (c) 2020-2021 Галушин Павел Викторович, galushin@gmail.com

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
#include <saga/type_traits.hpp>
#include <saga/utility/operators.hpp>

#include <cassert>

#include <array>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <tuple>
#include <type_traits>

// @todo Разбить на более мелкие файлы (в духе: один класс -- один файл)

namespace saga_test
{
    // Тестирование, основанное на свойствах
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
         : saga::operators::equality_comparable<function_input_iterator<Generator, Incrementable>>
        {
            friend bool operator==(function_input_iterator const & lhs,
                                   function_input_iterator const & rhs)
            {
                return lhs.pos_ == rhs.pos_;
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
        static bool generate(generation_t, UniformRandomBitGenerator & urbg)
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

    template <class IntType, IntType x_min, IntType x_max>
    struct bounded
    {
    public:
        // Свойства типа
        constexpr static IntType min()
        {
            return x_min;
        }

        constexpr static IntType max()
        {
            return x_max;
        }

        // Конструкторы
        constexpr explicit bounded(IntType value)
         : value_(std::move(value))
        {
            if(this->value_ < x_min || x_max < this->value_)
            {
                throw std::out_of_range("saga_test::bounded");
            }
        }

        // Преобразование
        constexpr IntType const & value() const
        {
            return this->value_;
        }

        constexpr operator IntType const & () const
        {
            return this->value();
        }

    private:
        IntType value_;
    };

    template <class IntType, IntType x_min, IntType x_max>
    struct arbitrary<saga_test::bounded<IntType, x_min, x_max>>
    {
        static_assert(x_min <= x_max, "Incorrect bounds");

        using value_type = saga_test::bounded<IntType, x_min, x_max>;

        template <class UniformRandomBitGenerator>
        static value_type generate(generation_t generation, UniformRandomBitGenerator & urbg)
        {
            assert(generation >= 0);

            switch(generation)
            {
            case 0:
                return value_type(x_min);

            case 1:
                return value_type(x_max);

            default:
                std::uniform_int_distribution<IntType> distr(x_min, x_max);
                return value_type(distr(urbg));
            }
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

    // Классы с типовыми особенностями
    template <class T>
    struct explicit_ctor_from
    {
        // @todo Правильно ли это реализовано?
        constexpr explicit explicit_ctor_from(T value) noexcept
         : value(std::move(value))
        {}

        saga::remove_cvref_t<T> value;
    };

    template <class T>
    struct move_only
    {
    template <class U>
    friend bool operator==(move_only<T> const & lhs, move_only<U> const & rhs)
    {
        return lhs.value == rhs.value;
    }

    public:
        move_only() = default;

        constexpr explicit move_only(T init_value)
         : value(std::move(init_value))
        {}

        template <class U
                 , std::enable_if_t<std::is_constructible<T, U&&>{}> * = nullptr
                 , std::enable_if_t<!std::is_convertible<U&&, T>{}> * = nullptr>
        constexpr explicit move_only(move_only<U> && other)
         : value(std::move(other).value)
        {}

        template <class U
                 , std::enable_if_t<std::is_constructible<T, U&&>{}> * = nullptr
                 , std::enable_if_t<std::is_convertible<U&&, T>{}> * = nullptr>
        constexpr move_only(move_only<U> && other)
         : value(std::move(other).value)
        {}

        move_only(move_only const &) = delete;
        move_only(move_only &&) = default;

        move_only & operator=(move_only const &) = delete;
        move_only & operator=(move_only &&) = default;

        T value;
    };

    template <class T>
    struct arbitrary<move_only<T>>
    {
        using value_type = move_only<T>;

        template <class UniformRandomBitGenerator>
        static value_type generate(generation_t generation, UniformRandomBitGenerator & urbg);
    };

    class Base
    {
    public:
        virtual ~Base() {};
    };

    class Derived
     : public Base
    {};

    // Вспомогательные constexpr-функции, выполняющие определённые операции
    template <class T>
    constexpr T use_constexpr_move(T x)
    {
        auto y = std::move(x);

        return y;
    }

    struct not_swapable
    {
        friend void swap(not_swapable &, not_swapable &) = delete;
    };

    struct init_by_reduce_initializer_list
    {
        constexpr init_by_reduce_initializer_list(std::initializer_list<int> inits, int arg)
         : value(arg)
        {
            for(auto const & each : inits)
            {
                value += each;
            }
        }

        int value = 0;

        constexpr bool operator==(init_by_reduce_initializer_list const & other) const
        {
            return this->value == other.value;
        }
    };

    template <class InputRange>
    std::istringstream
    make_istringstream_from_range(InputRange const & values)
    {
        using Value = typename std::iterator_traits<decltype(std::begin(values))>::value_type;

        std::ostringstream oss;

        std::copy(std::begin(values), std::end(values), std::ostream_iterator<Value>(oss, " "));

        return std::istringstream(oss.str());
    }
}
// namespace saga_test

#endif
// Z_SAGA_TEST_HPP_INCLUDED
