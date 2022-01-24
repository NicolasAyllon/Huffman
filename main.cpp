#include <iostream>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

#include "HuffmanTree.h"
#include "Compressor.h"

void PrintBanner() {
  std::cout << "┌───────────────────────────┐" << '\n';
  std::cout << "│ ██╗░░██╗██╗░░░██╗███████╗ │" << '\n';
  std::cout << "│ ██║░░██║██║░░░██║██╔════╝ │" << '\n';
  std::cout << "│ ███████║██║░░░██║█████╗░░ │" << '\n';
  std::cout << "│ ██╔══██║██║░░░██║██╔══╝░░ │" << '\n';
  std::cout << "│ ██║░░██║╚██████╔╝██║░░░░░ │" << '\n';
  std::cout << "│ ╚═╝░░╚═╝░╚═════╝░╚═╝░     │" << '\n';
  std::cout << "└─── C O M P R E S S O R ───┘" << '\n';
}

// Returns whether the file exists, for use in main()
bool FileExists(const std::string& filename) {
  std::ifstream ifs(filename);
  if(ifs.is_open()) {
    ifs.close();
    return true;
  } else {
    return false;
  }
}

// Returns file length, for use in main() to show size before/after compression
int GetFileLength(const std::string& filename) {
  int length = 0;
  std::ifstream ifs(filename, std::ios::in | std::ios::binary);
  ifs.ignore(std::numeric_limits<std::streamsize>::max());
  length = ifs.gcount();
  return length;
}

// Prompt the user for the name of a file that exists,
// or the empty string if the user wishes to quit
std::string PromptForFileNameOrQuit() {
  std::string filename = "";
  std::cout << "Enter file to compress or \".huf\" file to decompress." << '\n';
  std::cout << "(Leave blank to quit.)" << '\n';
  while(true) {
    std::cout << "> ";
    std::getline(std::cin, filename);
    if(FileExists(filename) || filename == "") { 
      return filename; 
    } else {
      std::cout << "File \"" << filename << "\" not found." << '\n';
      std::cout << "Is it spelled incorrectly or missing a directory?" << '\n';
      std::cout << "(Leave blank to quit.)" << '\n';
    }
  }
}


int main() {
  PrintBanner();
  std::cout << '\n';

  // Create compressor instance to use compress & decompress functions
  Compressor compressor;

  while(true) {
    std::string filename = PromptForFileNameOrQuit();
    if(filename.empty()) { break; }

    // The file is not a ".huf" file, so compress it
    // and place the resulting file in the same directory.
    std::string extension = filename.substr(filename.find('.') + 1);
    if(extension != "huf") {
      std::string compressed_filename = compressor.compress(filename);
      int original_length = GetFileLength(filename);
      int compressed_length = GetFileLength(compressed_filename);
      // Display results and file size before/after
      std::cout << "Compressed file \"" << compressed_filename 
                << "\" created" << '\n';
      std::cout << original_length << " bytes -> " 
                << compressed_length << " bytes" << '\n';
      std::cout << '\n';
    }

    // The file is a ".huf" file, so decompress it
    // and place the resulting file in the same directory.
    else {
      std::string decompressed_filename = compressor.decompress(filename);
      // Display results;
      std::cout << "Decompressed file \"" << decompressed_filename 
                << "\" created" << '\n';
      std::cout << '\n';
    }
  }
  
  return 0;
}
