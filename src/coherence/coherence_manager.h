
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
    CoherenceAction action;

   MESIState stateBefore;
    MESIState stateAfter;
std::vector<std::size_t> invalidatedCores;
 std::vector<CacheStateChange> affectedCores;
};


class CoherenceManager {
private:
    std::vector<Cache> caches;
    MainMemory memory;

    // Stores the history of bus transactions.
    std::vector<BusTransaction> transactionHistory;
    std::size_t nextSequenceNumber = 1;
    void validateCoreId(std::size_t coreId) const;

public:
    CoherenceManager(std::size_t coreCount,
                     int initialMemoryValue = 0);

    int read(std::size_t coreId);

    void write(std::size_t coreId, int value);

    const Cache& getCache(std::size_t coreId) const;

    int readMemory() const;

    // Allows read-only access to transaction history.
    const std::vector<BusTransaction>&
    getTransactionHistory() const;
};

#endif

