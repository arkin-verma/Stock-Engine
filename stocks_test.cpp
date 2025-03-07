#include <iostream>
#include <cassert>
#include <thread>
#include <vector>
#include <chrono>
#include <ctime>
#include <functional>

// Include all declarations from stocks.cpp
// Note: We need to exclude the main() function to avoid multiple definitions
#include "stocks.h"  // Include the header file that defines MAX_TICKERS
enum OrderType {BUY, SELL};
struct Order {
    std::atomic<bool> inUse;
    char ticker[16];
    std::atomic<OrderType> type;
    std::atomic<int> quantity;
    std::atomic<int> price;
};
extern Order orderBook[MAX_TICKERS];

// Function declarations (these are implemented in stocks.cpp)
extern int hashTicker(const char* symbol);
extern void initEngine();
extern void addOrder(OrderType type, const char* symbol, int quantity, int price);
extern void matchOrder();
extern std::string generateRandomTicker();
extern void simulateTransactions(int numTransactions, int delayMs);

// Test utility function
void printTestResult(const std::string& testName, bool passed) {
    std::cout << "[TEST] " << testName << ": " << (passed ? "PASSED" : "FAILED") << std::endl;
}

// Test functions
void testAddOrder() {
    std::cout << "\n=== Testing addOrder functionality ===\n";
    
    initEngine();
    
    // Test 1: Add a simple order
    addOrder(BUY, "AAPL", 100, 150);
    bool test1 = false;
    for (int i = 0; i < MAX_TICKERS; i++) {
        if (orderBook[i].inUse.load() && strcmp(orderBook[i].ticker, "AAPL") == 0 &&
            orderBook[i].type.load() == BUY && 
            orderBook[i].quantity.load() == 100 &&
            orderBook[i].price.load() == 150) {
            test1 = true;
            break;
        }
    }
    printTestResult("Add simple order", test1);
    
    // Test 2: Update existing order
    addOrder(BUY, "AAPL", 200, 160);
    bool test2 = false;
    for (int i = 0; i < MAX_TICKERS; i++) {
        if (orderBook[i].inUse.load() && strcmp(orderBook[i].ticker, "AAPL") == 0 &&
            orderBook[i].type.load() == BUY && 
            orderBook[i].quantity.load() == 200 &&
            orderBook[i].price.load() == 160) {
            test2 = true;
            break;
        }
    }
    printTestResult("Update existing order", test2);
    
    // Test 3: Error handling - invalid quantity
    bool test3 = false;
    try {
        addOrder(SELL, "MSFT", -10, 100);
    } catch (const std::runtime_error&) {
        test3 = true;
    }
    printTestResult("Invalid quantity throws exception", test3);
}

void testMatchOrder() {
    std::cout << "\n=== Testing matchOrder functionality ===\n";
    
    initEngine();
    
    // Add some test orders
    addOrder(SELL, "AAPL", 100, 150);  // Lowest sell
    addOrder(SELL, "GOOGL", 200, 200);
    addOrder(BUY, "AAPL", 150, 140);   // Below lowest sell
    addOrder(BUY, "MSFT", 300, 160);   // Above lowest sell
    
    // Redirect cout to capture output
    std::streambuf* oldCout = std::cout.rdbuf();
    std::stringstream capturedOutput;
    std::cout.rdbuf(capturedOutput.rdbuf());
    
    matchOrder();
    
    // Restore original cout
    std::cout.rdbuf(oldCout);
    
    std::string output = capturedOutput.str();
    printTestResult("Match reports lowest sell price", output.find("Lowest SELL price: $150") != std::string::npos);
    printTestResult("Match identifies correct buy order", output.find("Matched BUY order for MSFT") != std::string::npos);
    printTestResult("Non-matching buy order not reported", output.find("AAPL at price $140") == std::string::npos);
}

void testConcurrency() {
    std::cout << "\n=== Testing concurrent operations ===\n";
    
    initEngine();
    
    // Create multiple threads that add orders concurrently
    std::vector<std::thread> threads;
    const int numThreads = 10;
    const int ordersPerThread = 50;
    
    std::atomic<int> successCount(0);
    
    for (int t = 0; t < numThreads; t++) {
        threads.push_back(std::thread([t, ordersPerThread, &successCount]() {
            for (int i = 0; i < ordersPerThread; i++) {
                try {
                    char ticker[16];
                    sprintf(ticker, "T%d%d", t, i % 10);
                    OrderType type = (i % 2 == 0) ? BUY : SELL;
                    addOrder(type, ticker, 100 + i, 100 + i);
                    successCount++;
                } catch (const std::exception&) {
                    // Expected if capacity is reached
                }
            }
        }));
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Successfully added " << successCount << " orders across " 
              << numThreads << " threads\n";
    
    bool addedSomeOrders = (successCount > 0);
    printTestResult("Concurrent order addition", addedSomeOrders);
    
    // Test that the matchOrder works with concurrently added orders
    matchOrder();
}

void testEdgeCases() {
    std::cout << "\n=== Testing edge cases ===\n";
    
    initEngine();
    
    // Test hash collisions by forced linear probing
    int idx1 = hashTicker("TEST1");
    
    // Force-add an unrelated order at idx1
    orderBook[idx1].inUse.store(true, std::memory_order_relaxed);
    strcpy(orderBook[idx1].ticker, "FORCE");
    
    // Now add TEST1 which will need to find another slot
    bool test1 = true;
    try {
        addOrder(BUY, "TEST1", 100, 100);
    } catch (...) {
        test1 = false;
    }
    
    printTestResult("Linear probing handles hash collision", test1);
    
    // Test behavior with long ticker symbols
    bool test2 = true;
    try {
        addOrder(SELL, "VERYLONGTICKERNAME", 100, 100);
    } catch (...) {
        test2 = false;
    }
    
    printTestResult("Handles long ticker symbols safely", test2);
}

int main() {
    std::cout << "===============================\n";
    std::cout << "  STOCK TRADING ENGINE TESTS   \n";
    std::cout << "===============================\n";
    
    testAddOrder();
    testMatchOrder();
    testConcurrency();
    testEdgeCases();
    
    std::cout << "\nAll tests completed!\n";
    return 0;
}