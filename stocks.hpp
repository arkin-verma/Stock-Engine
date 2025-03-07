#ifndef STOCKS_H
#define STOCKS_H

#include <atomic>
#include <string>

// Define MAX_TICKERS constant
#define MAX_TICKERS 1000

// Function declarations
int hashTicker(const char* symbol);
void initEngine();
void addOrder(enum OrderType type, const char* symbol, int quantity, int price);
void matchOrder();
std::string generateRandomTicker();
void simulateTransactions(int numTransactions, int delayMs);

#endif // STOCKS_H