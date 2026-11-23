
#ifndef MESI_STATE_H
#define MESI_STATE_H

enum class MESIState {
    Modified,
    Exclusive,
    Shared,
    Invalid
};

enum class Operation {
    Read,
    Write
};
enum class CoherenceAction {
    None,
    BusRd,
    BusRdX,
    BusUpgr
};
struct TransitionResult {
    MESIState nextState;
    CoherenceAction action;
};
const char* stateToString(MESIState state);
TransitionResult getTransition(MESIState currentState, Operation operation);
#endif
