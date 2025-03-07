#ifndef STOCKS_H
#define STOCKS_H

#include <atomic>
#include <string>

// Define OrderType enumeration
enum OrderType {
    BUY,
    SELL
};

// Function declarations
int hashTicker(const char* symbol);
void initEngine();
void addOrder(OrderType type, const char* symbol, int quantity, int price);
void matchOrder();
std::string generateRandomTicker();
void simulateTransactions(int numTransactions, int delayMs);

#endif 