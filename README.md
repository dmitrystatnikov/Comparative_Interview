# Fast Datatypes

This is an implementation of (potentially) faster hash table (set) comparing to STL hash tables implementation.

# Content

This repo contains implementation of aforementioned hash table in source/HashTable.hpp

test/UnitTestHashTable.cpp - unit tests for specialized_datatypes::open_addressing_hash_set
Purpose: test correctness and catch any regression due to code changes.

test/PerformanceTestHashTable.cpp - testing performance against STL hash table implementation.
Purpose: test performance comparatively to std::unordered_set

# Build and testing

To run unit test:

make unit_test_run

To run performance test:

make perf_test_run
