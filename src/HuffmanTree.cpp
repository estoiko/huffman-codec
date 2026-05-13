#include "HuffmanTree.h"
#include "HuffmanCode.h"
#include "Queue.h"

struct HuffmanTree::Node {
    uint32_t symbol;
    int freq;

    Node* left;
    Node* right;

    bool isLeaf() const {
        return !left && !right;
    }

    bool operator<(const Node& other) const {
        return freq < other.freq;
    }
};

struct HuffmanTree::NodePtrLess {
    bool operator()(const Node* left, const Node* right) const {
        return left->freq < right->freq;
    }
};

void HuffmanTree::generateCodes(Node* node, HuffmanCode acc, HuffmanCode codes[256]) const {
    if (!node) return;
    if (node->isLeaf()) {
        codes[node->symbol] = acc.empty() ? (HuffmanCode('0')) : acc;
        return;
    }

    generateCodes(node->left, acc + '0', codes);
    generateCodes(node->right, acc + '1', codes);
}

void HuffmanTree::generateCodes(HuffmanCode codes[256]) const {
    generateCodes(root_, HuffmanCode{}, codes);
}

void HuffmanTree::deleteNode(Node* node) {
    if (!node) return;
    deleteNode(node->left);
    deleteNode(node->right);
    delete node;
}

HuffmanTree::~HuffmanTree() {
    deleteNode(root_);
}

HuffmanTree::HuffmanTree(const int freq[256])
    : root_(nullptr)
{
    Queue<Node*> q1, q2;
    NodePtrLess comp;

    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) {
            q1.push(new Node { static_cast<uint32_t>(i), freq[i], nullptr, nullptr });
        }
    }

    if (q1.isEmpty()) {
        throw EmptyInputFile();
    }

    q1.sort(comp);

    while (q1.size() + q2.size() > 1) {
        Node* left = popMin(q1, q2, comp);
        Node* right = popMin(q1, q2, comp);

        Node* parent = new Node { 0, left->freq + right->freq, left, right };
        q2.push(parent);
    }

    root_ = q1.isEmpty() ? q2.pop() : q1.pop();
}

HuffmanTree::Cursor::Cursor(const HuffmanTree& tree)
    : tree_(tree)
    , current_(tree.root_)
{}

void HuffmanTree::Cursor::reset() {
    current_ = tree_.root_;
}

void HuffmanTree::Cursor::move(int bit) {
    current_ = (bit == 0) ? current_->left : current_->right;
}

bool HuffmanTree::Cursor::isLeaf() const {
    return current_->isLeaf();
}

bool HuffmanTree::Cursor::isRoot() const {
    return current_ == tree_.root_;
}

unsigned char HuffmanTree::Cursor::symbol() const {
    return static_cast<unsigned char>(current_->symbol);
}

HuffmanTree::Cursor HuffmanTree::cursor() const {
    return Cursor(*this);
}

std::ostream& operator<<(std::ostream& out, const HuffmanTree::Node& node) {
    out << "( [" << node.symbol << "] : " << node.freq << " )";
    return out;
}
