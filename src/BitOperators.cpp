#include "BitOperators.h"

void BitWriter::writeBit(int bit) {
    buf |= (bit << (7 - count));
    ++count;

    if (count == 8) {
        out.put(static_cast<char>(buf));
        buf = 0;
        count = 0;
    }
}

int BitWriter::flush() {
    int tailBits = count == 0 ? 8 : count;

    if (count > 0) {
        out.put(static_cast<char>(buf));
        buf = 0;
        count = 0;
    }

    return tailBits;
}

int BitReader::readBit() {
    if (count == 0) {
        char c;
        if (!in.get(c)) return -1;

        buf = static_cast<unsigned char>(c);
        count = 8;
    }

    int current_bit = (buf >> 7) & 1;
    buf <<= 1;

    --count;
    return current_bit;
}
