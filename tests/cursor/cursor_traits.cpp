// Тестируемый файл
#include <saga/cursor/cursor_traits.hpp>

// Вспомогательные файлы
#include <saga/cursor/istream_cursor.hpp>
#include <saga/cursor/subrange.hpp>

#include <istream>
#include <forward_list>
#include <list>
#include <vector>

// Тесты
namespace
{
    using InputCursor = saga::istream_cursor<int, std::istream>;
    using ForwardCursor = saga::subrange_cursor<std::forward_list<int>::iterator>;
    using BidirectionalCursor = saga::subrange_cursor<std::list<int>::iterator>;
    using RandomAccessCursor = saga::subrange_cursor<std::vector<int>::iterator>;
}

static_assert(!saga::is_input_cursor<int>{});
static_assert(!saga::is_input_cursor<std::vector<int>>{});
static_assert(saga::is_input_cursor<InputCursor>{});
static_assert(saga::is_input_cursor<ForwardCursor>{});
static_assert(saga::is_input_cursor<BidirectionalCursor>{});
static_assert(saga::is_input_cursor<RandomAccessCursor>{});

static_assert(!saga::is_forward_cursor<int>{});
static_assert(!saga::is_forward_cursor<std::vector<int>>{});
static_assert(!saga::is_forward_cursor<InputCursor>{});
static_assert( saga::is_forward_cursor<ForwardCursor>{});
static_assert( saga::is_forward_cursor<BidirectionalCursor>{});
static_assert( saga::is_forward_cursor<RandomAccessCursor>{});

static_assert(!saga::is_bidirectional_cursor<int>{});
static_assert(!saga::is_bidirectional_cursor<std::vector<int>>{});
static_assert(!saga::is_bidirectional_cursor<InputCursor>{});
static_assert(!saga::is_bidirectional_cursor<ForwardCursor>{});
static_assert( saga::is_bidirectional_cursor<BidirectionalCursor>{});
static_assert( saga::is_bidirectional_cursor<RandomAccessCursor>{});

static_assert(!saga::is_random_access_cursor<int>{});
static_assert(!saga::is_random_access_cursor<std::vector<int>>{});
static_assert(!saga::is_random_access_cursor<InputCursor>{});
static_assert(!saga::is_random_access_cursor<ForwardCursor>{});
static_assert(!saga::is_random_access_cursor<BidirectionalCursor>{});
static_assert( saga::is_random_access_cursor<RandomAccessCursor>{});
