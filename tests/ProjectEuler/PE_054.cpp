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
#include <saga/action/sort.hpp>
#include <saga/algorithm.hpp>
#include <saga/cursor/by_line.hpp>
#include <saga/cursor/indices.hpp>
#include <saga/cursor/subrange.hpp>
#include <saga/functional.hpp>
#include <saga/numeric.hpp>

#include <fstream>

// Инфраструктура тестирования
#include <catch2/catch_amalgamated.hpp>

// PE 054 - Покерные комбинации
namespace
{
    struct poker_card
    {
        int value = 0;
        char suit = 0;
    };

    struct poker_hand
    {
        std::array<poker_card, 5> cards;
    };

    bool is_poker_flush(poker_hand const & hand)
    {
        auto pred = [suit = hand.cards.front().suit](poker_card const & card)
            { return card.suit == suit; };

        return saga::all_of(saga::cursor::all(hand.cards), pred);
    }

    bool is_poker_straight(poker_hand const & hand)
    {
        for(auto index : saga::cursor::indices(1, hand.cards.size()))
        {
            if(hand.cards[index-1].value != hand.cards[index].value + 1)
            {
                return false;
            }
        }

        return true;
    }

    bool less_by_highest(poker_hand const & lhs, poker_hand const & rhs)
    {
        auto const cmp = saga::compare_by(&poker_card::value, std::less<>{});

        return saga::lexicographical_compare(saga::cursor::all(lhs.cards)
                                            ,saga::cursor::all(rhs.cards), cmp);
    }

    struct poker_hand_summary
    {
        int four = 0;
        int triple = 0;
        int high_pair = 0;
        int low_pair = 0;
    };

    poker_hand_summary make_poker_hand_summary(poker_hand const & hand)
    {
        std::vector<int> freqs(13, 0);

        for(auto const & card : hand.cards)
        {
            freqs.at(card.value - 2) += 1;
        }

        poker_hand_summary result;

        for(auto index : saga::cursor::indices_of(freqs))
        {
            auto const value = index + 2;

            if(freqs[index] == 4)
            {
                result.four = value;
            }
            else if(freqs[index] == 3)
            {
                result.triple = value;
            }
            else if(freqs[index] == 2)
            {
                if(result.high_pair == 0)
                {
                    result.high_pair = value;
                }
                else
                {
                    result.low_pair = value;
                    if(result.low_pair > result.high_pair)
                    {
                        using std::swap;
                        swap(result.low_pair, result.high_pair);
                    }
                }
            }
        }

        return result;
    }

    bool operator<(poker_hand const & lhs, poker_hand const & rhs)
    {
        auto const lhs_flush = ::is_poker_flush(lhs);
        auto const rhs_flush = ::is_poker_flush(rhs);

        auto const lhs_straight = ::is_poker_straight(lhs);
        auto const rhs_straight = ::is_poker_straight(rhs);

        // Роял-флэш и Стрит-флэш
        if(lhs_flush && lhs_straight && rhs_flush && rhs_straight)
        {
            return less_by_highest(lhs, rhs);
        }
        if((lhs_flush && lhs_straight) || (rhs_flush && rhs_straight))
        {
            return (rhs_flush && rhs_straight);
        }

        auto const lhs_summary = make_poker_hand_summary(lhs);
        auto const rhs_summary = make_poker_hand_summary(rhs);

        // Четыре карты
        if(lhs_summary.four > 0 && rhs_summary.four > 0)
        {
            // Не может быть 8 карт одного номинала
            assert(lhs_summary.four != rhs_summary.four);

            return lhs_summary.four < rhs_summary.four;
        }
        else if(lhs_summary.four > 0 || rhs_summary.four > 0)
        {
            return rhs_summary.four > 0;
        }

        // Тройка+пара
        if(lhs_summary.triple > 0 && lhs_summary.high_pair > 0
           && rhs_summary.triple > 0 && rhs_summary.high_pair > 0)
        {
            // Не может быть 6 карт одного номинала
            assert(lhs_summary.triple != rhs_summary.triple);

            return lhs_summary.triple < rhs_summary.triple;
        }
        else if((lhs_summary.triple > 0 && lhs_summary.high_pair > 0)
                || (rhs_summary.triple > 0 && rhs_summary.high_pair > 0))
        {
            return (rhs_summary.triple > 0 && rhs_summary.high_pair > 0);
        }

        // Флэш
        if(lhs_flush && rhs_flush)
        {
            return less_by_highest(lhs, rhs);
        }
        if(lhs_flush || rhs_flush)
        {
            return rhs_flush;
        }

        // Стрит
        if(lhs_straight && rhs_straight)
        {
            return less_by_highest(lhs, rhs);
        }
        if(lhs_straight || rhs_straight)
        {
            return rhs_straight;
        }

        // Тройка
        if(lhs_summary.triple > 0 && rhs_summary.triple > 0)
        {
            // Не может быть 6 карт одного номинала
            assert(lhs_summary.triple != rhs_summary.triple);

            return lhs_summary.triple < rhs_summary.triple;
        }
        else if(lhs_summary.triple > 0 || rhs_summary.triple > 0)
        {
            return rhs_summary.triple > 0;
        }

        // Две пары
        if(lhs_summary.low_pair > 0 && rhs_summary.low_pair > 0)
        {
            assert(lhs_summary.high_pair > 0);
            assert(rhs_summary.high_pair > 0);

            if(lhs_summary.high_pair != rhs_summary.high_pair)
            {
                return lhs_summary.high_pair < rhs_summary.high_pair;
            }
            else if(lhs_summary.low_pair != rhs_summary.low_pair)
            {
                return lhs_summary.low_pair < rhs_summary.low_pair;
            }
            else
            {
                return less_by_highest(lhs, rhs);
            }
        }
        if(lhs_summary.low_pair > 0 || rhs_summary.low_pair > 0)
        {
            return rhs_summary.low_pair > 0;
        }

        // Пара
        if(lhs_summary.high_pair != rhs_summary.high_pair)
        {
            return lhs_summary.high_pair < rhs_summary.high_pair;
        }

        // Старшая карта
        return less_by_highest(lhs, rhs);
    }

    int poker_card_value(char arg)
    {
        if(arg == 'T')
        {
            return 10;
        }
        else if(arg == 'J')
        {
            return 11;
        }
        else if(arg == 'Q')
        {
            return 12;
        }
        else if(arg == 'K')
        {
            return 13;
        }
        else if(arg == 'A')
        {
            return 14;
        }
        else
        {
            assert(std::isdigit(arg));

            return int(arg - '0');
        }
    }

    poker_hand parse_poker_hand(std::string_view str)
    {
        assert(str.size() == 14);

        poker_hand hand;

        for(auto index : saga::cursor::indices_of(hand.cards))
        {
            hand.cards[index].value = poker_card_value(str[3*index]);
            hand.cards[index].suit  = str[3*index + 1];
        }

        hand.cards |= saga::action::sort(saga::compare_by(&poker_card::value, std::greater<>{}));

        return hand;
    }

    bool PE_054_line(std::string const & two_hands)
    {
        assert(two_hands.size() >= 29);
        assert(two_hands[14] == ' ');

        auto const lhs = parse_poker_hand(std::string_view(two_hands).substr(0, 14));
        auto const rhs = parse_poker_hand(std::string_view(two_hands).substr(15, 14));

        return rhs < lhs;
    }

    std::size_t PE_054_file(std::string const & path)
    {
        std::ifstream file(path);

        assert(!!file);

        return saga::transform_reduce(saga::cursor::by_line(file), std::size_t(0)
                                     , std::plus<>{}, PE_054_line);
    }
}

TEST_CASE("PE 054")
{
    CHECK(::PE_054_line("5H 5C 6S 7S KD 2C 3S 8S 8D TD") == false);
    CHECK(::PE_054_line("5D 8C 9S JS AC 2C 5C 7D 8S QH") == true);
    CHECK(::PE_054_line("2D 9C AS AH AC 3D 6D 7D TD QD") == false);
    CHECK(::PE_054_line("4D 6S 9H QH QC 3D 6D 7H QD QS") == true);
    CHECK(::PE_054_line("2H 2D 4C 4D 4S 3C 3D 3S 9S 9D") == true);

    CHECK(::PE_054_line("3S 4S 5S 6S 7S 2D 3D 4D 5D 6D") == true);
    CHECK(::PE_054_line("3S 4S 5S 6S 7H 2D 3D 4D 5D 6D") == false);
    CHECK(::PE_054_line("2H 2C 2S 2D AH 3H 3C 3S 3D KH") == false);
    CHECK(::PE_054_line("2H 2C 2S 2D AH 3H 3C 3S KD KH") == true);
    CHECK(::PE_054_line("3S 4S 5S 6S 8S 2D 3D 4D 6D 8D") == true);
    CHECK(::PE_054_line("3S 4S 5D 6S 7S 2D 3D 4S 5D 6D") == true);
    CHECK(::PE_054_line("2H KD 4C 4D 4S 3C 3D 3S 9S QD") == true);
    CHECK(::PE_054_line("2H 2D 4C 4D JS 3C 3D TS 9S 9D") == false);
    CHECK(::PE_054_line("2H 2D 4C 4D JS 3C 3D TS 4S 4H") == false);
    CHECK(::PE_054_line("2H 2D 4C 4D JS 2C 2H TS 3S 3H") == true);
    CHECK(::PE_054_line("3H 3D 4C 4D QS 3S 3D TS 4S 4H") == true);

    CHECK(::PE_054_file("ProjectEuler/p054_poker.txt") == 376);
}
