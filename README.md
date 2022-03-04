# Fast Datatypes

This is an implementation of (potencially) faster hash table (set) comparing to std::unordered_set.

# Content

This repo contains implementation of said hash set in source/HashTable.hpp

test/UnitTestHashTable.cpp - unit tests for specialized_datatypes::open_addressing_hash_set
Purpose: test correctness and catch any regression due to code changes.

test/PerformanceTestHashTable.cpp - testing performance against STL hash table implementation.

# Build and testing

To run unit test:

make unit_test_run

To run performance test:

make perf_test_run
