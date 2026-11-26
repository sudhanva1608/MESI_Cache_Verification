
#include "coherence_manager.h"

#include <stdexcept>

CoherenceManager::CoherenceManager(
    std::size_t coreCount,
    int initialMemoryValue
) : caches(coreCount) {

    if (coreCount == 0) {
        throw std::invalid_argument(
            "CoherenceManager requires at least one core"
        );
    }

    memory.write(initialMemoryValue);
}
void CoherenceManager::validateCoreId(
    std::size_t coreId
) const {

    if (coreId >= caches.size()) {
        throw std::out_of_range(
            "Invalid CPU core ID"
        );
    }
}
int CoherenceManager::readMemory() const {
    return memory.read();
}
const Cache& CoherenceManager::getCache(
    std::size_t coreId
) const {

    validateCoreId(coreId);

    return caches[coreId];
}
int CoherenceManager::read(std::size_t coreId) {

    validateCoreId(coreId);

    CacheLine& requesterLine =
        caches[coreId].getLine();
   MESIState stateBefore = requesterLine.state;

    // Case 1: Cache already has a valid copy.
    if (requesterLine.state != MESIState::Invalid) {
        return requesterLine.data;
    }

    // Record the BusRd request.
 // Case 2: Cache does not have the data.
    bool sharedCopyExists = false;

    int latestData = memory.read();
	std::vector<CacheStateChange> affectedCores;

    // Check other CPU caches.
    for (std::size_t i = 0; i < caches.size(); i++) {

        if (i == coreId) {
            continue;
        }

        CacheLine& otherLine = caches[i].getLine();

        if (otherLine.state == MESIState::Modified) {

            latestData = otherLine.data;

            // Write back dirty data to memory.
            memory.write(latestData);

            MESIState otherStateBefore = otherLine.state;

otherLine.state = MESIState::Shared;

affectedCores.push_back({
    i,
    otherStateBefore,
    otherLine.state
});

            sharedCopyExists = true;
            break;
        }

        if (otherLine.state == MESIState::Exclusive) {

            latestData = otherLine.data;

            MESIState otherStateBefore = otherLine.state;

otherLine.state = MESIState::Shared;

affectedCores.push_back({
    i,
    otherStateBefore,
    otherLine.state
});

            sharedCopyExists = true;
            break;
        }

        if (otherLine.state == MESIState::Shared) {

            latestData = otherLine.data;

            sharedCopyExists = true;
        }
    }

    // Give the requested data to this cache.
    requesterLine.data = latestData;

    if (sharedCopyExists) {
        requesterLine.state = MESIState::Shared;
    }
    else {
       requesterLine.state = MESIState::Exclusive;
    }
	affectedCores.push_back({
    coreId,
    stateBefore,
    requesterLine.state
});
    // Record the completed BusRd transaction.
transactionHistory.push_back({
    nextSequenceNumber++,
    coreId,
    CoherenceAction::BusRd,
    stateBefore,
    requesterLine.state,
    {},
    affectedCores
});

    return requesterLine.data;
}

void CoherenceManager::write(
    std::size_t coreId,
    int value
) {
    validateCoreId(coreId);

    CacheLine& requesterLine =
        caches[coreId].getLine();

    MESIState stateBefore = requesterLine.state;

    CoherenceAction action = CoherenceAction::None;

    std::vector<std::size_t> invalidatedCores;
    std::vector<CacheStateChange> affectedCores;

    // Determine the required coherence transaction.
    switch (requesterLine.state) {

        case MESIState::Invalid:
            action = CoherenceAction::BusRdX;
            break;

        case MESIState::Shared:
            action = CoherenceAction::BusUpgr;
            break;

        case MESIState::Exclusive:
        case MESIState::Modified:
            action = CoherenceAction::None;
            break;
    }

    // Invalidate other caches when ownership is requested.
    if (action == CoherenceAction::BusRdX ||
        action == CoherenceAction::BusUpgr) {

        for (std::size_t i = 0; i < caches.size(); i++) {

            if (i == coreId) {
                continue;
            }

            CacheLine& otherLine =
                caches[i].getLine();

            // Record only cores with valid copies.
            if (otherLine.state != MESIState::Invalid) {

                MESIState otherStateBefore =
                    otherLine.state;

                invalidatedCores.push_back(i);

                otherLine.state = MESIState::Invalid;

                affectedCores.push_back({
                    i,
                    otherStateBefore,
                    otherLine.state
                });
            }
        }
    }

    // Complete the write.
    requesterLine.data = value;
    requesterLine.state = MESIState::Modified;

    // Record the requester's state transition.
    affectedCores.push_back({
        coreId,
        stateBefore,
        requesterLine.state
    });

    // Record the completed transaction.
    if (action != CoherenceAction::None) {

        transactionHistory.push_back({
            nextSequenceNumber++,
            coreId,
            action,
            stateBefore,
            requesterLine.state,
            invalidatedCores,
            affectedCores
        });
    }
}

const std::vector<BusTransaction>&
CoherenceManager::getTransactionHistory() const {
    return transactionHistory;
}
