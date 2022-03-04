#include "HashTable.hpp"
#include "TestHashTable.hpp"

#include <cassert>
#include <functional>
#include <iterator>
#include <type_traits>
#include <utility>

#include <iostream>

using namespace specialized_datatypes;

namespace unit_test
{

template <typename ExceptionT, typename FunctionT, typename... ArgsT>
bool is_exception_thrown(FunctionT func, ArgsT&&... args)
{
    bool is_thrown = false;

    try
    {
        func(std::forward<ArgsT>(args)...);
    }
    catch (ExceptionT& ex)
    {
        return true;
    }

    return false;
}

constexpr std::integral_constant<int, 0> empty_value_0{};

using hash_table_type = open_addressing_hash_set<   int
                                                ,   simple_size_hasher
                                                ,   std::equal_to<int>
                                                ,   std::integral_constant<int, 0>
                                                >;

hash_table_type test_hash_set_initialization    ()
{
    constexpr static size_t hash_size = 17;

    hash_table_type table(hash_size, simple_size_hasher(hash_size));

    assert(table.is_empty());
    assert(table.capacity() == hash_size);
    assert(table.size() == 0);

    static_assert(table.empty_value() == 0);
    static_assert(table.predicate()(1,1));
    static_assert(table.predicate()(0, empty_value_0));
    static_assert(table.predicate()(table.empty_value(), empty_value_0));

    assert(table.hasher()(13) == table.hasher()(30));

    return table;
}

void test_hash_set_value_insert                 (hash_table_type & table, std::initializer_list<int> const & values)
{
    auto current_size = table.size();
    for (auto value : values)
    {
        if (value == empty_value_0)
            assert(!table.emplace(value));
        else
        {
            assert(table.emplace(value));
            ++current_size;
        }
    }
    assert(table.size() == current_size);

    for (auto value : values)
    {
        assert(value == empty_value_0 || table.find(value) != table.end());
    }

    for (auto value : values)
    {
        assert(!table.emplace(value));
    }
    assert(table.size() == current_size);
}

void test_hash_set_rebalance                    (hash_table_type & table, std::initializer_list<int> const & values)
{
    for (auto value : values)
    {
        assert(value == empty_value_0 || table.find(value) != table.end());
    }

    size_t failed_size = table.size() - 1;
    size_t hasher_original_size = table.hasher().size();
    auto failed_rebalance = [&table](size_t s, simple_size_hasher const && h) {table.rebalance(s, std::move(h));};
    assert  (   is_exception_thrown<hash_table_type::rebalancing_size_too_small>(
                    failed_rebalance, failed_size, simple_size_hasher(failed_size)
                )
            );
    assert(table.hasher().size() == hasher_original_size);

    constexpr size_t rebalanced_size = 29;
    table.rebalance(rebalanced_size, simple_size_hasher(rebalanced_size));

    assert(table.capacity() == rebalanced_size);
    assert(table.hasher().size() == rebalanced_size);

    for (auto value : values)
    {
        assert(value == empty_value_0|| table.find(value) != table.end());
    }

    auto conflict_value = *(values.end() - 2) + rebalanced_size;
    assert(table.emplace(conflict_value));
    assert(table.find(conflict_value) != table.end());
}

void test_hash_set_erase                        (hash_table_type & table, int value)
{
    assert(table.find(value) != table.end());
    assert(!table.emplace(value));
    assert(table.erase(value) == 1);
    assert(table.find(value) == table.end());
    assert(table.erase(value) == 0);

    assert(table.emplace(value));
    assert(table.find(value) != table.end());
}

void test_hash_set_iterators                    (hash_table_type & table, std::initializer_list<int> const & values)
{
    hash_table_type::const_iterator it = table.begin();
    const int conflicting_value = 42;

    assert(*it == 1);
    assert(*(it++) == 1);
    assert(*(--it) == 1);
    assert(*(++it) == 2);
    assert(*(++it) == 3);
    assert(*(++it) == 30);
    assert(*(++it) == 13);
    assert(it != table.end());
    assert(*(++it) == conflicting_value); // Conflicting value
    assert(*(++it) == -1);
    assert(it != table.end());
    ++it;
    assert(it == table.end());
    while (std::distance(table.begin(), it) > 0) --it;
    assert(it == table.begin());
    --it;
    assert(it==table.end());

    for (auto v : table)
    {
        assert(v == conflicting_value || std::find(values.begin(), values.end(), v) != values.end());
    }
}

void test_hash_set_value_erase                  (hash_table_type & table, std::initializer_list<int> const & erased_values)
{
    for (auto value : erased_values)
    {
        if (table.find(value) == table.end())
            assert(table.erase(value) == 0);
        else
            assert(table.erase(value) == 1);
    }

    for (auto value : erased_values)
    {
        assert(table.find(value) == table.end());
    }
}

}

int main(int argc, char * argv[])
{
    auto table = unit_test::test_hash_set_initialization();

    std::initializer_list<int> values {1, 2, 3, 0, -1, 13, 30};
    
    unit_test::test_hash_set_value_insert(table, values);
    unit_test::test_hash_set_rebalance(table, values);
    unit_test::test_hash_set_erase(table, 13);
    unit_test::test_hash_set_iterators(table, values);
    unit_test::test_hash_set_value_erase(table, {2, 42, 17});
}