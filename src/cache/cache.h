
#ifndef CACHE_H
#define CACHE_H

#include "cache_line.h"

class Cache {
private:
    CacheLine line;

public:
    CacheLine& getLine() {
    return line;
}

const CacheLine& getLine() const {
    return line;
}
};

#endif
