#ifndef ENCODER_H
#define ENCODER_H

#include <istream>
#include <ostream>

#include "HuffmanTree.h"
#include "BitOperators.h"

void countFreq(std::istream& in, int freq[256]);
void writeHeader(std::ostream& out, int freq[256], int lastBits);
const std::streampos emplaceHeader(std::ostream& out, int freq[256], int lastBits);
void encodeFile(std::istream& in, std::ostream& out);
void carefulEncodeFile(std::istream& in, std::ostream& out);

#endif // ENCODER_H
