#pragma once

#include <fstream>
#include <iostream>

#include "RamAccessFile.h"

constexpr int k_acceptableFsSize = 100;
static const std::array<char, RamAccess::k_RamSize> zerobuffer{};

class RamAccessFile : public RamAccess {
 public:
  RamAccessFile() = delete;
  RamAccessFile(const std::string& filename) : filename(filename) {}

  void RamClear() {
    std::fstream file(filename,
                      std::ios::in | std::ios::out | std::ios::binary);
    if (!file) {
      std::cerr << "Cannot open file for clear." << std::endl;
      return;
    }

    // Move the file pointer to the specified address
    file.seekp(0, std::ios::beg);

    // Write data to the file
    file.write(zerobuffer.data(), RamAccess::k_RamSize);

    file.close();
  }
  virtual void RamWrite(const void* const pData, const size_t size,
                        const size_t address) const override {
    if (!CheckRamAccessParameters(pData, size, address)) {
      std::cerr << "Bad write access parameters" << std::endl;
      return;
    }
    std::fstream file(filename,
                      std::ios::in | std::ios::out | std::ios::binary);
    if (!file) {
      std::cerr << "Cannot open file for writing." << std::endl;
      return;
    }

    // Move the file pointer to the specified address
    file.seekp(address, std::ios::beg);

    // Write data to the file
    file.write(static_cast<const char* const>(pData), size);

    file.close();
  }

  virtual void RamRead(void* const pData, const size_t size,
                       const size_t address) const override {
    if (!CheckRamAccessParameters(pData, size, address)) {
      std::cerr << "Bad read access parameters" << std::endl;
      return;
    }
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
      std::cerr << "Cannot open file for reading." << std::endl;
      return;
    }

    // Move the file pointer to the specified address
    file.seekg(address, std::ios::beg);

    // Read data from the file
    file.read(static_cast<char*>(pData), size);

    file.close();
  }

 private:
  std::string filename;
};