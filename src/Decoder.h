#ifndef DECODER_H
#define DECODER_H

#include <istream>
#include <cstdint>

#include "HuffmanTree.h"
#include "BitOperators.h"

void readHeader(std::istream& in, int freq[256], int& lastBits);
std::pair<std::streampos, uint16_t> extractHeader(std::istream& in, int freq[256], int& lastBits);
void decodeFile(std::istream& in, std::ostream& out);

#endif // DECODER_H
