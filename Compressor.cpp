#include "Compressor.h"

// Static Members
// **************
// Compressed files will have the extension .huf
const std::string Compressor::compressed_file_extension_ = "huf";



// Public Methods
// **************
Compressor::Compressor () {}

Compressor::~Compressor() {}

// Compress the input file and return the name of the compressed file
//
// Compressed files have the following structure:
// Header
//   1. null-teriminated string of extension of the original file
//      Example: ['j','p','g','\0']
//   2. length of the original file in bytes, as an int (4 bytes)
//   3. length of the flattened tree in bytes, as an int (4 bytes)
//   4. flattened Huffman tree used for decompression
// Content
//   5. compressed file data as a stream of bits
std::string Compressor::compress(const std::string& filename) {
  // Open file and verify it succeeded
  std::ifstream file(filename, std::ios::in | std::ios::binary);
  if(!file.is_open()) {
    std::cout << "ERROR: File not opened" << '\n';
    return "";
  }
  // Do not try to compress ".huf" files
  if(GetFileExtension(filename) == "huf") {
    std::cout << "ERROR: " << filename << " is already compressed" << '\n';
    return "";
  }
  //Build frequency table of bytes from file
  int frequencyTable[256] = {};
  CountByteFrequencies(file, frequencyTable);
  // Construct Huffman Tree
  HuffmanTree tree(frequencyTable);
  // Get mapping from bytes to its encoding as a sequence of bits
  auto encode_map = tree.GetEncodingMap();
  
  // Write compressed file
  // 0. Open a new file to write the data into
  std::string compressed_filename = MakeCompressedFileName(filename);
  std::ofstream outfile(compressed_filename, std::ios::out | std::ios::binary);

  // 1. null-terminated string of the extension of the original file
  std::string original_extension = GetFileExtension(filename);
  outfile.write(original_extension.c_str(), original_extension.length()+1);

  // 2. length of the original file in bytes, as an int (4 bytes)
  int file_length = GetFileLength(file);
  outfile.write(reinterpret_cast<char*>(&file_length), sizeof(int));

  // 3. length of the flattened tree in bytes, as an int (4 bytes)
  std::string flat_tree = tree.Flatten();
  int flat_tree_length = flat_tree.length();
  outfile.write(reinterpret_cast<char*>(&flat_tree_length), sizeof(int));

  // 4. flattened Huffman tree used for decompression
  outfile.write(flat_tree.c_str(), flat_tree_length);

  // 5. compressed file data as a stream of bits
  std::bitset<8> bit_accumulator; // initialized to 00000000
  int bit_index = 0; // Start at bit 0 (the rightmost bit)
  char c = '\0';
  while(file.get(c)) {
    std::vector<bool> encoded_byte = encode_map[static_cast<unsigned char>(c)];
    // Read all bits in the encoding of byte c
    for(bool bit : encoded_byte) {
      bit_accumulator[bit_index] = bit;
      bit_index++;
      if (bit_index == 8) {
        // Write byte to file
        unsigned char new_byte = bit_accumulator.to_ulong() & 0xFF;
        outfile.write(reinterpret_cast<char*>(&new_byte), 1);
        // Reset bit_index to 0
        bit_index = 0;
        // Clear bit_accumulator (optional)
        bit_accumulator.reset();
      }
    }
  }
  // If any bits were written to accumulator (bit_index > 0), 
  // write them to the file.
  if(bit_index > 0) {
    unsigned char last_byte = bit_accumulator.to_ulong() & 0xFF;
    outfile.write(reinterpret_cast<char*>(&last_byte), 1);
  }
  outfile.close();
  return compressed_filename;
}


// Decompress the input file and return the name of the decompressed file
std::string Compressor::decompress(const std::string& filename) {
  // 0. Open compressed file in binary mode and verify it succeeded
  std::ifstream compressed_file(filename, std::ios::in | std::ios::binary);
  if(!compressed_file.is_open()) {
    std::cout << "ERROR: File not opened" << '\n';
    return "";
  }
  // 1. Read original file extension (null-teminated C-string)
  char c = '\0';
  std::string original_extension = "";
  while(compressed_file.get(c) && c != '\0') {
    original_extension += c;
  }
  // 2. Read file length
  int decompressed_file_length = 0;
  compressed_file.read(reinterpret_cast<char*>(&decompressed_file_length), sizeof(int));

  // 3. Read length of flattened tree
  int flat_tree_length = 0;
  compressed_file.read(reinterpret_cast<char*>(&flat_tree_length), sizeof(int));

  // 4. Read flattened tree and unflatten
  char* flat_tree = new char[flat_tree_length + 1];
  compressed_file.read(flat_tree, flat_tree_length);
  flat_tree[flat_tree_length] = '\0'; // add null-terminator for c-string
  std::string flat_tree_s(flat_tree, flat_tree_length); // <!> NEW buffer ctor 
  HuffmanTree tree = Unflatten(flat_tree_s);
  delete[] flat_tree;
  
  // 5. Create new file to write decompressed data into
  std::string file_basename = GetFileBaseName(filename);
  std::string decompressed_filename = MakeUniqueDecompressedFileName(file_basename, original_extension);
  std::ofstream decompressed_file(decompressed_filename, std::ios::out | std::ios::binary);
  // Read 1 byte and then all 8 bits to traverse Huffman tree and 
  // write decoded bytes into the file.
  int decoded_bytes_written = 0;
  BitNode* current_node = tree.root();
  while(compressed_file.get(c)) {
    // Cast to unsigned char and then implicitly to int
    // to construct bitset
    std::bitset<8> byte_buffer(static_cast<unsigned char>(c));
    // Iterate through all 8 bits in byte_buffer
    for(int i = 0; i < 8; i++) {
      // If 0 = false is read, go left down tree
      if(byte_buffer[i] == false) { current_node = current_node->left;  }
      // If 1 = true is read, go right down tree
      if(byte_buffer[i] == true)  { current_node = current_node->right; }
      // If node is terminal (a leaf)
      if(current_node->terminal) {
        unsigned char decoded_byte = current_node->byte;
        // Write decoded character to file
        decompressed_file.write(reinterpret_cast<char*>(&decoded_byte), 1);
        // Reset current node to root
        current_node = tree.root();
        // Increment count of bytes written
        decoded_bytes_written++;
        // Stop decoding bits if all bytes 
        // in the original file have been decoded
        if(decoded_bytes_written == decompressed_file_length) break;
      }
    }
  }
  decompressed_file.close();
  return decompressed_filename;
}

// Return whether the two files are identical byte-by-byte
bool Compressor::FilesAreIdentical(const std::string& filename1, const std::string& filename2) {
  // Return false if either of the files does not exist
  if(!FileExists(filename1) || !FileExists(filename2)) return false;
  // At this point both files are known to exist, so open them
  std::ifstream ifs1(filename1, std::ios::in | std::ios::binary);
  std::ifstream ifs2(filename2, std::ios::in | std::ios::binary);
  unsigned char byte1 = 0;
  unsigned char byte2 = 0;
  int count = 1;
  // While both streams are good and bytes match, read next byte
  while(ifs1.good() && ifs2.good() && byte1 == byte2) {
    ifs1.read(reinterpret_cast<char*>(&byte1), 1);
    ifs2.read(reinterpret_cast<char*>(&byte2), 1);
  }
  // The files are identical if they match to the last byte,
  // and the end of both files was reached.
  return byte1 == byte2 && ifs1.eof() && ifs2.eof();
}


// Private Methods
// ***************
// Return whether the file with the given name exists, for use in this class
bool Compressor::FileExists(const std::string& filename) {
  std::ifstream ifs(filename);
  if(ifs.is_open()) {
    return true;
  } else {
    return false;
  } // ifstream destructor calls ifs.close()
}

// Return the length of an open file stream in bytes, 
// without changing the stream position, for use in this class
int Compressor::GetFileLength(std::ifstream& file) {
  std::streampos initial_pos = file.tellg();
  file.seekg(0, std::ios::beg);
  file.clear();
  file.ignore(std::numeric_limits<std::streamsize>::max());
  int length = file.gcount();
  file.seekg(initial_pos, std::ios::beg);
  file.clear();
  return length;
}

// Return the part of a file name before the dot and extension
// Example: "pictures/nebula.jpg" => "pictures/nebula"
std::string Compressor::GetFileBaseName(const std::string& filename) {
  return filename.substr(0, filename.find('.'));
}

// Return the file extension at the end of the name ("txt", "jpg", "mp3" ...)
std::string Compressor::GetFileExtension(const std::string& filename) {
  return filename.substr(filename.find('.') + 1);
}

// Return the name that will be given to the compressed file,
// keeping its original name but changing the extension to "huf"
// Example: "hamlet.txt" => "hamlet.huf"
std::string Compressor::MakeCompressedFileName(const std::string& filename) {
  std::size_t dot_pos = filename.find('.');
  std::string before_dot = filename.substr(0, dot_pos);
  return before_dot + "." + compressed_file_extension_;
}

// Find a unique name to write the decompressed file to avoid overwriting
std::string Compressor::MakeUniqueDecompressedFileName(const std::string& basename, const std::string& extension) {
  // Try "original.ext"
  std::string candidate = basename + "." + extension;
  if(!FileExists(candidate)) {
    return candidate;
  }
  // Try "original_decompressed.ext"
  candidate = basename + "_decompressed." + extension;
  if(!FileExists(candidate)) {
    return candidate;
  }
  // Try "original_decompressed (1).ext"
  //     "original_decompressed (2).ext"
  //     ...
  int num = 1;
  do {
    candidate = basename + "_decompressed (" + std::to_string(num) + ")." + extension;
    num++;
  } while (FileExists(candidate));
  return candidate;
}

// Count byte frequencies of file, writing into the provided array.
// Then, clear error flags on EOF and reset stream position to beginning.
void Compressor::CountByteFrequencies(std::ifstream& ifs, int frequency[256]){
  char c = '\0';
  while(ifs.get(c)) {
    frequency[static_cast<unsigned char>(c)]++;
  }
  ifs.clear(); // clear error flags from EOF to use get() again for compression
  ifs.seekg(0, std::ios::beg);
}

// Reconstruct a Huffman tree from a "flattened" string representation 
// based on its preorder traversal
//   -A nonterminal node is represented by "0(left subtree)(right subtree)"
//   -terminal nodes are represented by "1(decoded byte)"
//
// Example:
//                     •
//                    ↙ ↘
//   001a1b1c  =>    •   c
//                  ↙ ↘
//                 a   b
//
HuffmanTree Compressor::Unflatten(const std::string& encoding) {
  // Use a stack to hold the nodes visited in order
  std::stack<BitNode*> s;
  BitNode* root = nullptr;
  auto it = encoding.begin();
  // Read first 0 for the root node and push onto stack
  if(*it == '0') {
    root = new BitNode(); 
    root->terminal = false;
    s.push(root);
    it++;
  }
  // Read next character
  while(!s.empty() && it != encoding.end()) {
    BitNode* node = new BitNode();
    // If node at top of stack has no left child yet, set it to the new node.
    // Otherwise, set the right child to the new node.
    if(s.top()->left == nullptr) {
      s.top()->left = node;
    } else {
      s.top()->right = node;
    }
    // If next character is '0', mark node as nonterminal and push onto stack
    if(*it == '0') { 
      node->terminal = false;
      s.push(node);
    }
    // If next character is '1', mark node as terminal and
    // set its byte as the character immediately after the '1'
    if(*it == '1') { 
      node->terminal = true;
      node->byte = *(++it);
    }
    // Unwind stack until there is a node without a right child, or it's empty
    while (!s.empty() && s.top()->right != nullptr) {
      s.pop();
    }
    it++;
  }

  // Testing & Error Detection:
  // All characters in the flat tree were read, but the tree is not finished
  if(!s.empty())
    std::cout << "ERROR: encoding too short, tree incomplete\n";
  // A complete tree was built, but there are more characters in the flat tree
  if(it != encoding.end()) 
    std::cout << "ERROR: encoding too long, extra characters unused\n";

  return HuffmanTree(root);
}