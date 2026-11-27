
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

    memory.write(0, initialMemoryValue);
}

void CoherenceManager::validateCoreId(
    std::size_t coreId
) const {
    if (coreId >= caches.size()) {
        throw std::out_of_range("Invalid CPU core ID");
    }
}

int CoherenceManager::readMemory() const {
    return readMemory(0);
}

int CoherenceManager::readMemory(int address) const {
    return memory.read(address);
}

const Cache& CoherenceManager::getCache(
    std::size_t coreId
) const {
    validateCoreId(coreId);
    return caches[coreId];
}

// Backward-compatible read.
int CoherenceManager::read(std::size_t coreId) {
    return read(coreId, 0);
}

// Address-based read.
int CoherenceManager::read(
    std::size_t coreId,
    int address
) {
    validateCoreId(coreId);

    CacheLine& requesterLine =
        caches[coreId].getLine(address);

    MESIState stateBefore = requesterLine.state;
    int dataBefore = requesterLine.data;

    // Cache hit.
    if (requesterLine.isValid()) {
        return requesterLine.data;
    }

    bool sharedCopyExists = false;
    int latestData = memory.read(address);

    std::vector<CacheStateChange> affectedCores;

    // Check other cores for the requested address.
    for (std::size_t i = 0; i < caches.size(); i++) {
        if (i == coreId) {
            continue;
        }

        const CacheLine* otherLinePtr =
            caches[i].findLine(address);

        if (otherLinePtr == nullptr ||
            !otherLinePtr->isValid()) {
            continue;
        }

        CacheLine& otherLine =
            caches[i].getLine(address);

        if (otherLine.state == MESIState::Modified) {
            latestData = otherLine.data;

            // Write back modified data.
            memory.write(address, latestData);

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

    requesterLine.data = latestData;

    requesterLine.state = sharedCopyExists
        ? MESIState::Shared
        : MESIState::Exclusive;

    affectedCores.push_back({
        coreId,
        stateBefore,
        requesterLine.state
    });

    transactionHistory.push_back({
        nextSequenceNumber++,
        coreId,
        address,
        CoherenceAction::BusRd,
        stateBefore,
        requesterLine.state,
        dataBefore,
        requesterLine.data,
        {},
        affectedCores
    });

    return requesterLine.data;
}

// Backward-compatible write.
void CoherenceManager::write(
    std::size_t coreId,
    int value
) {
    write(coreId, 0, value);
}

// Address-based write.
void CoherenceManager::write(
    std::size_t coreId,
    int address,
    int value
) {
    validateCoreId(coreId);

    CacheLine& requesterLine =
        caches[coreId].getLine(address);

    MESIState stateBefore = requesterLine.state;
    int dataBefore = requesterLine.data;

    CoherenceAction action = CoherenceAction::None;

    std::vector<std::size_t> invalidatedCores;
    std::vector<CacheStateChange> affectedCores;

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

    // Invalidate copies of this address only.
    if (action == CoherenceAction::BusRdX ||
        action == CoherenceAction::BusUpgr) {

        for (std::size_t i = 0; i < caches.size(); i++) {
            if (i == coreId) {
                continue;
            }

            const CacheLine* otherLinePtr =
                caches[i].findLine(address);

            if (otherLinePtr == nullptr ||
                !otherLinePtr->isValid()) {
                continue;
            }

            CacheLine& otherLine =
                caches[i].getLine(address);

            MESIState otherStateBefore = otherLine.state;

            invalidatedCores.push_back(i);
            otherLine.state = MESIState::Invalid;

            affectedCores.push_back({
                i,
                otherStateBefore,
                otherLine.state
            });
        }
    }

    requesterLine.data = value;
    requesterLine.state = MESIState::Modified;

    affectedCores.push_back({
        coreId,
        stateBefore,
        requesterLine.state
    });

    if (action != CoherenceAction::None) {
        transactionHistory.push_back({
            nextSequenceNumber++,
            coreId,
            address,
            action,
            stateBefore,
            requesterLine.state,
            dataBefore,
            requesterLine.data,
            invalidatedCores,
            affectedCores
        });
    }
}

const std::vector<BusTransaction>&
CoherenceManager::getTransactionHistory() const {
    return transactionHistory;
}
