#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <fstream>
#include <iostream>

#include "RAMFS.h"

//helper code
static std::array<char,RamAccess::k_RamSize> zerobuffer{};
constexpr int k_acceptableFsSize = 100;
class RamAccessFile : public RamAccess {
  public:
  RamAccessFile() = delete;
  RamAccessFile(const std::string& filename) : filename(filename) {}

  void RamClear(){
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
  virtual void RamWrite(void* pData, size_t size, size_t address) override {
    if (!CheckRamAccessParameters(pData, size, address)) {
      std::cerr << "Bad write access parameters" << std::endl;
      return;
    }
    std::fstream file(filename, std::ios::in | std::ios::out | std::ios::binary);
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

RamAccessFile RamFileEmulator("tests/RAMEmulator.bin");

TEST_GROUP(TestRamAccessFile){
  void setup(){
    RamFileEmulator.RamClear();
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

TEST_GROUP(TestFsInstantiation){
  void setup(){
    RamFileEmulator.RamClear();
  }
  void teardown() {

  }
};

TEST(TestFsInstantiation, FileSystemHasAcceptableSize) {
  RAMFS MyFileSystem1(RamAccess::k_RamSize, &RamFileEmulator);

  size_t RawFileSystemSize = MyFileSystem1.GetRawFileSystemSize();

  std::string ErrorString = "File System Size over limit("+std::to_string(k_acceptableFsSize)+"): " + std::to_string(RawFileSystemSize);

  CHECK_TEXT(RawFileSystemSize < k_acceptableFsSize, ErrorString.data());
}

  TEST(TestFsInstantiation, FileSystemIsStoredInRam) {
    RAMFS MyFileSystem1(RamAccess::k_RamSize, &RamFileEmulator);

    size_t RawFileSystemSize = MyFileSystem1.GetRawFileSystemSize();

    uint8_t* RamFileSystemParams = new uint8_t[RawFileSystemSize];
    uint8_t* GetterFileSystemParams = new uint8_t[RawFileSystemSize];
    RamFileEmulator.RamRead(RamFileSystemParams, RawFileSystemSize, 0);
    MyFileSystem1.GetRawFileSystem(GetterFileSystemParams, RawFileSystemSize);

    MEMCMP_EQUAL(RamFileSystemParams, GetterFileSystemParams,
                 RawFileSystemSize);

    delete[] RamFileSystemParams;
    delete[] GetterFileSystemParams;
  }

  TEST(TestFsInstantiation, FileSystemIsLoadedWhenEqualSize) {
    RAMFS MyFileSystem1(RamAccess::k_RamSize, &RamFileEmulator);
    MyFileSystem1._TempFileMangler_();
    RAMFS MyFileSystem2(RamAccess::k_RamSize,&RamFileEmulator);

    CHECK(MyFileSystem1.IsInitialized());
    CHECK(MyFileSystem2.IsInitialized());
    CHECK(!MyFileSystem1.WasLoaded());
    CHECK(MyFileSystem2.WasLoaded());
    CHECK(MyFileSystem1 == MyFileSystem2);
  }

    TEST(TestFsInstantiation, FileSystemIsNotLoadedWhenDifferentSize) {
    RAMFS MyFileSystem1(RamAccess::k_RamSize, &RamFileEmulator);
    RAMFS MyFileSystem2(RamAccess::k_RamSize-100,&RamFileEmulator);

    CHECK(MyFileSystem1.IsInitialized());
    CHECK(MyFileSystem2.IsInitialized());
    CHECK(!MyFileSystem1.WasLoaded());
    CHECK(!MyFileSystem2.WasLoaded());
    CHECK(!(MyFileSystem1 == MyFileSystem2));
  }

  TEST(TestFsInstantiation, NullPtrInstantionDoesntCrash) {
    RAMFS MyFileSystem1(RamAccess::k_RamSize, nullptr);

    // check that initialization was skipped due to bad param
    CHECK (!MyFileSystem1.IsInitialized()); 

  }

  TEST(TestFsInstantiation, InstatiationWithTooLargeSizeIsIgnored) {
    RAMFS MyFileSystem1(RamAccess::k_RamSize+10,&RamFileEmulator);

    // check that initialization was skipped due to bad param
    CHECK(!MyFileSystem1.IsInitialized());
  }


  //test instantiate with bad size Over ram size (from accessor) 

  // also test initialization test non standard arguments 
  //test case where file system does not fit in RAM

  //separate test files, separate src files

  // CONST CORRECTNESS!!! (also for methods)
  //add doxygen comments