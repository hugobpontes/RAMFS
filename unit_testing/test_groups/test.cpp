#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <fstream>
#include <iostream>

#include "RamFs.h"
#include "RamAccessFile.h"
#include "RamFsFile.h"
#include "RamFsFragment.h"

RamAccessFile RamFileEmulator("unit_testing/RAMEmulator.bin");

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
  RamFs MyFileSystem1(RamAccess::k_RamSize, RamFileEmulator);

  size_t StorableFileSystemSize = MyFileSystem1.GetStorableFileSystemSize();

  std::string ErrorString = "File System Size over limit("+std::to_string(k_acceptableFsSize)+"): " + std::to_string(StorableFileSystemSize);

  CHECK_TEXT(StorableFileSystemSize < k_acceptableFsSize, ErrorString.data());
}

  TEST(TestFsInstantiation, FileSystemIsStoredInRam) {
    RamFs MyFileSystem1(RamAccess::k_RamSize, RamFileEmulator);

    size_t StorableFileSystemSize = MyFileSystem1.GetStorableFileSystemSize();

    uint8_t* RamFileSystemParams = new uint8_t[StorableFileSystemSize];
    uint8_t* GetterFileSystemParams = new uint8_t[StorableFileSystemSize];
    RamFileEmulator.RamRead(RamFileSystemParams, StorableFileSystemSize, 0);
    MyFileSystem1.GetStorableFileSystem(GetterFileSystemParams, StorableFileSystemSize);

    MEMCMP_EQUAL(RamFileSystemParams, GetterFileSystemParams,
                 StorableFileSystemSize);

    delete[] RamFileSystemParams;
    delete[] GetterFileSystemParams;
  }

  TEST(TestFsInstantiation, FileSystemIsLoadedWhenEqualSize) {
    RamFs NotLoadedFileSystem(RamAccess::k_RamSize, RamFileEmulator);
    // edit original fs so we can test that the second is loaded from it
    NotLoadedFileSystem._TempFileEdit_(); 
    RamFs LoadedFileSystem(RamAccess::k_RamSize,RamFileEmulator);

    CHECK(NotLoadedFileSystem.IsInitialized());
    CHECK(LoadedFileSystem.IsInitialized());
    CHECK(!NotLoadedFileSystem.WasLoaded());
    CHECK(LoadedFileSystem.WasLoaded());
    CHECK(NotLoadedFileSystem == LoadedFileSystem);
  }

    TEST(TestFsInstantiation, FileSystemIsNotLoadedWhenDifferentSize) {
      RamFs NotLoadedFileSystem1(RamAccess::k_RamSize, RamFileEmulator);
      RamFs NotLoadedFileSystem2(RamAccess::k_RamSize - 100, RamFileEmulator);

      CHECK(NotLoadedFileSystem1.IsInitialized());
      CHECK(NotLoadedFileSystem2.IsInitialized());
      CHECK(!NotLoadedFileSystem1.WasLoaded());
      CHECK(!NotLoadedFileSystem2.WasLoaded());
      CHECK(!(NotLoadedFileSystem1 == NotLoadedFileSystem2));
  }

  TEST(TestFsInstantiation, FileSystemIsNotLoadedWhenBrokenOrInexistent) {
    RamFs NotLoadedFileSystem1(RamAccess::k_RamSize, RamFileEmulator);
    // edit original fs so we can test that the second is not loaded from it
    NotLoadedFileSystem1._TempFileEdit_();
    //clear ram so file system can't be loaded from it
    RamFileEmulator.RamClear();
    RamFs NotLoadedFileSystem2(RamAccess::k_RamSize, RamFileEmulator);

    CHECK(NotLoadedFileSystem1.IsInitialized());
    CHECK(NotLoadedFileSystem2.IsInitialized());
    CHECK(!NotLoadedFileSystem1.WasLoaded());
    CHECK(!NotLoadedFileSystem2.WasLoaded());
    CHECK(!(NotLoadedFileSystem1 == NotLoadedFileSystem2));
  }

  TEST(TestFsInstantiation, InstatiationWithTooLargeRamSizeIsIgnored) {
    RamFs TooLargeRamSizeFileSystem(RamAccess::k_RamSize+10,RamFileEmulator);

    // check that initialization was skipped due to bad param
    CHECK(!TooLargeRamSizeFileSystem.IsInitialized());
  }

  TEST(TestFsInstantiation, InstatiationWithTooSmallRamIsIgnored) {
    // test case where ram size is too small to accomodate fs

    RamFs UsedToGetSize1(RamAccess::k_RamSize, RamFileEmulator);
    RamFs<20, 20> UsedToGetSize2(RamAccess::k_RamSize, RamFileEmulator);

    size_t DefaultFsSize = UsedToGetSize1.GetStorableFileSystemSize();
    size_t CustomFsSize = UsedToGetSize2.GetStorableFileSystemSize();

    RamFs TooSmallFileSystem1(DefaultFsSize-1, RamFileEmulator);
    RamFs<20, 20> TooSmallFileSystem2(CustomFsSize - 1, RamFileEmulator);
    //even though it is the same size as the first file system, it is created with enough ram to hold it
    RamFs CorrectlySizedFileSystem(CustomFsSize - 1, RamFileEmulator);

    CHECK(!TooSmallFileSystem1.IsInitialized());
    CHECK(!TooSmallFileSystem2.IsInitialized());
    CHECK(CorrectlySizedFileSystem.IsInitialized());
  }

  IGNORE_TEST(TestFsInstantiation, InstatiationWithNonDefaultArgumentsCreatesCorrectlySizedFs) {
    RamFs<4,4> MyFileSystem1(RamAccess::k_RamSize, RamFileEmulator);
    RamFs<8,8> MyFileSystem2(RamAccess::k_RamSize, RamFileEmulator);

    CHECK(MyFileSystem2.GetStorableFileSystemSize()-MyFileSystem1.GetStorableFileSystemSize()==(4*sizeof(RamFsFile)+4*sizeof(RamFsFragment)));
    CHECK(0); //always fail when run, until test is improved when structures are more stable

    //this seems to work, but due to packing, this test is implementation dependent, so delay it until structures are stable 
  }


TEST_GROUP(TestFileCreationAndFind){
  void setup(){
    RamFileEmulator.RamClear();
  }
  void teardown() {

  }
};

  TEST(TestFileCreationAndFind, SingleFileCanBeCreatedAndFound) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    RamFsFile* pMyFile1;
    RamFsFile* pMyFile2;
    RamFs_Status creation_status;
    RamFs_Status find_status;

    creation_status = MyFileSystem.CreateFile("a.txt", pMyFile1, 100);
    find_status = MyFileSystem.FindFile("a.txt", pMyFile2);

    CHECK(pMyFile1 == pMyFile2);
    CHECK(MyFileSystem.GetFileCount() == 1);
    CHECK(creation_status == RamFs_Status::SUCCESS);
    CHECK(find_status == RamFs_Status::SUCCESS);
  }

  TEST(TestFileCreationAndFind, MultipleFilesCanBeCreatedAndFound) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    RamFsFile* pMyFile1;
    RamFsFile* pMyFile2;
    RamFsFile* pMyFile3;
    RamFsFile* pMyFile4;
    RamFs_Status creation_status1;
    RamFs_Status creation_status2;
    RamFs_Status find_status1;
    RamFs_Status find_status2;

    creation_status1 = MyFileSystem.CreateFile("a.txt", pMyFile1, 100);
    creation_status2 = MyFileSystem.CreateFile("b", pMyFile2, 110); //confirm that single char filenames work
    find_status1 = MyFileSystem.FindFile("a.txt", pMyFile3);
    find_status2 = MyFileSystem.FindFile("b", pMyFile4);

    CHECK(MyFileSystem.GetFileCount() == 2);
    CHECK(creation_status1 == RamFs_Status::SUCCESS);
    CHECK(creation_status2 == RamFs_Status::SUCCESS);
    CHECK(find_status1 == RamFs_Status::SUCCESS);
    CHECK(find_status2 == RamFs_Status::SUCCESS);
    CHECK(pMyFile1 == pMyFile3);
    CHECK(pMyFile2 == pMyFile4);
  }

  TEST(TestFileCreationAndFind, CreateAndFindWithBadFilenames) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    RamFsFile* pMyFile1;
    RamFsFile* pMyFile2;
    RamFsFile* pMyFile3;
    RamFsFile* pMyFile4;
    RamFs_Status creation_status1;
    RamFs_Status creation_status2;
    RamFs_Status find_status1;
    RamFs_Status find_status2;

    creation_status1 = MyFileSystem.CreateFile("", pMyFile1, 100);
    creation_status2 = MyFileSystem.CreateFile("filename_is_long.txt", pMyFile2, 100);
    find_status1 = MyFileSystem.FindFile("", pMyFile3);
    find_status2 = MyFileSystem.FindFile("filename_is_long.txt", pMyFile4);

    CHECK(creation_status1 == RamFs_Status::INVALID_FILENAME);
    CHECK(find_status1 == RamFs_Status::INVALID_FILENAME);
    CHECK(creation_status2 == RamFs_Status::INVALID_FILENAME);
    CHECK(find_status2 == RamFs_Status::INVALID_FILENAME);
    CHECK(pMyFile1 == nullptr);
    CHECK(pMyFile2 == nullptr);
    CHECK(pMyFile3 == nullptr);
    CHECK(pMyFile4 == nullptr);
  }

  TEST(TestFileCreationAndFind, CreateAndFindInexistentFile) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    RamFsFile* pMyFile1;
    RamFsFile* pMyFile2;
    RamFs_Status creation_status;
    RamFs_Status find_status;

    creation_status = MyFileSystem.CreateFile("file.txt",pMyFile1,100);
    find_status = MyFileSystem.FindFile("doesnt_exist.txt",pMyFile2);

    CHECK(creation_status == RamFs_Status::SUCCESS);
    CHECK(find_status == RamFs_Status::FILE_NOT_FOUND);
    CHECK(pMyFile2 == nullptr);
  }
    // creating when file slots are full

    // tidy class/file names, make all lower case


    // test that a file can be created (and its arg variants), include status
    // code variants (TIMESTAMP,invalid string too large, too small,no more file
    // slots,...) test that a file can be found (and its arg variants), include
    // status code (invalid string too large, too small, not found) test that
    // one can write to a file (TIMESTAMP,and variants) test that one can read
    // from a file (and variants) test that one can get fs free size test that
    // one can get file size test that one can get file timestamp test that one
    // can get filename

    // only when all of the above are done, do we think about appending,deleting
    // and variants that require multiple fragments

    // add feature to only store parts of the filesystem

    //->Add non default size test when structures are stable

    // add doxygen comments