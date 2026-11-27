#include <gtest/gtest.h>
#include "../../src/coherence/coherence_manager.h"

TEST(CoherenceManagerTest, InitializesMainMemory) {
    CoherenceManager manager(2, 100);

    EXPECT_EQ(manager.readMemory(), 100);
}

TEST(CoherenceManagerTest, CreatesMultipleCaches) {
    CoherenceManager manager(4, 100);

    for (std::size_t i = 0; i < 4; i++) {
        EXPECT_EQ(
            manager.getCache(i).getLine().state,
            MESIState::Invalid
        );
    }
}

TEST(CoherenceManagerTest, RejectsZeroCores) {
    EXPECT_THROW(
        CoherenceManager manager(0, 100),
        std::invalid_argument
    );
}

TEST(CoherenceManagerTest, RejectsInvalidCoreId) {
    CoherenceManager manager(2, 100);

    EXPECT_THROW(
        manager.getCache(2),
        std::out_of_range
    );
}
TEST(CoherenceManagerTest, ReadFromEmptyCache) {

    CoherenceManager manager(2, 100);

    int value = manager.read(0);

    EXPECT_EQ(value, 100);

    EXPECT_EQ(
        manager.getCache(0).getLine().state,
        MESIState::Exclusive
    );

    EXPECT_EQ(
        manager.getCache(0).getLine().data,
        100
    );

    EXPECT_EQ(manager.readMemory(), 100);
}
TEST(CoherenceManagerTest, TwoCoresReadSameData) {

    CoherenceManager manager(2, 100);

    int core0Value = manager.read(0);
    int core1Value = manager.read(1);

    EXPECT_EQ(core0Value, 100);
    EXPECT_EQ(core1Value, 100);

    EXPECT_EQ(
        manager.getCache(0).getLine().state,
        MESIState::Shared
    );

    EXPECT_EQ(
        manager.getCache(1).getLine().state,
        MESIState::Shared
    );
}
TEST(CoherenceManagerTest, WriteToEmptyCache) {

    CoherenceManager manager(2, 100);

    manager.write(0, 200);

    EXPECT_EQ(
        manager.getCache(0).getLine().state,
        MESIState::Modified
    );

    EXPECT_EQ(
        manager.getCache(0).getLine().data,
        200
    );

    EXPECT_EQ(
        manager.getCache(1).getLine().state,
        MESIState::Invalid
    );

    EXPECT_EQ(manager.readMemory(), 100);
}
TEST(CoherenceManagerTest, WriteInvalidatesOtherSharedCopies) {

    CoherenceManager manager(2, 100);

    // Both cores obtain a shared copy.
    manager.read(0);
    manager.read(1);

    // Core 0 writes a new value.
    manager.write(0, 200);

    EXPECT_EQ(
        manager.getCache(0).getLine().state,
        MESIState::Modified
    );

    EXPECT_EQ(
        manager.getCache(0).getLine().data,
        200
    );

    EXPECT_EQ(
        manager.getCache(1).getLine().state,
        MESIState::Invalid
    );

    EXPECT_EQ(manager.readMemory(), 100);
}
TEST(CoherenceManagerTest, ReadGetsLatestModifiedValue) {

    CoherenceManager manager(2, 100);

    manager.read(0);
    manager.read(1);

    manager.write(0, 200);

    int value = manager.read(1);

    EXPECT_EQ(value, 200);

    EXPECT_EQ(
        manager.getCache(0).getLine().state,
        MESIState::Shared
    );

    EXPECT_EQ(
        manager.getCache(1).getLine().state,
        MESIState::Shared
    );

    EXPECT_EQ(
        manager.getCache(0).getLine().data,
        200
    );

    EXPECT_EQ(
        manager.getCache(1).getLine().data,
        200
    );

    EXPECT_EQ(manager.readMemory(), 200);
}
TEST(CoherenceManagerTest, IndependentMemoryAddresses) {

    CoherenceManager manager(2);

    manager.write(0, 100, 500);
    manager.write(0, 200, 800);

    EXPECT_EQ(manager.read(0, 100), 500);
    EXPECT_EQ(manager.read(0, 200), 800);

    EXPECT_EQ(manager.read(1, 100), 500);
    EXPECT_EQ(manager.read(1, 200), 800);
}
TEST(CoherenceManagerTest, AddressSpecificInvalidation) {

    CoherenceManager manager(2);

    manager.read(0, 100);
    manager.read(1, 100);

    manager.read(0, 200);
    manager.read(1, 200);

    manager.write(0, 100, 500);

    EXPECT_EQ(
        manager.getCache(0).findLine(100)->state,
        MESIState::Modified
    );

    EXPECT_EQ(
        manager.getCache(1).findLine(100)->state,
        MESIState::Invalid
    );

    EXPECT_EQ(
        manager.getCache(0).findLine(200)->state,
        MESIState::Shared
    );

    EXPECT_EQ(
        manager.getCache(1).findLine(200)->state,
        MESIState::Shared
    );
}
TEST(CoherenceManagerTest, AddressSpecificWriteBack) {

    CoherenceManager manager(2);

    manager.write(0, 100, 500);
    manager.write(0, 200, 800);

    EXPECT_EQ(manager.readMemory(100), 0);
    EXPECT_EQ(manager.readMemory(200), 0);

    EXPECT_EQ(manager.read(1, 100), 500);

    EXPECT_EQ(manager.readMemory(100), 500);
    EXPECT_EQ(manager.readMemory(200), 0);

    EXPECT_EQ(manager.read(1, 200), 800);

    EXPECT_EQ(manager.readMemory(200), 800);
}
TEST(CoherenceManagerTest, BackwardCompatibleDefaultAddress) {

    CoherenceManager manager(2, 100);

    EXPECT_EQ(manager.read(0), 100);
    EXPECT_EQ(manager.read(0, 0), 100);

    manager.write(0, 500);

    EXPECT_EQ(manager.read(1, 0), 500);
    EXPECT_EQ(manager.readMemory(), 500);
    EXPECT_EQ(manager.readMemory(0), 500);

    manager.write(1, 0, 800);

    EXPECT_EQ(manager.read(0), 800);
    EXPECT_EQ(manager.readMemory(), 800);
}
