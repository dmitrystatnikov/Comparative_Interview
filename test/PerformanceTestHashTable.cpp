#include "HashTable.hpp"
#include "SpecialIterators.hpp"
#include "TestHashTable.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <random>
#include <type_traits>
#include <unordered_set>
#include <vector>

using namespace std;
using namespace specialized_datatypes;

namespace performance_test
{

class random_number_generator
{
public:

    explicit random_number_generator(uint32_t lower_limit, uint32_t upper_limit)
    : dev   ()
    , rng   (dev())
    , dist  (lower_limit, upper_limit)
    {
    }

    mt19937::result_type operator ()()
    {
        return dist(rng);
    }

private:
    random_device dev;
    mt19937 rng;
    uniform_int_distribution<mt19937::result_type> dist;
};

static vector<uint32_t>
generate_random_numbers (   size_t count
                        ,   uint32_t lower_limit
                        ,   uint32_t upper_limit = numeric_limits<uint32_t>::max()
                        )
{
    random_number_generator rgen(lower_limit, upper_limit);
    vector<uint32_t> random_values;
    random_values.reserve(count);

    utilities::counting_iterator first;
    utilities::counting_iterator last = random_values.capacity();

    generate_n(back_inserter(random_values), count, [&rgen](){return rgen();});

    return random_values;
}

}

using specialized_hash_table_type = open_addressing_hash_set<   int
                                                            ,   unit_test::simple_size_hasher
                                                            ,   unit_test::is_equal
                                                            >;

template <typename TestFunc, typename... ArgsT>
auto timed_test(TestFunc && func, ArgsT&&... args) ->
enable_if_t <   !is_void_v<decltype(func(forward<ArgsT>(args)...))>
            ,   pair<decltype(func(forward<ArgsT>(args)...)), chrono::nanoseconds>
            >
{
    auto start_time = chrono::high_resolution_clock::now();
    auto result     = func(forward<ArgsT>(args)...);
    auto stop_time  = chrono::high_resolution_clock::now();

    chrono::nanoseconds test_duration = stop_time - start_time;
    return make_pair(result, test_duration);
}

template <typename TestFunc, typename... ArgsT>
auto timed_test(TestFunc && func, ArgsT&&... args) ->
enable_if_t<is_void_v<decltype(func(forward<ArgsT>(args)...))>, chrono::nanoseconds>
{
    auto start_time = chrono::high_resolution_clock::now();
    func(forward<ArgsT>(args)...);
    auto stop_time  = chrono::high_resolution_clock::now();
    return stop_time - start_time;
}

int main(int argc, char * argv[])
{
    constexpr size_t range_lower_limit      = 1'000'000'000;
    constexpr size_t range_upper_limit      = 4'000'000'000;
    constexpr size_t pool_size              = 1'000'000;
    constexpr size_t stored_numbers_count   = 100'000;
    constexpr size_t specialized_hash_size  = 400'009;

    auto const rand_numbers_pool  = performance_test::generate_random_numbers(pool_size, range_lower_limit, range_upper_limit);
    auto const rand_indexes       = performance_test::generate_random_numbers(stored_numbers_count, 0, pool_size);

    auto sub_range = rand_numbers_pool;
    sub_range.resize(stored_numbers_count);

    auto [stl_hash, stl_duration] =
        timed_test  (   [&sub_range, stored_numbers_count]()
                        {
                            return unordered_set<uint32_t, unit_test::simple_size_hasher>(sub_range.begin(), sub_range.end(), specialized_hash_size, unit_test::simple_size_hasher(specialized_hash_size));
                        }
                    );
    cout << "stl hash table size:" << stl_hash.size() << ", initialization time: " << stl_duration.count() << endl;

    auto [specialized_hash, specialized_duration] =
        timed_test  (   [&sub_range, specialized_hash_size]()
                        {
                            specialized_hash_table_type
                                specialized_hash    (   specialized_hash_size
                                                    ,   unit_test::simple_size_hasher(specialized_hash_size)
                                                    );
                            for (auto value : sub_range)
                            {
                                specialized_hash.emplace(value);
                            }
                            return specialized_hash;
                        }
                    );
    cout << "specialized hash table size:" << specialized_hash.size() << ", initialization time: " << specialized_duration.count() << endl;

    auto find_in_hash = [&rand_numbers_pool, &rand_indexes]
                        <typename HashT, typename Iter>(HashT const & hash, Iter last)
                        {
                            size_t counter = 0;
                            for (auto index : rand_indexes)
                            {
                                if (hash.find(rand_numbers_pool[index]) != last) ++counter;
                            }
                            return counter;
                        };

    size_t counter = 0;
    tie(counter, stl_duration) = timed_test  (find_in_hash, stl_hash, stl_hash.end());
    cout << "stl hash table found:" << counter << ", search time: " << stl_duration.count() << endl;

    tie(counter, specialized_duration) = timed_test (find_in_hash, specialized_hash, specialized_hash.end());
    cout << "specialized hash table found:" << counter << ", search time: " << specialized_duration.count() << endl;


    return 0;
}