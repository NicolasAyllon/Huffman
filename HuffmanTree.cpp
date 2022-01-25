#include "HuffmanTree.h"

// Public Methods
// **************
HuffmanTree::HuffmanTree(BitNode* root_in /* = nullptr */) : root_(root_in) {

}

// Construct HuffmanTree from frequency table
HuffmanTree::HuffmanTree(int byte_frequencies[256]) {

  // Fill vector with pointers to heap-allocated BitNodes
  // for each byte that appeared at least once
  std::vector<BitNode*> v;
  v.reserve(256);
  for(int byte = 0; byte < 256; byte++) {
    if(byte_frequencies[byte] > 0) {
      v.emplace_back(new BitNode(byte_frequencies[byte], true, byte));
    }
  }
  // Construct min heap from vector
  std::priority_queue<BitNode*, std::vector<BitNode*>, BitNodePtrComp> 
    pq(v.begin(), v.end());

  // If there were no nodes, make tree root nullptr, then return
  if(pq.size() == 0) {
    root_ = nullptr;
    return;
  }
  // If only 1 node in priority queue, create the root node of the tree
  // with that node as its left child, then return
  if(pq.size() == 1) {
    BitNode* node = new BitNode();
    node->frequency = pq.top()->frequency;
    node->terminal = false;
    node->left = pq.top();
    node->right = nullptr;
    root_ = node;
    return;
  }
  // Otherwise, while priority queue has more than 1 node
  while(pq.size() > 1) {
    // Take two nodes from top and pop from priority queue
    BitNode* left_child = pq.top(); pq.pop();
    BitNode* right_child = pq.top(); pq.pop();
    // Create new nonterminal node with frequency equal to their sum
    // and set its left and right children to the nodes above
    BitNode* node = new BitNode();
    node->frequency = left_child->frequency + right_child->frequency;
    node->terminal = false;
    node->left = left_child;
    node->right = right_child;
    // Finally, push the newly created node onto the priority queue
    pq.push(node);
  }
  // The top of priority queue (with 1 element) is the root of the tree
  root_ = pq.top();
}


// Free all nodes of the HuffmanTree, starting at the root
HuffmanTree::~HuffmanTree() {
  Erase(root_);
}


// Return the root, for access from outside the class
BitNode* HuffmanTree::root() {
  return root_;
}

// Return a map from each byte (unsigned char) to its encoding 
// as a sequence of bits, represented as vector<bool>
std::unordered_map<unsigned char, std::vector<bool>> 
  HuffmanTree::GetEncodingMap () {
    std::unordered_map<unsigned char, std::vector<bool>> result = {};
    std::vector<bool> encoded = {};
    GetEncodingMap(root_, encoded, result);
    return result;
}
// Helping method
void HuffmanTree::GetEncodingMap 
    (BitNode* node, std::vector<bool>& encoded,  
    std::unordered_map<unsigned char, std::vector<bool>>& map) {
  // Base case
  if(node == nullptr) return;
  // Reached a leaf node, map its byte to the encoding built up in the vector
  if(node->terminal) {
    map[node->byte] = encoded;
  }
  // Otherwise, explore both left and right subtrees
  // Left subtree:
  // Push 0 = false on encoding vector, then recursively explore left subtree
  // and pop that 0 off afterwards.
  encoded.push_back(0);
  GetEncodingMap(node->left, encoded, map);
  encoded.pop_back();
  // Right subtree:
  // Push 1 = true on encoding vector, then recursively explore right subtree
  // and pop that 1 off afterwards.
  encoded.push_back(1);
  GetEncodingMap(node->right, encoded, map);
  encoded.pop_back();
}


// Returns a string representing the Huffman Tree based on preorder traversal
//   -A nonterminal node is represented by "0(left subtree)(right subtree)"
//   -terminal nodes are represented by "1(decoded byte)"
// Example:
//
//       •
//      ↙ ↘
//     •   c   =>  0(0(1a)(1b))(1c)  =>  001a1b1c
//    ↙ ↘
//   a   b
//
std::string HuffmanTree::Flatten() {
  std::string result = "";
  Flatten(root_, result);
  return result;
}
// Helping method which builds the string returned by Flatten() above
// Note: std::string encoding is an out parameter
void HuffmanTree::Flatten(BitNode* node, std::string& encoding) {
  // Base case
  if(node == nullptr) return;
  // Print terminal node as 1 followed by the byte that sequence encodes
  if(node->terminal) {
    encoding += "1";
    encoding += node->byte;
  }
  // Build string in preorder traversal
  // with a 0 indicating nonterminal nodes
  // and 1 (followed by byte) indicating terminal nodes
  else {
    encoding += "0";
    Flatten(node->left, encoding);
    Flatten(node->right, encoding);
  }
}

// Private Methods
// ***************
// Recursively traverse tree in postorder and free memory of all nodes.
void HuffmanTree::Erase(BitNode* node) {
  // Base case: if node is null, do nothing
  if(node == nullptr) return;
  // Delete left and right subtrees
  Erase(node->left);
  Erase(node->right);
  // Delete this node (postorder)
  delete node;
}
