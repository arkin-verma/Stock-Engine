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
static Order orderBook[MAX_TICKERS];

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

// Initialize all Order structs in orderBook
void initEngine() {
    for (int i = 0; i < MAX_TICKERS; ++i) {
        orderBook[i].inUse.store(false, std::memory_order_relaxed);
        orderBook[i].quantity.store(0, std::memory_order_relaxed);
        orderBook[i].price.store(0, std::memory_order_relaxed);
        std::memset(orderBook[i].ticker, 0, sizeof(orderBook[i].ticker));
    }
}

// Add or update an order for a ticker
void addOrder(OrderType type, const char* symbol, int quantity, int price) {
    if (quantity <= 0 || price <= 0) throw std::runtime_error("Invalid quantity/price: cannot be negative. ");

    int startIndex = hashTicker(symbol);
    int idx = startIndex;
    for (int attempt = 0; attempt < MAX_TICKERS; ++attempt) {
        bool expected = false;

        // If current index is available
        if (!orderBook[idx].inUse.load(std::memory_order_relaxed)) {
            // Attempt to claim this index
            if (orderBook[idx].inUse.compare_exchange_strong(expected, true)) {
                // Write atomic values
                // Use strncpy to avoid buffer overflow
                std::strncpy(orderBook[idx].ticker, symbol, sizeof(orderBook[idx].ticker) - 1);
                orderBook[idx].ticker[sizeof(orderBook[idx].ticker) - 1] = '\0';
                orderBook[idx].type.store(type, std::memory_order_relaxed);
                orderBook[idx].quantity.store(quantity, std::memory_order_relaxed);
                orderBook[idx].price.store(price, std::memory_order_relaxed);
                return;
            }
        } else {
            // Check if same ticker
            if (std::strncmp(orderBook[idx].ticker, symbol, sizeof(orderBook[idx].ticker)) == 0) {
                // Update existing ticker
                orderBook[idx].type.store(type, std::memory_order_relaxed);
                orderBook[idx].quantity.store(quantity, std::memory_order_relaxed);
                orderBook[idx].price.store(price, std::memory_order_relaxed);
                return;
            }
        }
        idx = (idx + 1) % MAX_TICKERS;
    }
    throw std::runtime_error("No more capacity for new ticker.");
}

void matchOrder() {
    int lowestSellPrice = INT_MAX;

    // First pass: find min SELL price
    for (int i = 0; i < MAX_TICKERS; ++i) {
        if (!orderBook[i].inUse.load(std::memory_order_relaxed)) continue;
        int p = orderBook[i].price.load(std::memory_order_relaxed);
        if (orderBook[i].type.load(std::memory_order_relaxed) == SELL) {
            if (p < lowestSellPrice) {
                lowestSellPrice = p;
            }
        }
        }
    
    // If no SELL orders found, return
    if (lowestSellPrice == INT_MAX) {
        std::cout << "No SELL orders found." << std::endl;
        return;
    }

    // Print the lowest SELL price
    std::cout << "Lowest SELL price: " << lowestSellPrice << std::endl;

    // Second pass: Match orders by looking for BUY orders with price >= lowest SELL price
    for (int i = 0; i < MAX_TICKERS; ++i) {
        if (!orderBook[i].inUse.load(std::memory_order_relaxed)) continue;
        int p = orderBook[i].price.load(std::memory_order_relaxed);
        if (orderBook[i].type.load(std::memory_order_relaxed) == BUY) {
            if (p >= lowestSellPrice) {
                std::cout << "Matched BUY order for " << orderBook[i].ticker << " at price " << p << std::endl;
            }
        }
    }
}

// Function to generate a random ticker symbol
std::string generateRandomTicker() {
    static const char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string ticker;
    int length = 3 + rand() % 4; // 3-6 characters
    
    for (int i = 0; i < length; ++i) {
        ticker += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    
    return ticker;
}

// Function to simulate random stock transactions
void simulateTransactions(int numTransactions, int delayMs = 100) {
    // Seed the random number generator
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    
    std::cout << "Starting transaction simulation with " << numTransactions << " orders..." << std::endl;
    
    for (int i = 0; i < numTransactions; ++i) {
        // Generate random order parameters
        OrderType type = (rand() % 2 == 0) ? BUY : SELL;
        std::string ticker = generateRandomTicker();
        int quantity = 100 * (1 + rand() % 100); // 100-10000 shares
        int price = 10 + rand() % 990; // $10-$1000
        
        try {
            // Add the order to the order book
            addOrder(type, ticker.c_str(), quantity, price);
            
            // Print the order details
            std::cout << "Order #" << i+1 << ": " 
                      << (type == BUY ? "BUY" : "SELL") << " "
                      << quantity << " shares of " << ticker
                      << " at $" << price << std::endl;
            
            // Match orders periodically
            if (i % 5 == 4) {
                std::cout << "\n--- Matching orders ---" << std::endl;
                matchOrder();
                std::cout << "----------------------\n" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error adding order #" << i+1 << ": " << e.what() << std::endl;
        }
        
        // Add a small delay between transactions
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
    }
    
    std::cout << "Transaction simulation complete." << std::endl;
}

// Example main function to demonstrate the simulation
int main() {
    initEngine();
    simulateTransactions(50, 200); // 50 transactions with 200ms delay
    return 0;
}