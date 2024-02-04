#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <fstream>
#include <iostream>

#include "RAMFS.h"


static std::array<char,RAM_SIZE> zerobuffer{};
class RamAccessFile : public RamAccess {
  public:
  RamAccessFile() = delete;
  RamAccessFile(const std::string& filename) : filename(filename) {}

  virtual void RamWrite(void* pData, size_t size, size_t address) override {
    std::fstream file(filename,
                      std::ios::in | std::ios::out | std::ios::binary);
    if (!file) {
      std::cerr << "Cannot open file for writing." << std::endl;
      return;
    }

    // Move the file pointer to the specified address
    file.seekp(address, std::ios::beg);

    // Write data to the file
    file.write(static_cast<char*>(pData), size);

    file.close();
  }

  virtual void RamRead(void* pData, size_t size, size_t address) override {
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

RamAccessFile RamFileEmulator("tests/RAMEmulator.bin");

TEST_GROUP(TestRamAccessFile){
  void setup(){
  }
  void teardown(){
  }
};

TEST(TestRamAccessFile, SingleByteWriteAndRead) {
  uint8_t w_val = 0xAA;
  RamFileEmulator.RamWrite(&w_val,1,5);
  uint8_t r_val;
  RamFileEmulator.RamRead(&r_val,1,5);
  BYTES_EQUAL(w_val,r_val);
}

TEST(TestRamAccessFile, MultiByteWriteAndRead) {
  uint8_t w_vals[3] = {0xBB, 0xCC, 0xDD};
  RamFileEmulator.RamWrite(w_vals, sizeof(w_vals), 1);
  uint8_t r_vals[3];
  RamFileEmulator.RamRead(r_vals, sizeof(w_vals), 1);
  MEMCMP_EQUAL(w_vals,r_vals,sizeof(w_vals));
}