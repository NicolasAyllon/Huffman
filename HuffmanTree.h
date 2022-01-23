#ifndef HUFFMAN_TREE_H
#define HUFFMAN_TREE_H
#include <string>
#include <vector>
#include <queue> //std::priority_queue
#include <unordered_map>

struct BitNode {
  public:
    int frequency;
    bool terminal;
    unsigned char byte;
    BitNode* left;
    BitNode* right;
    
    BitNode(int frequency_in = 0, bool terminal_in = true, 
        unsigned char byte_in = '\0', BitNode* left_in = nullptr,
        BitNode* right_in = nullptr) :
      frequency(frequency_in),
      terminal(terminal_in),
      byte(byte_in),
      left(left_in),
      right(right_in)
      {}
};

class HuffmanTree {
  public:
    HuffmanTree(BitNode* root_in = nullptr);
    HuffmanTree(int byte_frequencies[256]);
    ~HuffmanTree();
    BitNode* root();

    std::unordered_map<unsigned char, std::vector<bool>> GetEncodingMap();
    void GetEncodingMap
        (BitNode* node, std::vector<bool>& encoded, 
        std::unordered_map<unsigned char, std::vector<bool>>& map);

    std::string Flatten();
    void Flatten(BitNode* node, std::string& encoding);
  private:
    BitNode* root_;
    void Erase(BitNode* node);
};


// Comparison struct with overloaded operator()
// is needed to create std::priority_queue as 
// min heap based on frequency
struct BitNodePtrComp {
  bool operator() (const BitNode* lhs, const BitNode* rhs) {
    return lhs->frequency > rhs->frequency;
  }
};

#endif // HUFFMAN_TREE_H