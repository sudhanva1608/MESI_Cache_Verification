#include <gtest/gtest.h>

#include "../../src/coherence/coherence_manager.h"

TEST(TransactionHistoryTest, RecordsBusRdOnCacheMiss) {
    CoherenceManager manager(2, 100);

    manager.read(0);

    const auto& history = manager.getTransactionHistory();

    ASSERT_EQ(history.size(), 1);
    EXPECT_EQ(history[0].requesterCore, 0);
    EXPECT_EQ(history[0].action, CoherenceAction::BusRd);
}
TEST(TransactionHistoryTest, RecordsBusRdXOnWriteMiss) {
    CoherenceManager manager(2, 100);

    manager.write(0, 200);

    const auto& history = manager.getTransactionHistory();

    ASSERT_EQ(history.size(), 1);
    EXPECT_EQ(history[0].requesterCore, 0);
    EXPECT_EQ(history[0].action, CoherenceAction::BusRdX);
}
TEST(TransactionHistoryTest, RecordsBusUpgrOnSharedWrite) {
    CoherenceManager manager(2, 100);

    manager.read(0);
    manager.read(1);
    manager.write(0, 200);

    const auto& history = manager.getTransactionHistory();

    ASSERT_EQ(history.size(), 3);

    EXPECT_EQ(history[0].action, CoherenceAction::BusRd);
    EXPECT_EQ(history[1].action, CoherenceAction::BusRd);
    EXPECT_EQ(history[2].requesterCore, 0);
    EXPECT_EQ(history[2].action, CoherenceAction::BusUpgr);
}
TEST(TransactionHistoryTest, DoesNotRecordBusTransactionOnCacheHit) {
    CoherenceManager manager(2, 100);

    manager.read(0);  // First read: BusRd
    manager.read(0);  // Second read: cache hit

    const auto& history = manager.getTransactionHistory();

    ASSERT_EQ(history.size(), 1);
    EXPECT_EQ(history[0].action, CoherenceAction::BusRd);
}
TEST(TransactionHistoryTest, DoesNotRecordBusTransactionOnExclusiveWrite) {
    CoherenceManager manager(2, 100);

    manager.read(0);
    manager.write(0, 200);

    const auto& history = manager.getTransactionHistory();

    ASSERT_EQ(history.size(), 1);
    EXPECT_EQ(history[0].action, CoherenceAction::BusRd);

    EXPECT_EQ(
        manager.getCache(0).getLine().state,
        MESIState::Modified
    );
}
TEST(TransactionHistoryTest, DoesNotRecordBusTransactionOnModifiedWrite) {
    CoherenceManager manager(2, 100);

    manager.write(0, 200);  // BusRdX: obtain ownership
    manager.write(0, 300);  // Local write: already Modified

    const auto& history = manager.getTransactionHistory();

    ASSERT_EQ(history.size(), 1);
    EXPECT_EQ(history[0].action, CoherenceAction::BusRdX);

    EXPECT_EQ(
        manager.getCache(0).getLine().state,
        MESIState::Modified
    );

    EXPECT_EQ(manager.getCache(0).getLine().data, 300);
}
TEST(TransactionHistoryTest, DoesNotRecordBusTransactionOnModifiedRead) {
    CoherenceManager manager(2, 100);

    manager.write(0, 200);
    int value = manager.read(0);

    const auto& history = manager.getTransactionHistory();

    ASSERT_EQ(history.size(), 1);
    EXPECT_EQ(history[0].action, CoherenceAction::BusRdX);

    EXPECT_EQ(value, 200);
    EXPECT_EQ(
        manager.getCache(0).getLine().state,
        MESIState::Modified
    );
}
TEST(TransactionHistoryTest, AssignsSequentialNumbersToTransactions) {
    CoherenceManager manager(2, 100);

    manager.read(0);       // Transaction 1: BusRd
    manager.read(1);       // Transaction 2: BusRd
    manager.write(0, 200); // Transaction 3: BusUpgr

    const auto& history = manager.getTransactionHistory();

    ASSERT_EQ(history.size(), 3);

    EXPECT_EQ(history[0].sequenceNumber, 1);
    EXPECT_EQ(history[1].sequenceNumber, 2);
    EXPECT_EQ(history[2].sequenceNumber, 3);
}
TEST(TransactionHistoryTest, RecordsInvalidToExclusiveOnReadMiss) {
    CoherenceManager manager(2, 100);

    manager.read(0);

    const auto& history = manager.getTransactionHistory();

    ASSERT_EQ(history.size(), 1);

    EXPECT_EQ(history[0].stateBefore, MESIState::Invalid);
    EXPECT_EQ(history[0].stateAfter, MESIState::Exclusive);
}

TEST(TransactionHistoryTest, RecordsInvalidToModifiedOnWriteMiss) {
    CoherenceManager manager(2, 100);

    manager.write(0, 200);

    const auto& history = manager.getTransactionHistory();

    ASSERT_EQ(history.size(), 1);

    EXPECT_EQ(history[0].stateBefore, MESIState::Invalid);
    EXPECT_EQ(history[0].stateAfter, MESIState::Modified);
}

TEST(TransactionHistoryTest, RecordsSharedToModifiedOnUpgrade) {
    CoherenceManager manager(2, 100);

    manager.read(0);
    manager.read(1);
    manager.write(0, 200);

    const auto& history = manager.getTransactionHistory();

    ASSERT_EQ(history.size(), 3);

    EXPECT_EQ(history[2].stateBefore, MESIState::Shared);
    EXPECT_EQ(history[2].stateAfter, MESIState::Modified);
}
TEST(TransactionHistoryTest, RecordsOnlyCoresWithValidCopiesAsInvalidated) {
    CoherenceManager manager(3, 100);

    manager.read(0);       // Core 0: Exclusive
    manager.read(1);       // Core 0 and 1: Shared
    manager.write(0, 200); // Core 1 invalidated

    const auto& history = manager.getTransactionHistory();

    ASSERT_EQ(history.size(), 3);

    const auto& transaction = history[2];

    EXPECT_EQ(transaction.action, CoherenceAction::BusUpgr);

    ASSERT_EQ(transaction.invalidatedCores.size(), 1);

    EXPECT_EQ(transaction.invalidatedCores[0], 1);

    EXPECT_EQ(
        manager.getCache(1).getLine().state,
        MESIState::Invalid
    );

    EXPECT_EQ(
        manager.getCache(2).getLine().state,
        MESIState::Invalid
    );
}
TEST(TransactionHistoryTest, RecordsModifiedToSharedDowngrade) {
    CoherenceManager manager(2, 100);

    manager.write(0, 200);  // Core 0 becomes Modified.
    manager.read(1);        // Core 1 requests the data.

    const auto& history = manager.getTransactionHistory();

    ASSERT_EQ(history.size(), 2);

    const auto& transaction = history[1];

    EXPECT_EQ(transaction.action, CoherenceAction::BusRd);

    ASSERT_EQ(transaction.affectedCores.size(), 2);

    // Core 0: Modified -> Shared
    EXPECT_EQ(transaction.affectedCores[0].coreId, 0);
    EXPECT_EQ(transaction.affectedCores[0].stateBefore, MESIState::Modified);
    EXPECT_EQ(transaction.affectedCores[0].stateAfter, MESIState::Shared);

    // Core 1: Invalid -> Shared
    EXPECT_EQ(transaction.affectedCores[1].coreId, 1);
    EXPECT_EQ(transaction.affectedCores[1].stateBefore, MESIState::Invalid);
    EXPECT_EQ(transaction.affectedCores[1].stateAfter, MESIState::Shared);
}
TEST(TransactionHistoryTest, RecordsExclusiveToSharedDowngrade) {
    CoherenceManager manager(2, 100);

    manager.read(0);  // Core 0 becomes Exclusive.
    manager.read(1);  // Core 1 requests the same data.

    const auto& history = manager.getTransactionHistory();

    ASSERT_EQ(history.size(), 2);

    const auto& transaction = history[1];

    ASSERT_EQ(transaction.affectedCores.size(), 2);

    EXPECT_EQ(transaction.affectedCores[0].coreId, 0);
    EXPECT_EQ(transaction.affectedCores[0].stateBefore, MESIState::Exclusive);
    EXPECT_EQ(transaction.affectedCores[0].stateAfter, MESIState::Shared);

    EXPECT_EQ(transaction.affectedCores[1].coreId, 1);
    EXPECT_EQ(transaction.affectedCores[1].stateBefore, MESIState::Invalid);
    EXPECT_EQ(transaction.affectedCores[1].stateAfter, MESIState::Shared);
}

TEST(TransactionHistoryTest, RecordsAllAffectedCoresOnSharedWrite) {
    CoherenceManager manager(3, 100);

    // All three cores obtain a shared copy.
    manager.read(0);
    manager.read(1);
    manager.read(2);

    // Core 0 upgrades to Modified.
    manager.write(0, 200);

    const auto& history = manager.getTransactionHistory();

    ASSERT_EQ(history.size(), 4);

    const auto& transaction = history[3];

    EXPECT_EQ(transaction.action, CoherenceAction::BusUpgr);

    // Cores 1 and 2 must be invalidated.
    ASSERT_EQ(transaction.invalidatedCores.size(), 2);
    EXPECT_EQ(transaction.invalidatedCores[0], 1);
    EXPECT_EQ(transaction.invalidatedCores[1], 2);

    // All three cores must appear in affectedCores.
    ASSERT_EQ(transaction.affectedCores.size(), 3);

    // Core 1: Shared -> Invalid
    EXPECT_EQ(transaction.affectedCores[0].coreId, 1);
    EXPECT_EQ(transaction.affectedCores[0].stateBefore, MESIState::Shared);
    EXPECT_EQ(transaction.affectedCores[0].stateAfter, MESIState::Invalid);

    // Core 2: Shared -> Invalid
    EXPECT_EQ(transaction.affectedCores[1].coreId, 2);
    EXPECT_EQ(transaction.affectedCores[1].stateBefore, MESIState::Shared);
    EXPECT_EQ(transaction.affectedCores[1].stateAfter, MESIState::Invalid);

    // Core 0: Shared -> Modified
    EXPECT_EQ(transaction.affectedCores[2].coreId, 0);
    EXPECT_EQ(transaction.affectedCores[2].stateBefore, MESIState::Shared);
    EXPECT_EQ(transaction.affectedCores[2].stateAfter, MESIState::Modified);
}

TEST(TransactionHistoryTest, RecordsAllAffectedCoresOnBusRdX) {
    CoherenceManager manager(3, 100);

    manager.read(0);
    manager.read(1);

    manager.write(2, 300);

    const auto& history = manager.getTransactionHistory();

    ASSERT_EQ(history.size(), 3);

    const auto& transaction = history[2];

    EXPECT_EQ(transaction.action, CoherenceAction::BusRdX);

    ASSERT_EQ(transaction.invalidatedCores.size(), 2);
    EXPECT_EQ(transaction.invalidatedCores[0], 0);
    EXPECT_EQ(transaction.invalidatedCores[1], 1);

    ASSERT_EQ(transaction.affectedCores.size(), 3);

    // Core 0: Shared -> Invalid
    EXPECT_EQ(transaction.affectedCores[0].coreId, 0);
    EXPECT_EQ(transaction.affectedCores[0].stateBefore, MESIState::Shared);
    EXPECT_EQ(transaction.affectedCores[0].stateAfter, MESIState::Invalid);

    // Core 1: Shared -> Invalid
    EXPECT_EQ(transaction.affectedCores[1].coreId, 1);
    EXPECT_EQ(transaction.affectedCores[1].stateBefore, MESIState::Shared);
    EXPECT_EQ(transaction.affectedCores[1].stateAfter, MESIState::Invalid);

    // Core 2: Invalid -> Modified
    EXPECT_EQ(transaction.affectedCores[2].coreId, 2);
    EXPECT_EQ(transaction.affectedCores[2].stateBefore, MESIState::Invalid);
    EXPECT_EQ(transaction.affectedCores[2].stateAfter, MESIState::Modified);
}

TEST(TransactionHistoryTest, DoesNotRecordAlreadyInvalidCoresAsAffected) {
    CoherenceManager manager(4, 100);

    manager.read(0);
    manager.read(1);

    // Cores 2 and 3 remain Invalid.
    manager.write(0, 200);

    const auto& history = manager.getTransactionHistory();
    const auto& transaction = history[2];

    ASSERT_EQ(transaction.invalidatedCores.size(), 1);
    EXPECT_EQ(transaction.invalidatedCores[0], 1);

    ASSERT_EQ(transaction.affectedCores.size(), 2);

    EXPECT_EQ(transaction.affectedCores[0].coreId, 1);
    EXPECT_EQ(transaction.affectedCores[0].stateBefore, MESIState::Shared);
    EXPECT_EQ(transaction.affectedCores[0].stateAfter, MESIState::Invalid);

    EXPECT_EQ(transaction.affectedCores[1].coreId, 0);
    EXPECT_EQ(transaction.affectedCores[1].stateBefore, MESIState::Shared);
    EXPECT_EQ(transaction.affectedCores[1].stateAfter, MESIState::Modified);
}
TEST(TransactionHistoryTest, RecordsDataOnBusRd) {

    CoherenceManager manager(2, 100);

    manager.read(0);

    const auto& history =
        manager.getTransactionHistory();

    ASSERT_EQ(history.size(), 1);

    EXPECT_EQ(history[0].action, CoherenceAction::BusRd);
    EXPECT_EQ(history[0].dataBefore, 0);
    EXPECT_EQ(history[0].dataAfter, 100);
}
TEST(TransactionHistoryTest, RecordsDataOnBusRdX) {

    CoherenceManager manager(2, 100);

    manager.write(0, 200);

    const auto& history =
        manager.getTransactionHistory();

    ASSERT_EQ(history.size(), 1);

    EXPECT_EQ(history[0].action, CoherenceAction::BusRdX);
    EXPECT_EQ(history[0].dataBefore, 0);
    EXPECT_EQ(history[0].dataAfter, 200);
}
TEST(TransactionHistoryTest, RecordsDataOnBusUpgr) {

    CoherenceManager manager(2, 100);

    manager.read(0);
    manager.read(1);

    manager.write(0, 300);

    const auto& history =
        manager.getTransactionHistory();

    ASSERT_EQ(history.size(), 3);

    EXPECT_EQ(history[2].action, CoherenceAction::BusUpgr);
    EXPECT_EQ(history[2].dataBefore, 100);
    EXPECT_EQ(history[2].dataAfter, 300);
}
TEST(TransactionHistoryTest, RecordsCorrectMemoryAddress) {

    CoherenceManager manager(2);

    manager.read(0, 100);
    manager.write(1, 200, 500);
    manager.read(1, 100);
    manager.write(0, 100, 800);

    const auto& history =
        manager.getTransactionHistory();

    ASSERT_EQ(history.size(), 4u);

    EXPECT_EQ(history[0].address, 100);
    EXPECT_EQ(history[1].address, 200);
    EXPECT_EQ(history[2].address, 100);
    EXPECT_EQ(history[3].address, 100);

    EXPECT_EQ(history[0].action, CoherenceAction::BusRd);
    EXPECT_EQ(history[1].action, CoherenceAction::BusRdX);
    EXPECT_EQ(history[2].action, CoherenceAction::BusRd);
    EXPECT_EQ(history[3].action, CoherenceAction::BusUpgr);
}
