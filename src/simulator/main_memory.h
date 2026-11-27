#ifndef MAIN_MEMORY_H
#define MAIN_MEMORY_H

#include <map>

class MainMemory {
private:
    int data;
    std::map<int, int> memory;

public:
    MainMemory() {
        data = 0;
    }

    // Existing single-value interface
    int read() const {
        return data;
    }

    void write(int newData) {
        data = newData;
    }

    // New address-based interface
    int read(int address) const {
        auto it = memory.find(address);

        if (it == memory.end()) {
            return 0;
        }

        return it->second;
    }

    void write(int address, int newData) {
        memory[address] = newData;
    }
};

#endif
