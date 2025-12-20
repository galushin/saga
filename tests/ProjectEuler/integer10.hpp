/* (c) 2022-2025 Галушин Павел Викторович, galushin@gmail.com

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

#ifndef Z_TEST_PE_INTEGER10_HPP_INCLUDED
#define Z_TEST_PE_INTEGER10_HPP_INCLUDED

#include <saga/action/reverse.hpp>
#include <saga/numeric/digits_of.hpp>
#include <saga/numeric.hpp>

#include <charconv>
#include <iomanip>

namespace saga
{
namespace experimental
{
    template <class IntType>
    IntType from_chars_whole(char const * first, char const * last)
    {
        assert(first != last);

        auto reader = IntType(0);
        auto result = std::from_chars(first, last, reader);

        assert(result.ptr == last);
        assert(result.ec == std::errc{});

        return reader;
    }

    template <class IntType>
    IntType from_string_whole(std::string_view str)
    {
        return saga::experimental::from_chars_whole<IntType>(str.data(), str.data() + str.size());
    }

    class integer10
    {
        // Равенство
        friend bool operator==(integer10 const & lhs, integer10 const & rhs)
        {
            return lhs.units_ == rhs.units_;
        }

        template <class IntType>
        friend auto operator==(integer10 const & lhs, IntType rhs)
        -> std::enable_if_t<std::is_integral<IntType>{}, bool>
        {
            return saga::equal(saga::cursor::all(lhs.data())
                              , saga::cursor::digits_of(rhs, lhs.unit_base));
        }

        // Вывод
        template <class CharT, class Traits>
        friend std::basic_ostream<CharT, Traits> &
        operator<<(std::basic_ostream<CharT, Traits> & out, integer10 const & value)
        {
            auto num = value.units_.size();

            if(num == 0)
            {
                out << '0';
                return out;
            }

            out << value.units_.back();
            -- num;

            for(; num > 0; -- num)
            {
                out << std::setw(value.digits_per_unit)
                    << std::setfill('0')
                    << value.units_[num - 1];
            }

            return out;
        }

        // Сложение
        friend integer10 operator+(integer10 lhs, integer10 const & rhs)
        {
            lhs += rhs;

            return lhs;
        }

        // Умножение
        friend integer10 operator*(integer10 const & lhs, integer10 const & rhs)
        {
            if(lhs.units_.empty() || rhs.units_.empty())
            {
                return integer10{};
            }

            auto result = integer10{};

            auto carry = Unit(0);

            auto const lhs_size = lhs.units_.size();
            auto const rhs_size = rhs.units_.size();

            for(auto pos : saga::cursor::indices(lhs_size + rhs_size))
            {
                auto new_unit = carry % unit_base;
                carry /= unit_base;

                auto const first = rhs_size <= pos ? pos - rhs_size + 1 : 0 * pos;
                auto const last = std::min(lhs_size, pos + 1);

                for(auto const & index : saga::cursor::indices(first, last))
                {
                    auto prod = lhs.units_[index] * rhs.units_[pos - index];

                    assert(prod < std::numeric_limits<Unit>::max() - new_unit);

                    new_unit += prod;
                }

                result.units_.push_back(new_unit % unit_base);
                carry += new_unit / unit_base;
            }

            while(!result.units_.empty() && result.units_.back() == 0)
            {
                assert(!result.units_.empty());

                result.units_.pop_back();
            }

            assert(carry == 0);

            return result;
        }

        template <class IntType>
        friend auto operator*(integer10 const & lhs, IntType rhs)
        -> std::enable_if_t<std::is_integral<IntType>{}, integer10>
        {
            return lhs * integer10(std::move(rhs));
        }


    private:
        using Unit = std::uint64_t;

        static_assert(std::is_unsigned<Unit>{});

    public:
        // @todo Сделать настраиваемым через шаблонный параметр,
        // Добавить проверку, что бит достаточно для хранения квадрата
        static constexpr auto digits_per_unit = 9;

        // Создание, копирование, уничтожение
        integer10() = default;

        template <class IntType, class = std::enable_if_t<std::is_integral<IntType>{}>>
        explicit integer10(IntType value)
        {
            assert(value >= 0);

            saga::copy(saga::cursor::digits_of(value, unit_base)
                      , saga::back_inserter(this->units_));
        }

        explicit integer10(std::string_view str)
        {
            auto const tail_size = str.size() % this->digits_per_unit;
            auto const units_count = str.size() / this->digits_per_unit + (tail_size != 0);

            this->units_.reserve(units_count);

            auto first = str.data();
            auto last = str.data() + (tail_size == 0 ? digits_per_unit : tail_size);
            auto const stop = str.data() + str.size();

            for(; first != stop; first = last, last += digits_per_unit)
            {
                this->units_.push_back(saga::experimental::from_chars_whole<Unit>(first, last));
            }

            this->units_ |= saga::action::reverse;
        }

        // Арифметические операции
        integer10 & operator+=(integer10 const & rhs)
        {
            auto const num = std::max(this->units_.size(), rhs.units_.size());

            this->units_.resize(num, 0);

            auto carry = Unit(0);

            for(auto index : saga::cursor::indices(rhs.units_.size()))
            {
                carry += this->units_[index] + rhs.units_[index];

                this->units_[index] = carry % unit_base;

                carry /= unit_base;
            }

            for(auto index : saga::cursor::indices(rhs.units_.size(), this->units_.size()))
            {
                carry += this->units_[index];

                this->units_[index] = carry % unit_base;

                carry /= unit_base;
            }

            assert(carry < unit_base);

            if(carry > 0)
            {
                this->units_.push_back(carry);
            }

            return *this;
        }

        integer10 & operator*=(integer10 const & rhs)
        {
            *this = *this * rhs;

            return *this;
        }

        // Остаток
        void mod10(std::size_t power)
        {
            auto const tail_size = power % this->digits_per_unit;
            auto const units_to_keep = power / this->digits_per_unit + (tail_size != 0);

            if(this->units_.size() >= units_to_keep)
            {
                this->units_.resize(units_to_keep);

                if(tail_size > 0)
                {
                    this->units_.back() %= saga::power_natural(10, tail_size);
                }
            }
        }

        // Доступ к представлению
        std::vector<Unit> const & data() const
        {
            return this->units_;
        }

    private:
        static constexpr auto unit_base = saga::power_natural(Unit(10), digits_per_unit);

        std::vector<Unit> units_;
    };

    inline long digits_sum(integer10 const & num)
    {
        long digits_sum = 0;

        for(auto const & unit : saga::cursor::all(num.data()))
        {
            digits_sum += saga::reduce(saga::cursor::digits_of(unit));
        }

        return digits_sum;
    }
}
}

#endif
// Z_TEST_PE_INTEGER10_HPP_INCLUDED
