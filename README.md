# Stock Trading Engine

## Abstract

This project matches Buys with Sells in real time using lock-free data structures via C++11 atomic operations. Ticker symbols are hashed and stored in a fixed-sized array, orderBook, to allow for fast look-ups. Order objects are stored in every index of the array, with an atomic expression, inUse, dictating if an index is in use or not.

## Implementation

- Uses C++11 atomics (std::atomic) with a compare-and-swap mechanism to allow multiple threads to modify the order book concurrently without locks.
- The `matchOrder` function scans the order book to find the lowest SELL price and matches any BUY orders greater than or equal to that price.
- The engine can simulate random stock transactions via the `simulateTransactions` wrapper function to demonstrate real-time behavior.
- The engine uses a fixed-size array with 1,024 slots and an [FNV-1a hash function](https://gist.github.com/hwei/1950649d523afd03285c) for ticker placement.

## Files

- **stocks.cpp:** Contains the core implementation of the order book, order addition, order matching, and simulation functions.
- **stocks.hpp:** Declarations for all functions included here.

## Build Instructions

This project uses C++11 and is typically built using a g++ compiler.

1. Open a terminal and navigate to the project directory:
```
   cd /navigate/to/your/directory
```

2. Compile the program:
   ```
   g++ -std=c++11 -o stocks stocks.cpp -lpthread
   ```

3. Run the executable:
   ```
   ./stocks
   ```
