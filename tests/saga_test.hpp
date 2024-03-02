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

#include <saga/type_traits.hpp>
#include <saga/test/regular_tracer.hpp>
#include <saga/utility/operators.hpp>

#include <catch2/catch_amalgamated.hpp>

#include <cassert>

#include <array>
#include <algorithm>
#include <sstream>
#include <type_traits>

namespace saga_test
{
    // Тестирование, основанное на свойствах
    using generation_t = int;

    template <class T, class SFINAE = void>
    struct arbitrary;

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
        struct arbitrary_container;

        // @todo Поддержка распределителей памяти без конструктора без аргументов
        template <class T, class A>
        struct arbitrary_container<std::vector<T, A>>
        {
        public:
            using value_type = std::vector<T, A>;

            template <class UniformRandomBitGenerator>
            static value_type generate(generation_t generation, UniformRandomBitGenerator & urbg)
            {
                assert(generation >= 0);

                using Container = value_type;

                using Size = typename Container::size_type;
                auto num = Size{arbitrary<container_size<Size>>::generate(generation, urbg)};

                if(num == 0)
                {
                    return {};
                }

                Container result;
                result.reserve(num);

                for(; num > 0; -- num)
                {
                    std::uniform_int_distribution<generation_t> distr(0, generation);
                    using Element = typename Container::value_type;
                    result.push_back(arbitrary<Element>::generate(distr(urbg), urbg));
                }

                return result;
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
                using Elem = typename Container::value_type;

                using VectorGenerator = arbitrary_container<std::vector<Elem>>;

                auto src = VectorGenerator::generate(generation, urbg);

                return Container(std::make_move_iterator(src.begin())
                                 , std::make_move_iterator(src.end()));
            }
        };

        template <typename T, class SFINAE = void>
        struct is_container
         : std::false_type
        {};

        template <typename T>
        struct is_container<T, std::void_t<typename T::value_type, typename T::size_type
                                          ,typename T::iterator>>
         : std::is_constructible<T, typename T::value_type const *, typename T::value_type const *>
        {};

        template <class T>
        struct arbitrary_tuple_like_constructible
        {
            using value_type = T;

            template <class UniformRandomBitGenerator, std::size_t... Ints>
            static value_type generate(generation_t generation, UniformRandomBitGenerator & urbg
                                       , std::index_sequence<Ints...>)
            {
                return value_type(arbitrary<std::decay_t<std::tuple_element_t<Ints, value_type>>>::generate(generation, urbg)...);
            }

            template <class UniformRandomBitGenerator>
            static value_type generate(generation_t generation, UniformRandomBitGenerator & urbg)
            {
                return generate(generation, urbg
                               , std::make_index_sequence<std::tuple_size<T>::value>{});
            }
        };

        template <class T, class Seq>
        struct is_tuple_like_constructible_n;

        template <class T, std::size_t... Ints>
        struct is_tuple_like_constructible_n<T, std::index_sequence<Ints...>>
         : std::is_constructible<T, std::tuple_element_t<Ints, T>...>
        {};

        template <class T, class SFINAE = void>
        struct is_tuple_like_constructible
         : std::false_type
        {};

        template <class T>
        struct is_tuple_like_constructible<T, std::void_t<decltype(std::tuple_size<T>::value)>>
         : is_tuple_like_constructible_n<T, std::make_index_sequence<std::tuple_size<T>::value>>
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
                return std::uniform_int_distribution<int>(32, 126)(urbg);
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

    // @todo Нужна ли специализированная генерация для ассоциативных и последовательных контейнеров?
    template <class T>
    struct arbitrary<T, std::enable_if_t<detail::is_container<T>::value>>
     : detail::arbitrary_container<T>
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

    template <class T>
    struct arbitrary<T, std::enable_if_t<detail::is_tuple_like_constructible<T>{}>>
     : detail::arbitrary_tuple_like_constructible<T>
    {};

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

    template <class IntType>
    struct arbitrary<std::function<bool(IntType const &)>
                    , std::enable_if_t<std::is_integral<IntType>{}>>
    {
        using value_type = std::function<bool(IntType const &)>;

        template <class UniformRandomBitGenerator>
        static value_type generate(generation_t generation, UniformRandomBitGenerator & urbg)
        {
            using BinPredGen = arbitrary<std::function<bool(IntType const &, IntType const &)>>;

            auto bin_pred = BinPredGen::generate(generation, urbg);

            auto value = arbitrary<IntType>::generate(generation, urbg);

            if(generation % 2 == 0)
            {
                return [=](IntType const & arg) { return bin_pred(arg, value); };
            }
            else
            {
                return [=](IntType const & arg) { return bin_pred(value, arg); };
            }
        }
    };

    template <class IntType>
    struct arbitrary<std::function<IntType(IntType const &)>
                    , std::enable_if_t<std::is_integral<IntType>{}>>
    {
        using value_type = std::function<IntType(IntType const &)>;

        template <class UniformRandomBitGenerator>
        static value_type generate(generation_t generation, UniformRandomBitGenerator & urbg)
        {
            using BinOpGen = arbitrary<std::function<IntType(IntType const &, IntType const &)>>;

            auto bin_op = BinOpGen::generate(generation, urbg);

            auto value = arbitrary<IntType>::generate(generation, urbg);

            if(generation % 2 == 0)
            {
                return [=](IntType const & arg) { return bin_op(arg, value); };
            }
            else
            {
                return [=](IntType const & arg) { return bin_op(value, arg); };
            }
        }
    };

    template <class IntType, class Arg>
    struct arbitrary<std::function<IntType(Arg, Arg)>
                    , std::enable_if_t<std::is_unsigned<IntType>{}
                                       && (std::is_same<Arg, IntType>{}
                                           || std::is_same<Arg, IntType const &>{})>>
    {
        using value_type = std::function<IntType(Arg, Arg)>;

        template <class UniformRandomBitGenerator>
        static value_type generate(generation_t generation, UniformRandomBitGenerator & urbg)
        {
            switch(saga_test::arbitrary<size_t>::generate(generation, urbg) % 11)
            {
            break; case 0:
                return value_type(std::bit_xor<>{});

            break; case 1:
                return value_type(std::bit_or<>{});

            break; case 2:
                return value_type(std::bit_and<>{});

            break; case 3:
                return value_type([](IntType const & lhs, IntType const & rhs)
                                  { return std::min(lhs, rhs); });

            break; case 4:
                return value_type([](IntType const & lhs, IntType const & rhs)
                                  { return std::max(lhs, rhs); });

            break; case 5:
                return value_type(std::plus<>{});

            break; case 6:
                return value_type(std::multiplies<>{});

            break; case 7:
                {
                    auto value = arbitrary<IntType>::generate(generation, urbg);

                    return value_type([=](IntType const &, IntType const &) { return value; });
                }

            // Некоммутативные операции
            break; case 8:
                return value_type([](IntType const & lhs, IntType const &) { return lhs; });

            break; case 9:
                return value_type([](IntType const &, IntType const & rhs) { return rhs; });

            // Не ассоциативные операции
            break; default:
                return value_type(std::minus<>{});
            }
        }
    };

    template <class IntType1, class IntType2>
    struct arbitrary<std::function<bool(IntType1 const &, IntType2 const &)>
                    , std::enable_if_t<std::is_integral<IntType1>{}
                                       && std::is_integral<IntType2>{}>>
    {
        using value_type = std::function<bool(IntType1 const &, IntType2 const &)>;

        template <class UniformRandomBitGenerator>
        static value_type generate(generation_t generation, UniformRandomBitGenerator & urbg)
        {
            using Distr1 = std::uniform_int_distribution<IntType1>;
            using Distr2 = std::uniform_int_distribution<IntType2>;

            auto mod_1 = Distr1(1, std::numeric_limits<IntType1>::max())(urbg);
            auto mod_2 = Distr2(1, std::numeric_limits<IntType2>::max())(urbg);

            switch(generation % 8)
            {
            break; case 0:
                return value_type([=](IntType1 const & lhs, IntType2 const & rhs)
                                  { return std::equal_to<>{}(lhs % mod_1, rhs % mod_2); });
            break; case 1:
                return value_type([=](IntType1 const & lhs, IntType2 const & rhs)
                                  { return std::less<>{}(lhs % mod_1, rhs % mod_2); });
            break; case 2:
                return value_type([=](IntType1 const & lhs, IntType2 const & rhs)
                                  { return std::not_equal_to<>{}(lhs % mod_1, rhs % mod_2); });
            break; case 3:
                return value_type([=](IntType1 const & lhs, IntType2 const & rhs)
                                  { return std::greater<>{}(lhs % mod_1, rhs % mod_2); });
            break; case 4:
                return value_type([=](IntType1 const & lhs, IntType2 const & rhs)
                                  { return std::less_equal<>{}(lhs % mod_1, rhs % mod_2); });
            break; case 5:
                return value_type([=](IntType1 const & lhs, IntType2 const & rhs)
                                  { return std::greater_equal<>{}(lhs % mod_1, rhs % mod_2); });
            break; case 6:
                return value_type([](IntType1 const &, IntType2 const &){ return true; });

            break; default:
                return value_type([](IntType1 const &, IntType2 const &){ return false; });
            }
        }
    };

    template <class Value>
    class strict_weak_order
    {
    friend class arbitrary<strict_weak_order>;

    public:
        using Function = std::function<bool(Value const &, Value const &)>;

        bool operator()(Value const & lhs, Value const & rhs) const
        {
            return this->impl_(lhs, rhs);
        }

        friend bool operator==(strict_weak_order const & lhs, strict_weak_order const & rhs)
        {
            return lhs.id_ == rhs.id_;
        }

    private:
        using id_type = std::ptrdiff_t;

        explicit strict_weak_order(Function fun, id_type id)
         : impl_(std::move(fun))
         , id_(id)
        {}

        Function impl_;
        id_type id_ = 0;
    };

    template <class IntType>
    struct arbitrary<strict_weak_order<IntType>, std::enable_if_t<std::is_integral<IntType>{}>>
    {
        using value_type = strict_weak_order<IntType>;

        template <class UniformRandomBitGenerator>
        static value_type generate(generation_t generation, UniformRandomBitGenerator & urbg)
        {
            auto Modulus_distr
                = std::uniform_int_distribution<IntType> (1, std::numeric_limits<IntType>::max());

            auto Modulus = Modulus_distr(urbg);
            auto id = Modulus ^ generation;

            if(generation % 2 == 0)
            {
                return value_type([=](IntType const & lhs, IntType const & rhs)
                                  { return std::less<>{}(lhs % Modulus, rhs % Modulus); }
                                  , id);
            }
            else
            {
                return value_type([=](IntType const & lhs, IntType const & rhs)
                                  { return std::greater<>{}(lhs % Modulus, rhs % Modulus); }
                                 , id);
            }
        }
    };

    template <class Value>
    class associative_operation
    {
    friend arbitrary<associative_operation>;

    public:
        using Function = std::function<Value(Value const &, Value const &)>;

        Value operator()(Value const & lhs, Value const & rhs) const
        {
            return this->impl_(lhs, rhs);
        }

    private:
        explicit associative_operation(Function fun)
         : impl_(std::move(fun))
        {}

        Function impl_;
    };

    template <class IntType>
    struct arbitrary<associative_operation<IntType>, std::enable_if_t<std::is_unsigned<IntType>{}>>
    {
        using value_type = associative_operation<IntType>;

        template <class UniformRandomBitGenerator>
        static value_type generate(generation_t generation, UniformRandomBitGenerator & urbg)
        {
            switch(saga_test::arbitrary<size_t>::generate(generation, urbg) % 10)
            {
            break; case 0:
                return value_type(std::bit_xor<>{});

            break; case 1:
                return value_type(std::bit_or<>{});

            break; case 2:
                return value_type(std::bit_and<>{});

            break; case 3:
                return value_type([](IntType const & lhs, IntType const & rhs)
                                  { return std::min(lhs, rhs); });

            break; case 4:
                return value_type([](IntType const & lhs, IntType const & rhs)
                                  { return std::max(lhs, rhs); });

            break; case 5:
                return value_type(std::plus<>{});

            break; case 6:
                return value_type(std::multiplies<>{});

            break; case 7:
                {
                    auto value = arbitrary<IntType>::generate(generation, urbg);

                    return value_type([=](IntType const &, IntType const &) { return value; });
                }

            // Некоммутативные операции
            break; case 8:
                return value_type([](IntType const & lhs, IntType const &) { return lhs; });

            break; default:
                return value_type([](IntType const &, IntType const & rhs) { return rhs; });
            }
        }
    };

    template <class Value>
    class abelian_group_operation
    {
    friend arbitrary<abelian_group_operation>;

    public:
        using Function = std::function<Value(Value const &, Value const &)>;

        Value operator()(Value const & lhs, Value const & rhs) const
        {
            return this->impl_(lhs, rhs);
        }

    private:
        explicit abelian_group_operation(Function fun)
         : impl_(std::move(fun))
        {}

        Function impl_;
    };

    template <class IntType>
    struct arbitrary<abelian_group_operation<IntType>, std::enable_if_t<std::is_unsigned<IntType>{}>>
    {
        using value_type = abelian_group_operation<IntType>;

        template <class UniformRandomBitGenerator>
        static value_type generate(generation_t generation, UniformRandomBitGenerator & urbg)
        {
            switch(saga_test::arbitrary<size_t>::generate(generation, urbg) % 8)
            {
            break; case 0:
                return value_type(std::bit_xor<>{});

            break; case 1:
                return value_type(std::bit_or<>{});

            break; case 2:
                return value_type(std::bit_and<>{});

            break; case 3:
                return value_type([](IntType const & lhs, IntType const & rhs)
                                  { return std::min(lhs, rhs); });

            break; case 4:
                return value_type([](IntType const & lhs, IntType const & rhs)
                                  { return std::max(lhs, rhs); });

            break; case 5:
                return value_type(std::plus<>{});

            break; case 6:
                return value_type(std::multiplies<>{});

            break; default:
                {
                    auto value = arbitrary<IntType>::generate(generation, urbg);

                    return value_type([=](IntType const &, IntType const &) { return value; });
                }
            }
        }
    };

    namespace detail
    {
        template <class... Args>
        void check_property(void(*property)(Args...))
        {
            auto const max_generation = generation_t{100};

            using Value = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;

            for(auto gen = generation_t{0}; gen < max_generation; ++ gen)
            {
                auto args = saga_test::arbitrary<Value>::generate(gen, saga_test::random_engine());
                std::apply(property, std::move(args));
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
        inline constexpr auto const & property_checker = detail::property_checker_t{};
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

        template <class U>
        friend bool operator==(move_only<T> const & lhs, U const & rhs)
        {
            return lhs.value == rhs;
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

        operator T const &() const { return this->value; };

        T value;
    };

    template <class T>
    struct arbitrary<move_only<T>>
    {
        using value_type = move_only<T>;

        template <class UniformRandomBitGenerator>
        static value_type generate(generation_t generation, UniformRandomBitGenerator & urbg);
    };

    struct throws_on_move
    {
        int value = 0;

        explicit throws_on_move(int init_value)
         : value(init_value)
        {}

        throws_on_move(throws_on_move const & rhs) noexcept(false)
         : value(rhs.value)
        {}

        throws_on_move(throws_on_move &&) noexcept(false)
        {
            throw std::runtime_error("throws_on_move::move ctor");
        }

        throws_on_move & operator=(throws_on_move &&) noexcept(false)
        {
            throw std::runtime_error("throws_on_move::move assign");
            return *this;
        }

        friend bool operator==(throws_on_move const & lhs, throws_on_move const & rhs)
        {
            return lhs.value == rhs.value;
        }
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

namespace Catch
{
    template <class T, class Tag>
    struct StringMaker<saga::regular_tracer<T, Tag>>
    {
        static std::string convert(saga::regular_tracer<T, Tag> const & rhs)
        {
            return Catch::StringMaker<T>::convert(rhs.value());
        }
    };
}
// namespace Catch

#endif
// Z_SAGA_TEST_HPP_INCLUDED
