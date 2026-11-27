#ifndef CACHE_LINE_H
#define CACHE_LINE_H

#include "../coherence/mesi_state.h"

class CacheLine {
public:
    int address;
    MESIState state;
    int data;

    CacheLine() {
        address = -1;  // -1 means no address assigned
        state = MESIState::Invalid;
        data = 0;
    }

    bool isValid() const {
        return state != MESIState::Invalid;
    }
};

#endif
