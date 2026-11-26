# MESI Cache Verification

This repository contains an early C++ prototype for modeling and testing MESI cache coherence. It currently models a single shared integer value and one cache line per core, with coherence behavior exercised through sequential calls in unit tests.

## Current implementation

- MESI states: Modified, Exclusive, Shared, and Invalid.
- A `CoherenceManager` creates one cache per requested core and supports reads and writes by core ID.
- The manager models cache-to-cache sharing, invalidations, and write-back of modified data.
- Coherence bus actions and affected core state changes are recorded in transaction history.
- GoogleTest unit tests cover MESI state transitions, cache and memory initialization, coherence behavior, and transaction history.

This is a functional state model, not yet a full cache or hardware simulator. Each cache contains one line with one `int`; `MainMemory` also stores one `int`. There are no memory addresses, configurable cache capacities, cache hierarchy, actual concurrent core threads, cycle timing, or DRAM model.

## Build and run tests

Requirements: CMake 3.20 or newer, a C++17 compiler, and GoogleTest.

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
