#include <gtest/gtest.h>
#include "../../src/simulator/main_memory.h"

TEST(MainMemoryTest, InitialValueIsZero) {
    MainMemory memory;

    EXPECT_EQ(memory.read(), 0);
}

TEST(MainMemoryTest, WriteAndReadValue) {
    MainMemory memory;

    memory.write(100);

    EXPECT_EQ(memory.read(), 100);
}

TEST(MainMemoryTest, UpdatedValueIsReturned) {
    MainMemory memory;

    memory.write(100);
    memory.write(200);

    EXPECT_EQ(memory.read(), 200);
}
