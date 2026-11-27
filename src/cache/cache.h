#ifndef CACHE_H
#define CACHE_H

#include <map>

#include "cache_line.h"

class Cache {
private:
    CacheLine line;
    std::map<int, CacheLine> lines;

public:
    CacheLine& getLine() {
        return line;
    }

    const CacheLine& getLine() const {
        return line;
    }

    CacheLine& getLine(int address) {
        if (address == 0) {
            line.address = 0;
            return line;
        }

        CacheLine& cacheLine = lines[address];
        cacheLine.address = address;
        return cacheLine;
    }

    const CacheLine* findLine(int address) const {
        if (address == 0) {
            return &line;
        }

        auto it = lines.find(address);

        if (it == lines.end()) {
            return nullptr;
        }

        return &it->second;
    }
};

#endif
