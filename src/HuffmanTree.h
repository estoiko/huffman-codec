#ifndef HUFFMANTREE_H
#define HUFFMANTREE_H

#include <cstdint>
#include <ios>
#include <ostream>

class HuffmanTree {
private:
    struct Node;
    struct NodePtrLess;

    void generateCodes(Node* node, std::string acc, std::string codes[256]) const;
    void deleteNode(Node* node);

    Node* root_;

public:
    explicit HuffmanTree(const int freq[256]);
    ~HuffmanTree();

    HuffmanTree(const HuffmanTree&) = delete;
    HuffmanTree& operator=(const HuffmanTree&) = delete;

    void generateCodes(std::string codes[256]) const;

    friend std::ostream& operator<<(std::ostream& out, const Node& node);

    class Cursor {
    private:
        const HuffmanTree& tree_;
        const Node* current_;
    public:
        explicit Cursor(const HuffmanTree& tree);

        void reset();
        void move(int bit);
        bool isLeaf() const;
        unsigned char symbol() const;
    };

    Cursor cursor() const;
};

#endif // HUFFMANTREE_H
