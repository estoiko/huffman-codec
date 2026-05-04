#include "HuffmanTree.h"

std::ostream& operator<<(std::ostream& out, const HuffmanTree::Node& node) {
    out << "( [" << node.symbol << "] : " << node.freq << " )";
    return out;
}
