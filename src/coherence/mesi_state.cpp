
#include "mesi_state.h"

const char* stateToString(MESIState state) {
    switch (state) {
        case MESIState::Modified:
            return "Modified";

        case MESIState::Exclusive:
            return "Exclusive";

        case MESIState::Shared:
            return "Shared";

        case MESIState::Invalid:
            return "Invalid";
    }

    return "Unknown";
}

TransitionResult getTransition(MESIState currentState, Operation operation) {

    switch (currentState) {

        case MESIState::Modified:
            return {MESIState::Modified, CoherenceAction::None};

        case MESIState::Exclusive:
            if (operation == Operation::Read) {
                return {MESIState::Exclusive, CoherenceAction::None};
            }
            return {MESIState::Modified, CoherenceAction::None};

        case MESIState::Shared:
            if (operation == Operation::Read) {
                return {MESIState::Shared, CoherenceAction::None};
            }
            return {MESIState::Modified, CoherenceAction::BusUpgr};

        case MESIState::Invalid:
            if (operation == Operation::Read) {
                return {MESIState::Shared, CoherenceAction::BusRd};
            }
            return {MESIState::Modified, CoherenceAction::BusRdX};
    }

    return {MESIState::Invalid, CoherenceAction::None};
}

