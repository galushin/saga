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

// Используемые возможности
#include <saga/algorithm.hpp>
#include <saga/cursor/iota.hpp>
#include <saga/cursor/take_while.hpp>
#include <saga/cursor/to.hpp>
#include <saga/cursor/transform.hpp>
#include <saga/numeric.hpp>

// Инфраструктура тестирования
#include <catch2/catch_amalgamated.hpp>

// PE 061 - Цикличные фигурные числа
namespace
{
    template <class UnaryFunction>
    std::vector<int>
    PE_061_prepare(UnaryFunction fun)
    {
        auto cur = saga::cursor::iota(1)
                 | saga::cursor::transform(std::move(fun));

        return saga::find_if(std::move(cur), [](int const & num) {return num >= 1000;})
               | saga::cursor::take_while([](int const & num) { return num < 10000; })
               | saga::cursor::to<std::vector>();
    }

    bool is_chained(int lhs, int rhs)
    {
        assert(1000 <= lhs && lhs < 10'000);
        assert(1000 <= rhs && rhs < 10'000);

        return lhs % 100 == rhs / 100;
    }

    int PE_061_recursion(std::vector<std::vector<int>> points
                         , std::vector<int> const & indices
                         , std::vector<int> & path)
    {
        assert(!path.empty());

        if(path.size() == indices.size())
        {
            if(::is_chained(path.back(), path.front()))
            {
                return saga::reduce(saga::cursor::all(path));
            }
            else
            {
                return 0;
            }
        }

        for(auto const & point : points[indices[path.size()]])
        {
            if(::is_chained(path.back(), point))
            {
                path.push_back(point);

                auto result = ::PE_061_recursion(points, indices, path);

                path.pop_back();

                if(result != 0)
                {
                    return result;
                }
            }
        }

        return 0;
    }

    int PE_061_start(std::vector<std::vector<int>> points
                     , std::vector<int> const & indices)
    {
        assert(!points.empty());

        std::vector<int> path;

        for(auto const & start : points[0])
        {
            path.push_back(start);

            auto result = PE_061_recursion(points, indices, path);

            if(result != 0)
            {
                return result;
            }

            path.pop_back();
        }

        return 0;
    }

    template <class... IntVector>
    int PE_061(IntVector const &... args)
    {
        std::vector<std::vector<int>> points{args...};

        auto indices = saga::cursor::indices(0, static_cast<int>(points.size()))
                     | saga::cursor::to<std::vector<int>>();

        auto tail = saga::cursor::all(indices);
        tail.drop_front();

        do
        {
            auto result = PE_061_start(points, indices);

            if(result != 0)
            {
                return result;
            }
        }
        while(saga::next_permutation(tail));

        return 0;
    }

    struct heptagonal_number_fn
    {
        template <class IntType>
        IntType operator()(IntType const & num) const
        {
            return 5 * saga::triangular_number(num) - 4 * num;
        }
    };

    inline constexpr auto heptagonal_number = heptagonal_number_fn{};

    struct octagonal_number_fn
    {
        template <class IntType>
        IntType operator()(IntType const & num) const
        {
            return num * (3*num - 2);
        }
    };

    inline constexpr auto octagonal_number = octagonal_number_fn{};
}

TEST_CASE("PE 061")
{
    auto const nums_3 = ::PE_061_prepare(saga::triangular_number);
    auto const nums_4 = ::PE_061_prepare(saga::square);
    auto const nums_5 = ::PE_061_prepare(saga::pentagonal_number);
    auto const nums_6 = ::PE_061_prepare(saga::hexagonal_number);
    auto const nums_7 = ::PE_061_prepare(::heptagonal_number);
    auto const nums_8 = ::PE_061_prepare(::octagonal_number);

    CAPTURE(nums_3, nums_4, nums_5, nums_6, nums_7, nums_8);

    REQUIRE(PE_061(nums_4) == 0);
    REQUIRE(PE_061(nums_3, nums_4, nums_5) == 8128 + 2882 + 8281);
    REQUIRE(PE_061(nums_3, nums_4, nums_5, nums_6, nums_7, nums_8) == 28684);
}
