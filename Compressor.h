#ifndef COMPRESSOR_H
#define COMPRESSOR_H
#include <iostream>
#include <fstream>
#include <bitset>
#include <limits>
#include <stack>
#include "HuffmanTree.h"

class Compressor {
  public:
    Compressor();
    ~Compressor();
    std::string compress(const std::string& filename);
    std::string decompress(const std::string& filename);
    bool FilesAreIdentical(const std::string& filename1, const std::string& filename2);

  private:
    static const std::string compressed_file_extension_;

    bool FileExists(const std::string& filename);
    int GetFileLength(std::ifstream& file);
    std::string GetFileBaseName(const std::string& filename);
    std::string GetFileExtension(const std::string& filename);
    std::string MakeCompressedFileName(const std::string& filename);
    std::string MakeUniqueDecompressedFileName(const std::string& basename, const std::string& extension);

    void CountByteFrequencies(std::ifstream& ifs, int frequency[256]);
    HuffmanTree Unflatten(const std::string& encoding);
};

#endif // COMPRESSOR_H