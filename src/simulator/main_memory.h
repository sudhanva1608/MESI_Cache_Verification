#ifndef MAIN_MEMORY_H
#define MAIN_MEMORY_H

class MainMemory {
private:
    int data;

public:
    MainMemory() {
        data = 0;
    }

    int read() const {
        return data;
    }

    void write(int newData) {
        data = newData;
    }
};

#endif
