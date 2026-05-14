#ifndef HUFFMANCODE_H
#define HUFFMANCODE_H

#include <cstdint>
#include <stdexcept>

class HuffmanCode {
private:
    char bits[256];
    uint16_t len = 0;

public:
    explicit HuffmanCode() = default;
    explicit HuffmanCode(char bit) {
        push(bit);
    }

    void push(char bit) {
        bits[len++] = bit;
    }

    bool empty() const { return len == 0; }
    uint16_t size() const { return len; }

    const char* begin() const { return bits; }
    const char* end() const { return bits + len; }

    HuffmanCode operator+(char bit) const {
        HuffmanCode copy = *this;
        copy.push(bit);
        return copy;
    }
};

#endif // HUFFMANCODE_H
