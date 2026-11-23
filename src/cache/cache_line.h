
#ifndef CACHE_LINE_H
#define CACHE_LINE_H

#include "../coherence/mesi_state.h"

class CacheLine {
public:

    MESIState state;
    int data;

    CacheLine() {
        state = MESIState::Invalid;
        data = 0;
    }

    bool isValid() const {
        return state != MESIState::Invalid;
    }
};

#endif
