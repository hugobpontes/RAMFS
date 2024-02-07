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
    RAMFS NotLoadedFileSystem(RamAccess::k_RamSize, &RamFileEmulator);
    // edit original fs so we can test that the second is loaded from it
    NotLoadedFileSystem._TempFileEdit_(); 
    RAMFS LoadedFileSystem(RamAccess::k_RamSize,&RamFileEmulator);

    CHECK(NotLoadedFileSystem.IsInitialized());
    CHECK(LoadedFileSystem.IsInitialized());
    CHECK(!NotLoadedFileSystem.WasLoaded());
    CHECK(LoadedFileSystem.WasLoaded());
    CHECK(NotLoadedFileSystem == LoadedFileSystem);
  }

    TEST(TestFsInstantiation, FileSystemIsNotLoadedWhenDifferentSize) {
      RAMFS NotLoadedFileSystem1(RamAccess::k_RamSize, &RamFileEmulator);
      RAMFS NotLoadedFileSystem2(RamAccess::k_RamSize - 100, &RamFileEmulator);

      CHECK(NotLoadedFileSystem1.IsInitialized());
      CHECK(NotLoadedFileSystem2.IsInitialized());
      CHECK(!NotLoadedFileSystem1.WasLoaded());
      CHECK(!NotLoadedFileSystem2.WasLoaded());
      CHECK(!(NotLoadedFileSystem1 == NotLoadedFileSystem2));
  }

  TEST(TestFsInstantiation, FileSystemIsNotLoadedWhenBrokenOrInexistent) {
    RAMFS NotLoadedFileSystem1(RamAccess::k_RamSize, &RamFileEmulator);
    // edit original fs so we can test that the second is not loaded from it
    NotLoadedFileSystem1._TempFileEdit_();
    //clear ram so file system can't be loaded from it
    RamFileEmulator.RamClear();
    RAMFS NotLoadedFileSystem2(RamAccess::k_RamSize, &RamFileEmulator);

    CHECK(NotLoadedFileSystem1.IsInitialized());
    CHECK(NotLoadedFileSystem2.IsInitialized());
    CHECK(!NotLoadedFileSystem1.WasLoaded());
    CHECK(!NotLoadedFileSystem2.WasLoaded());
    CHECK(!(NotLoadedFileSystem1 == NotLoadedFileSystem2));
  }

  TEST(TestFsInstantiation, NullPtrInstantionDoesntCrash) {
    RAMFS NullPtrFileSystem(RamAccess::k_RamSize, nullptr);

    // check that initialization was skipped due to bad param
    CHECK(!NullPtrFileSystem.IsInitialized());
  }

  TEST(TestFsInstantiation, InstatiationWithTooLargeRamSizeIsIgnored) {
    RAMFS TooLargeRamSizeFileSystem(RamAccess::k_RamSize+10,&RamFileEmulator);

    // check that initialization was skipped due to bad param
    CHECK(!TooLargeRamSizeFileSystem.IsInitialized());
  }

  TEST(TestFsInstantiation, InstatiationWithTooSmallRam) {
    // test case where ram size is too small to accomodate fs

    RAMFS UsedToGetSize1(RamAccess::k_RamSize, &RamFileEmulator);
    RAMFS<20, 20> UsedToGetSize2(RamAccess::k_RamSize, &RamFileEmulator);

    size_t DefaultFsSize = UsedToGetSize1.GetRawFileSystemSize();
    size_t CustomFsSize = UsedToGetSize2.GetRawFileSystemSize();

    RAMFS TooSmallFileSystem1(DefaultFsSize-1, &RamFileEmulator);
    RAMFS<20, 20> TooSmallFileSystem2(CustomFsSize - 1, &RamFileEmulator);
    //even though it is the same size as the first file system, it is created with enough ram to hold it
    RAMFS CorrectlySizedFileSystem(CustomFsSize - 1, &RamFileEmulator);

    CHECK(!TooSmallFileSystem1.IsInitialized());
    CHECK(!TooSmallFileSystem2.IsInitialized());
    CHECK(CorrectlySizedFileSystem.IsInitialized());
  }

  IGNORE_TEST(TestFsInstantiation, InstatiationWithNonDefaultArgumentsCreatesCorrectlySizedFs) {
    RAMFS<4,4> MyFileSystem1(RamAccess::k_RamSize, &RamFileEmulator);
    RAMFS<8,8> MyFileSystem2(RamAccess::k_RamSize, &RamFileEmulator);

    CHECK(MyFileSystem2.GetRawFileSystemSize()-MyFileSystem1.GetRawFileSystemSize()==(4*sizeof(RamFsFile)+4*sizeof(RamFsFragment)));
    CHECK(0); //always fail when run, until test is improved when structures are more stable

    //this seems to work, but due to packing, this test is implementation dependent, so delay it until structures are stable 
  }

  //->CONST CORRECTNESS!!! (also for methods)

  //separate test files, separate src files

  //add doxygen comments

  //test that a file can be created (and its arg variants), include status code variants (TIMESTAMP,invalid string too large, too small,no more file slots,...)
  //test that a file can be found (and its arg variants), include status code (invalid string too large, too small, not found)
  //test that one can write to a file (TIMESTAMP,and variants)
  //test that one can read from a file (and variants)
  //test that one can get fs free size
  //test that one can get file size
  //test that one can get file timestamp
  //test that one can get filename

  //only when all of the above are done, do we think about appending,deleting and variants that require multiple fragments


  //->Add non default size test when structures are stable