#include <iostream>
#include <thread>
#include <atomic>
#include <stdexcept>
#include <cstring>
#include <cstdlib>

static const int MAX_TICKERS = 1024;

// Initialize Order Type enum
enum OrderType {BUY, SELL};

// Initialize Order struct
struct Order {
    std::atomic<bool> inUse;
    char ticker[16];
    std::atomic<OrderType> type;
    std::atomic<int> quantity;
    std::atomic<int> price;
};

// Global array of Buy/Sell Orders
// addOrder will modify this array
static Order gOrders[MAX_TICKERS];

// Hash function to store tickers to specific indices
// Used FNV-1a hash algorithm
int hashTicker(const char* symbol) {
    // FNV-1a hash algorithm constants
    const uint32_t FNV_PRIME = 16777619;
    const uint32_t FNV_OFFSET_BASIS = 2166136261;
    
    uint32_t hash = FNV_OFFSET_BASIS;
    
    // Calculate hash considering character position
    for (int i = 0; symbol[i] != '\0'; ++i) {
        hash ^= static_cast<uint32_t>(symbol[i]);
        hash *= FNV_PRIME;
    }
    
    return hash % MAX_TICKERS;
}

// Initialize all Order structs in gOrders
void initEngine() {
    for (int i = 0; i < MAX_TICKERS; ++i) {
        gOrders[i].inUse.store(false, std::memory_order_relaxed);
        gOrders[i].quantity.store(0, std::memory_order_relaxed);
        gOrders[i].price.store(0, std::memory_order_relaxed);
        std::memset(gOrders[i].ticker, 0, sizeof(gOrders[i].ticker));
    }
}