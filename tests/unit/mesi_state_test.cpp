
#include <gtest/gtest.h>
#include "../../src/coherence/mesi_state.h"

TEST(MESIStateTest, ModifiedStateToString) {
    EXPECT_STREQ(stateToString(MESIState::Modified), "Modified");
}

TEST(MESIStateTest, ExclusiveStateToString) {
    EXPECT_STREQ(stateToString(MESIState::Exclusive), "Exclusive");
}

TEST(MESIStateTest, SharedStateToString) {
    EXPECT_STREQ(stateToString(MESIState::Shared), "Shared");
}

TEST(MESIStateTest, InvalidStateToString) {
    EXPECT_STREQ(stateToString(MESIState::Invalid), "Invalid");
}

TEST(MESITransitionTest, ModifiedRead) {
    auto result = getTransition(
        MESIState::Modified, Operation::Read
    );

    EXPECT_EQ(result.nextState, MESIState::Modified);
    EXPECT_EQ(result.action, CoherenceAction::None);
}

TEST(MESITransitionTest, ExclusiveWrite) {
    auto result = getTransition(
        MESIState::Exclusive, Operation::Write
    );

    EXPECT_EQ(result.nextState, MESIState::Modified);
    EXPECT_EQ(result.action, CoherenceAction::None);
}

TEST(MESITransitionTest, SharedRead) {
    auto result = getTransition(
        MESIState::Shared, Operation::Read
    );

    EXPECT_EQ(result.nextState, MESIState::Shared);
    EXPECT_EQ(result.action, CoherenceAction::None);
}

TEST(MESITransitionTest, SharedWrite) {
    auto result = getTransition(
        MESIState::Shared, Operation::Write
    );

    EXPECT_EQ(result.nextState, MESIState::Modified);
    EXPECT_EQ(result.action, CoherenceAction::BusUpgr);
}

TEST(MESITransitionTest, InvalidRead) {
    auto result = getTransition(
        MESIState::Invalid, Operation::Read
    );

    EXPECT_EQ(result.nextState, MESIState::Shared);
    EXPECT_EQ(result.action, CoherenceAction::BusRd);
}

TEST(MESITransitionTest, InvalidWrite) {
    auto result = getTransition(
        MESIState::Invalid, Operation::Write
    );

    EXPECT_EQ(result.nextState, MESIState::Modified);
    EXPECT_EQ(result.action, CoherenceAction::BusRdX);
}

