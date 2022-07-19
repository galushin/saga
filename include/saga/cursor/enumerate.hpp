#ifndef Z_SAGA_CURSOR_ENUMERATE_HPP_INCLUDED
#define Z_SAGA_CURSOR_ENUMERATE_HPP_INCLUDED

/** @file saga/cursor/enumerate.hpp
 @brief Адаптер курсора, значениями которого является структура, содержащая индекс и значение
 элемента базового курсора
*/

#include <saga/cursor/cursor_facade.hpp>
#include <saga/cursor/cursor_traits.hpp>

namespace saga
{
    namespace cursor
    {
        namespace detail
        {
            template <class Cursor>
            struct enumerate_cursor_reference
            {
                saga::cursor_difference_t<Cursor> index;
                saga::cursor_reference_t<Cursor> value;
            };
        }
        // namespace detail

        template <class Cursor>
        class enumerate_cursor
         : cursor_facade<enumerate_cursor<Cursor>, detail::enumerate_cursor_reference<Cursor>>
        {
        public:
            // Типы
            using reference = detail::enumerate_cursor_reference<Cursor>;

            // Конструктор
            explicit enumerate_cursor(Cursor cur)
             : base_(std::move(cur))
            {}

            // Курсор ввода
            bool operator!() const
            {
                return !this->base();
            }

            reference front() const
            {
                return {this->index_, this->base().front()};
            }

            void drop_front()
            {
                this->base_.drop_front();
                ++ this->index_;
            }

            // Адаптер курсора
            Cursor const & base() const
            {
                return this->base_;
            }

        private:
            Cursor base_;
            saga::cursor_difference_t<Cursor> index_ = 0;
        };

        template <class Cursor>
        enumerate_cursor<Cursor> enumerate(Cursor cur)
        {
            return enumerate_cursor<Cursor>(std::move(cur));
        }
    }
    // namespace cursor
}
//namespace saga


#endif
// Z_SAGA_CURSOR_ENUMERATE_HPP_INCLUDED
