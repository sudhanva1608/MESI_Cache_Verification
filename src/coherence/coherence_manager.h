#ifndef COHERENCE_MANAGER_H
#define COHERENCE_MANAGER_H

#include <vector>
#include <cstddef>

#include "../cache/cache.h"
#include "../simulator/main_memory.h"

struct CacheStateChange {
    std::size_t coreId;
    MESIState stateBefore;
    MESIState stateAfter;
};

// Represents one bus transaction.
struct BusTransaction {
    std::size_t sequenceNumber;
    std::size_t requesterCore;
      int address;
    CoherenceAction action;

    MESIState stateBefore;
    MESIState stateAfter;

    int dataBefore;
    int dataAfter;

    std::vector<std::size_t> invalidatedCores;
    std::vector<CacheStateChange> affectedCores;
};

class CoherenceManager {
private:
    std::vector<Cache> caches;
    MainMemory memory;

    std::vector<BusTransaction> transactionHistory;
    std::size_t nextSequenceNumber = 1;

    void validateCoreId(std::size_t coreId) const;

public:
    CoherenceManager(std::size_t coreCount,
                     int initialMemoryValue = 0);

    // Existing single-address interface
    int read(std::size_t coreId);
    void write(std::size_t coreId, int value);

    // New multi-address interface
    int read(std::size_t coreId, int address);
    void write(std::size_t coreId, int address, int value);

    const Cache& getCache(std::size_t coreId) const;

    int readMemory() const;
    int readMemory(int address) const;

    const std::vector<BusTransaction>&
    getTransactionHistory() const;
};

#endif
