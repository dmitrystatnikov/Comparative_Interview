COMPILER 	= clang++ -std=c++20

TEST_DIR 	= test
SRC_DIR		= source

unit_test_run:		unit_test_build
					$(TEST_DIR)/UnitTestHashTable

unit_test_build:	$(TEST_DIR)/UnitTestHashTable.cpp $(SRC_DIR)/*.hpp
					$(COMPILER) $(TEST_DIR)/UnitTestHashTable.cpp -I./$(SRC_DIR) -o $(TEST_DIR)/UnitTestHashTable

perf_test_run:		perf_test_build
					$(TEST_DIR)/PerformanceTestHashTable

perf_test_build:	$(TEST_DIR)/PerformanceTestHashTable.cpp $(SRC_DIR)/*.hpp
					$(COMPILER) $(TEST_DIR)/PerformanceTestHashTable.cpp -I./$(SRC_DIR) -O2 -o $(TEST_DIR)/PerformanceTestHashTable
						