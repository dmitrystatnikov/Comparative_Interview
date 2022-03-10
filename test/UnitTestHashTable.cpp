#include "HashTable.hpp"
#include "TestHashTable.hpp"

#include <cassert>
#include <iterator>
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

constexpr is_equal::empty_type empty_value_0{};

using hash_table_type = open_addressing_hash_set<   int
                                                ,   simple_size_hasher
                                                ,   is_equal
                                                >;

hash_table_type test_hash_set_initialization    ()
{
    constexpr static size_t hash_size = 17;

    hash_table_type table(hash_size, simple_size_hasher(hash_size));

    assert(table.is_empty());
    assert(table.capacity() == hash_size);
    assert(table.size() == 0);

    static_assert(table.empty_value() == empty_value_0.value);
    static_assert(table.predicate()(1,1));
    static_assert(table.predicate()(is_equal::empty_type{}, empty_value_0));
    static_assert(table.predicate()(table.empty_value(), empty_value_0.value));

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

    size_t const original_size = table.size();
    size_t const failed_size = original_size - 1;
    size_t hasher_original_capacity = table.hasher().size();
    auto failed_rebalance = [&table](size_t s, simple_size_hasher const && h) {table.rebalance(s, std::move(h));};
    assert  (   is_exception_thrown<hash_table_type::rebalancing_size_too_small>(
                    failed_rebalance, failed_size, simple_size_hasher(failed_size)
                )
            );
    assert(table.hasher().size() == hasher_original_capacity);
    assert(table.size() == original_size);

    constexpr size_t rebalanced_capacity = 29;
    table.rebalance(rebalanced_capacity, simple_size_hasher(rebalanced_capacity));

    assert(table.capacity() == rebalanced_capacity);
    assert(table.hasher().size() == rebalanced_capacity);

    for (auto value : values)
    {
        assert(value == empty_value_0 || table.find(value) != table.end());
    }

    auto conflict_value = *(values.end() - 2) + rebalanced_capacity;
    assert(table.emplace(conflict_value));
    assert(table.find(conflict_value) != table.end());
    assert(table.size() == original_size + 1);
}

void test_hash_set_erase                        (hash_table_type & table, int value)
{
    auto original_size = table.size();
    assert(table.find(value) != table.end());
    assert(!table.emplace(value));
    assert(table.size() == original_size);
    assert(table.erase(value) == 1);
    assert(table.size() == original_size - 1);
    assert(table.find(value) == table.end());
    assert(table.erase(value) == 0);
    assert(table.size() == original_size - 1);

    assert(table.emplace(value));
    assert(table.find(value) != table.end());
    assert(table.size() == original_size);
}

void test_hash_set_iterators                    (hash_table_type & table, std::initializer_list<int> const & values)
{
    hash_table_type::const_iterator it = table.begin();
    const int conflicting_value = 42;

    assert(*it == 0);
    assert(*(it++) == 0);
    assert(*(--it) == 0);
    assert(*(++it) == 1);
    assert(*(++it) == 2);
    assert(*(++it) == 3);
    assert(*(++it) == 30);
    assert(it != table.end());
    assert(*(++it) == conflicting_value); // Conflicting value
    assert(*(++it) == 13);
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

void test_hash_set_find_value                   (   hash_table_type const & table
                                                ,   std::initializer_list<int> const & values_present
                                                ,   std::initializer_list<int> const & values_missing
                                                )
{
    for (auto value : values_present)
    {
        assert(table.find(value) != table.end());
    }

    for (auto value : values_missing)
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
    unit_test::test_hash_set_find_value(table, {1, 3, -1, 13, 30}, {42, 17, 55, unit_test::empty_value_0});
}