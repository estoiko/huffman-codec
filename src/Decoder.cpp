#include "Decoder.h"
#include "Exceptions.h"
#include "HuffmanTree.h"
#include <cstdint>

struct HeaderInfo {
    std::streampos headerStart;
    uint16_t headerSize;
};

void readHeader(std::istream& in, int freq[256], int& lastBits) {
    uint16_t count;
    in.read(reinterpret_cast<char*>(&count), sizeof(count));

    if (count > 256 || count == 0) {
        throw InvalidArchive();
    }

    for (int i = 0; i < count; ++i) {
        uint8_t sym;
        uint32_t symbolFreq;
        in.read(reinterpret_cast<char*>(&sym), 1);
        in.read(reinterpret_cast<char*>(&symbolFreq), sizeof(symbolFreq));
        freq[sym] = static_cast<int>(symbolFreq);

        if (freq[sym] == 0) {
            throw InvalidArchive();
        }
    }

    uint8_t storedLastBits;
    in.read(reinterpret_cast<char*>(&storedLastBits), sizeof(storedLastBits));
    lastBits = storedLastBits;

    if (lastBits < 1 || lastBits > 8) {
        throw InvalidArchive();
    }
}

HeaderInfo extractHeader(std::istream& in, int freq[256], int& lastBits) {
    in.seekg(0, std::ios::end);
    const std::streampos fileEnd = in.tellg();

    if (fileEnd < static_cast<std::streamoff>(sizeof(uint16_t))) {
        throw InvalidArchive();
    }

    uint16_t headerSize;
    in.seekg(fileEnd - static_cast<std::streamoff>(sizeof(headerSize)));
    in.read(reinterpret_cast<char*>(&headerSize), sizeof(headerSize));

    const std::streampos headerStart =
        fileEnd
        - static_cast<std::streamoff>(sizeof(headerSize))
        - static_cast<std::streamoff>(headerSize);

    if (static_cast<std::streamoff>(headerSize)
        + static_cast<std::streamoff>(sizeof(headerSize))
        >= static_cast<std::streamoff>(fileEnd)) {
        throw InvalidArchive();
    }

    const auto encodedFileSize = static_cast<std::streamoff>(fileEnd);
    const auto headerTotalSize = static_cast<std::streamoff>(headerSize)
                           + static_cast<std::streamoff>(sizeof(headerSize));

    if (headerTotalSize >= encodedFileSize) {
        throw InvalidArchive();
    }

    in.seekg(headerStart);
    readHeader(in, freq, lastBits);

    return HeaderInfo{ headerStart, headerSize };
}

void decodeFile(std::istream& in, std::ostream& out) {
    int freq[256] {};
    int lastBits;

    auto [headerStart, headerSize] = extractHeader(in, freq, lastBits);

    const uint64_t totalBytes = static_cast<uint64_t>(headerStart);
    const uint64_t totalBits = (totalBytes - 1) * 8 + static_cast<uint64_t>(lastBits);

    if (totalBytes == 0) {
        throw InvalidArchive();
    }

    int symbolCount = 0;
    int onlySymbol = 0;
    uint64_t originalSize = 0;
    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) {
            ++symbolCount;
            onlySymbol = i;
            originalSize += freq[i];
        }
    }

    uint16_t expected = sizeof(uint16_t)
                      + symbolCount * (1 + sizeof(uint32_t))
                      + sizeof(uint8_t);

    if (symbolCount > 256 || headerSize != expected) {
        throw InvalidArchive();
    }

    if (symbolCount == 1) {
        if (totalBits != originalSize) {
            throw InvalidArchive();
        }

        in.clear();
        in.seekg(0);

        if (!in) {
            throw InvalidArchive();
        }

        BitReader br(in);

        for (uint64_t i = 0; i < totalBits; ++i) {
            int bit = br.readBit();

            if (bit != 0) {
                throw InvalidArchive();
            }
        }

        for (uint64_t i = 0; i < originalSize; ++i) {
            out.put(static_cast<char>(onlySymbol));
        }

        return;
    }

    HuffmanTree tree(freq);

    in.seekg(0);

    BitReader br(in);

    HuffmanTree::Cursor cursor = tree.cursor();

    uint64_t decodedSymCount = 0;
    for (uint64_t i = 0; i < totalBits; ++i) {
        int bit = br.readBit();
        if (bit == -1) throw InvalidArchive();

        cursor.move(bit);

        if (cursor.isLeaf()) {
            out.put(static_cast<char>(cursor.symbol()));
            cursor.reset();
            ++decodedSymCount;
        }
    }

    if (decodedSymCount != static_cast<uint64_t>(originalSize) || !cursor.isRoot()) {
        throw InvalidArchive();
    }
}
