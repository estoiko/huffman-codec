#ifndef BITOPERATOR_H
#define BITOPERATOR_H

#include <cstdint>
#include <ostream>
#include <istream>

class BitWriter {
private:
    std::ostream& out;
    uint8_t buf = 0;
    int count = 0;

public:
    BitWriter(std::ostream& out) : out(out) {}
    void writeBit(int bit);
    int flush();
};

class BitReader {
private:
    std::istream& in;
    uint8_t buf = 0;
    int count = 0;

public:
    BitReader(std::istream& in) : in(in) {}
    int readBit();
};

#endif // BITOPERATOR_H
