#ifndef Z_SAGA_TEST_INVOKABLE_TRACER_HPP_INCLUDED
#define Z_SAGA_TEST_INVOKABLE_TRACER_HPP_INCLUDED

#include <saga/type_traits.hpp>
#include <saga/functional.hpp>

#include <atomic>
#include <cstddef>

namespace saga
{
    template <class Invocable, class Tag = void>
    class invocable_tracer
    {
    public:
        // Типы
        using count_type = std::ptrdiff_t;

        // Счётчики
        static count_type invoke_count()
        {
            return invocable_tracer::invoke_count_ref();
        }

        // Создание, копирование и уничтожение
        invocable_tracer(Invocable fun)
         : fun_(std::move(fun))
        {}

        // Оператор вызова
        template <class... Args>
        invoke_result_t<Invocable &, Args...>
        operator()(Args&&... args) &
            noexcept(is_nothrow_invocable<Invocable &, Args...>{})
        {
            ++ invocable_tracer::invoke_count_ref();

            return saga::invoke(this->fun_, std::forward<Args>(args)...);
        }

        template <class... Args>
        invoke_result_t<Invocable const &, Args...>
        operator()(Args&&... args) const &
            noexcept(is_nothrow_invocable<Invocable const &, Args...>{})
        {
            ++ invocable_tracer::invoke_count_ref();

            return saga::invoke(this->fun_, std::forward<Args>(args)...);
        }

        template <class... Args>
        invoke_result_t<Invocable &&, Args...>
        operator()(Args&&... args) &&
            noexcept(is_nothrow_invocable<Invocable &&, Args...>{})
        {
            ++ invocable_tracer::invoke_count_ref();

            return saga::invoke(std::move(this->fun_), std::forward<Args>(args)...);
        }

        template <class... Args>
        invoke_result_t<Invocable const &&, Args...>
        operator()(Args&&... args) const &&
            noexcept(is_nothrow_invocable<Invocable const &&, Args...>{})
        {
            ++ invocable_tracer::invoke_count_ref();

            return saga::invoke(std::move(this->fun_), std::forward<Args>(args)...);
        }

    private:
        Invocable fun_;

        using count_impl_type = std::atomic<count_type>;

        static count_impl_type & invoke_count_ref()
        {
            static count_impl_type instance_{0};
            return instance_;
        }
    };

    template <class Tag = void, class Invocable>
    invocable_tracer<Invocable, Tag>
    make_invocable_tracer(Invocable fun)
    {
        return invocable_tracer<Invocable, Tag>(std::move(fun));
    }
}
// namespace saga

#endif
// Z_SAGA_TEST_INVOKABLE_TRACER_HPP_INCLUDED
