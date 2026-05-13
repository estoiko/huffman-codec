#ifndef HUFFMANTREE_H
#define HUFFMANTREE_H

#include <cstdint>
#include <ios>
#include <ostream>

#include "Queue.h"

class HuffmanTree {
private:
    struct Node {
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

    struct NodePtrLess {
        bool operator()(const Node* left, const Node* right) const {
            return left->freq < right->freq;
        }
    };

    void generateCodes(Node* node, std::string acc, std::string codes[256]) const {
        if (!node) return;
        if (node->isLeaf()) {
            codes[node->symbol] = acc.empty() ? "0" : acc;
            return;
        }
        generateCodes(node->left, acc + "0", codes);
        generateCodes(node->right, acc + "1", codes);
    }

    void deleteNode(Node* node) {
        if (!node) return;
        deleteNode(node->left);
        deleteNode(node->right);
        delete node;
    }

    Node* root_;

public:
    ~HuffmanTree() {
        deleteNode(root_);
    }

    HuffmanTree(const HuffmanTree&) = delete;
    HuffmanTree& operator=(const HuffmanTree&) = delete;

    explicit HuffmanTree(const int freq[256])
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

    void generateCodes(std::string codes[256]) const {
        generateCodes(root_, "", codes);
    }

    friend std::ostream& operator<<(std::ostream& out, const Node& node);

    class Cursor {
    private:
        const HuffmanTree& tree_;
        const Node* current_;
    public:
        explicit Cursor(const HuffmanTree& tree)
            : tree_(tree)
            , current_(tree.root_)
        {}

        void reset() {
            current_ = tree_.root_;
        }

        void move(int bit) {
            current_ = (bit == 0) ? current_->left : current_->right;
        }

        bool isLeaf() const {
            return current_->isLeaf();
        }

        unsigned char symbol() const {
            return static_cast<unsigned char>(current_->symbol);
        }
    };

    Cursor cursor() const {
        return Cursor(*this);
    }
};

#endif // HUFFMANTREE_H
