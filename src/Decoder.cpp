#include "Decoder.h"
#include "HuffmanTree.h"

void readHeader(std::istream& in, int freq[256], int& lastBits) {
    uint16_t count;
    in.read(reinterpret_cast<char*>(&count), sizeof(count));

    for (int i = 0; i < count; ++i) {
        uint8_t sym;
        uint32_t symbolFreq;
        in.read(reinterpret_cast<char*>(&sym), 1);
        in.read(reinterpret_cast<char*>(&symbolFreq), sizeof(symbolFreq));
        freq[sym] = static_cast<int>(symbolFreq);
    }

    uint8_t storedLastBits;
    in.read(reinterpret_cast<char*>(&storedLastBits), sizeof(storedLastBits));
    lastBits = storedLastBits;
}

const std::streampos extractHeader(std::istream& in, int freq[256], int& lastBits) {
    in.seekg(0, std::ios::end);
    const std::streampos fileEnd = in.tellg();

    uint16_t headerSize;
    in.seekg(fileEnd - static_cast<std::streamoff>(sizeof(headerSize)));
    in.read(reinterpret_cast<char*>(&headerSize), sizeof(headerSize));

    const std::streampos headerStart =
        fileEnd
        - static_cast<std::streamoff>(sizeof(headerSize))
        - static_cast<std::streamoff>(headerSize);

    in.seekg(headerStart);
    readHeader(in, freq, lastBits);

    return headerStart;
}

void decodeFile(std::istream& in, std::ostream& out) {
    int freq[256] {};
    int lastBits;

    const std::streampos headerStart = extractHeader(in, freq, lastBits);

    int symbolCount = 0;
    int onlySymbol = 0;
    int originalSize = 0;
    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) {
            ++symbolCount;
            onlySymbol = i;
            originalSize += freq[i];
        }
    }

    if (symbolCount == 0) {
        return;
    }

    if (symbolCount == 1) {
        for (int i = 0; i < originalSize; ++i) {
            out.put(static_cast<char>(onlySymbol));
        }
        return;
    }

    HuffmanTree tree(freq);

    const uint64_t totalBytes = static_cast<uint64_t>(headerStart);
    in.seekg(0);

    const uint64_t totalBits = (totalBytes - 1) * 8 + static_cast<uint64_t>(lastBits);

    BitReader br(in);

    HuffmanTree::Cursor cursor = tree.cursor();

    for (uint64_t i = 0; i < totalBits; ++i) {
        int bit = br.readBit();

        cursor.move(bit);

        if (cursor.isLeaf()) {
            out.put(static_cast<char>(cursor.symbol()));
            cursor.reset();
        }
    }
}
