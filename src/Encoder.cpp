#include "Encoder.h"
#include "Exceptions.h"
#include "HuffmanCode.h"
#include <cstdint>

void countFreq(std::istream& in, int freq[256]) {
    char c;
    while (in.get(c)) {
        ++freq[static_cast<unsigned char>(c)];
    }
}

void writeHeader(std::ostream& out, int freq[256], uint16_t count, int lastBits) {
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));

    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) {
            uint8_t sym = static_cast<uint8_t>(i);
            uint32_t symbolFreq = static_cast<uint32_t>(freq[i]);
            out.write(reinterpret_cast<const char*>(&sym), 1);
            out.write(reinterpret_cast<const char*>(&symbolFreq), sizeof(symbolFreq));
        }
    }

    uint8_t storedLastBits = static_cast<uint8_t>(lastBits);
    out.write(reinterpret_cast<const char*>(&storedLastBits), sizeof(storedLastBits));
}

std::streampos emplaceHeader(std::ostream& out, int freq[256], uint16_t count, int lastBits) {
    const std::streampos headerStart = out.tellp();
    writeHeader(out, freq, count, lastBits);
    const std::streampos headerEnd = out.tellp();

    const uint16_t headerSize = static_cast<uint16_t>(headerEnd - headerStart);
    out.write(reinterpret_cast<const char*>(&headerSize), sizeof(headerSize));

    return headerStart;
}

void encodeFile(std::istream& in, std::ostream& out, bool force) {
    int freq[256] {};
    countFreq(in, freq);

    HuffmanTree tree(freq);

    HuffmanCode codes[256];
    tree.generateCodes(codes);

    uint64_t originalSize = 0;
    uint16_t uniqueCount = 0;
    uint64_t compressedSize = 0;
    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) {
            originalSize += static_cast<uint64_t>(freq[i]);
            ++uniqueCount;

            compressedSize += static_cast<uint64_t>(freq[i]) * codes[i].size();
        }
    }

    compressedSize = (compressedSize + 7) / 8;
    const uint64_t headerSize =
        sizeof(uint16_t)
        + static_cast<uint64_t>(uniqueCount) * (sizeof(uint8_t) + sizeof(uint32_t))
        + sizeof(uint8_t);
    const uint64_t archiveSize = compressedSize + headerSize + sizeof(uint16_t);

    if (archiveSize >= originalSize && !force) {
        throw CompressionIneffective();
    }

    in.clear();
    in.seekg(0);

    BitWriter bw(out);

    char c;
    while (in.get(c)) {
        for (unsigned char bit : codes[static_cast<unsigned char>(c)]) {
            bw.writeBit(bit - '0');
        }
    }

    int lastBits = bw.flush();

    emplaceHeader(out, freq, uniqueCount, lastBits);
}
