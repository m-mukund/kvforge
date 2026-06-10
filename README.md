# KVForge
A Redis-inspired in-memory KV store written in modern C++17/20. Features a custom TCP server, RESP protocol parser, concurrent hash map, TTL expiry, and LRU eviction. b\Built from scratch.


## Core Features
 
- **Thread-Safe LRU Cache:** A fixed-capacity in-memory database that automatically evicts the least recently used keys when full. Safely handles simultaneous reads and writes using C++ standard library concurrency primitives.
- **Custom RESP Parser:** Implements the Redis Serialization Protocol (RESP). It efficiently processes client requests and fully supports network pipelining (processing multiple batched commands in a single buffer).
- **Asynchronous AOF Persistence (Write-Ahead Log):** Ensures data durability without sacrificing performance. Write commands are queued in memory and flushed to an Append-Only File (`.aof`) by a dedicated background thread, allowing the main engine to operate at peak speed. The database seamlessly recovers state from this file on startup.
- **Custom Thread Pool:** Manages multiple concurrent client connections efficiently without the overhead of spawning a new thread for every request.
- **Automated CI/CD:** Fully integrated with GitHub Actions to automatically build the project and execute the GoogleTest suite on every push to the main branch.
---

## Technology Stack
 
| Component | Technology / Detail |
|---|---|
| Language | C++17 |
| Build System | CMake |
| Testing Framework | GoogleTest (GTest) |
| Concurrency | `<thread>`, `<mutex>`, `<condition_variable>` |
| CI/CD | GitHub Actions (Ubuntu runners) |
 
---

##  Building the Project
 
KVForge uses CMake for a clean, cross-platform build process.
 
### Prerequisites
 
- A C++17 compatible compiler (GCC, Clang, or MSVC)
- CMake (v3.10 or higher)

### Build Instructions
 
1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/kvforge.git
   cd kvforge
   ```
 
2. Generate the build files:
   ```bash
   cmake -B build
   ```
 
3. Compile the project (using multiple cores for speed):
   ```bash
   cmake --build build --parallel 4
   ```
 
---

## Running the Tests
 
KVForge features an automated test suite verifying the parsing logic, LRU eviction, and disk recovery processes.
 
To run the tests, execute the compiled test binary from your build directory:
 
```bash
cd build
./KVForgeTests
```
 
**Expected Output:**
```
[==========] Running 6 tests from 3 test suites.
[  PASSED  ] 6 tests.
```
 
---

## Supported Commands
 
KVForge currently supports the following core network commands via RESP:
 
| Command | Description |
|---|---|
| `SET <key> <value>` | Stores the value in the database. |
| `GET <key>` | Retrieves the value associated with the key. Returns a null response if the key does not exist. |
| `DEL <key>` | Removes the key and its associated value from the database. |
 
---
 
