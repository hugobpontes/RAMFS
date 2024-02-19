#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <fstream>
#include <iostream>

#include "RamFs.h"
#include "RamAccessFile.h"

#include <stdlib.h>

RamAccessFile RamFileEmulator("unit_testing/RAMEmulator.bin");

RamFsFile* pMyFile1;
RamFsFile* pMyFile2;
RamFsFile* pMyFile3;
RamFsFile* pMyFile4;
RamFsFile* pMyFile5;
RamFsFile* pMyFile6;
RamFs_Status read_status1;
RamFs_Status read_status2;
RamFs_Status read_status3;
RamFs_Status write_status1;
RamFs_Status write_status2;
RamFs_Status write_status3;
RamFs_Status creation_status1;
RamFs_Status creation_status2;
RamFs_Status creation_status3;
RamFs_Status find_status1;
RamFs_Status find_status2;
RamFs_Status find_status3;
RamFs_Status append_status1;
RamFs_Status append_status2;
RamFs_Status delete_status1;
RamFs_Status delete_status2;
size_t free_size_after_writing1;
Timestamp modif_time1;
Timestamp modif_time2;
Timestamp modif_time3;
Timestamp creation_time1;
Timestamp creation_time2;
Timestamp creation_time3;
size_t free_size_after_writing2;
size_t free_size_after_writing3;
size_t file_size_after_writing1;
size_t file_size_after_writing2;
size_t file_size_after_writing3;
size_t write_size1;
size_t write_size2;
size_t write_size3;
size_t read_size1;
size_t read_size2;
size_t read_size3;
size_t read_pos1;
size_t read_pos2;
size_t read_pos3;

uint8_t* WriteData;
uint8_t* ReadData;

/*one more than the usable file system space*/
int DataArraySize = RamAccess::k_RamSize - RamFs::GetStorableParamsSize()+ 1;
constexpr Timestamp time1 = 100;
constexpr Timestamp time2 = 110;
constexpr Timestamp time3 = 120;
constexpr Timestamp time4 = 130;
constexpr Timestamp time5 = 140;
constexpr Timestamp time6 = 150;

constexpr char fname1[] = "file1.txt";
constexpr char fname2[] = "file2.txt";
constexpr char fname3[] = "file3.txt";

static void fillDataArray(uint8_t* arr, size_t size, bool zero_fill ) {
  int j;
  for (int i = 0; i < size; i++) {
    if (zero_fill){
      arr[i]=0;
    } else {
      arr[i] = j++;
      j = (j > 255) ? 0 : j;
    }
  }
}

static void initializeDataArrays() {
  WriteData = new uint8_t[DataArraySize];
  ReadData = new uint8_t[DataArraySize];
  fillDataArray(WriteData, DataArraySize,false);
  fillDataArray(ReadData, DataArraySize, true);
}

static void deleteDataArrays(){
  delete[] WriteData;
  delete[] ReadData;
  WriteData = nullptr;
  ReadData = nullptr;
}
static void resetGlobalTestData() {
  pMyFile1 = nullptr;
  pMyFile2 = nullptr;
  pMyFile3 = nullptr;
  pMyFile4 = nullptr;
  pMyFile5 = nullptr;
  pMyFile6 = nullptr;

  read_status1 = RamFs_Status::STATUS_UNSET;
  read_status2 = RamFs_Status::STATUS_UNSET;
  read_status3 = RamFs_Status::STATUS_UNSET;
  write_status1 = RamFs_Status::STATUS_UNSET;
  write_status2 = RamFs_Status::STATUS_UNSET;
  write_status3 = RamFs_Status::STATUS_UNSET;
  creation_status1 = RamFs_Status::STATUS_UNSET;
  creation_status2 = RamFs_Status::STATUS_UNSET;
  creation_status3 = RamFs_Status::STATUS_UNSET;
  delete_status1 = RamFs_Status::STATUS_UNSET;
  delete_status2 = RamFs_Status::STATUS_UNSET;
  find_status1 = RamFs_Status::STATUS_UNSET;
  find_status2 = RamFs_Status::STATUS_UNSET;
  find_status3 = RamFs_Status::STATUS_UNSET;
  append_status1 = RamFs_Status::STATUS_UNSET;
  append_status2 = RamFs_Status::STATUS_UNSET;
  free_size_after_writing1 = 0;
  free_size_after_writing2 = 0;
  free_size_after_writing3 = 0;
  file_size_after_writing1 = 0;
  file_size_after_writing2 = 0;
  file_size_after_writing3 = 0;
  write_size1 = 0;
  write_size2 = 0;
  write_size3 = 0;
  read_size1 = 0;
  read_size2 = 0;
  read_size3 = 0;
  read_pos1 = 0;
  read_pos2 = 0;
  read_pos3 = 0;
}

static void defaultSetup(){
  RamFileEmulator.RamClear();
  initializeDataArrays();
  resetGlobalTestData();  
}

static void defaultTeardown(){
  deleteDataArrays();
}

TEST_GROUP(TestRamAccessFile){
  void setup(){
    defaultSetup();
  }
  void teardown(){
    defaultTeardown();
  }
};

TEST(TestRamAccessFile, SingleByteWriteAndRead) {
  write_size1 = 1;
  constexpr int k_Address = 5;
  RamFileEmulator.RamWrite(WriteData, write_size1, k_Address);
  RamFileEmulator.RamRead(ReadData, write_size1, k_Address);
  BYTES_EQUAL(WriteData[0], ReadData[0]);
}

TEST(TestRamAccessFile, MultiByteWriteAndRead) {
  write_size1 = 3;
  constexpr int k_Address = 0;
  RamFileEmulator.RamWrite(WriteData, write_size1, k_Address);
  RamFileEmulator.RamRead(ReadData, write_size1, k_Address);
  MEMCMP_EQUAL(WriteData, ReadData, write_size1);
}

TEST_GROUP(TestFsInstantiation){
  void setup(){
    defaultSetup();
  }
  void teardown(){
    defaultTeardown();
  }
};

TEST(TestFsInstantiation, FileSystemHasAcceptableSize) {
  RamFs MyFileSystem1(RamAccess::k_RamSize, RamFileEmulator);
  size_t StorableFileSystemSize = RamFs::GetStorableParamsSize();

  std::string ErrorString = "File System Size over limit("+std::to_string(k_acceptableFsSize)+"): " + std::to_string(StorableFileSystemSize);

  CHECK_TEXT(StorableFileSystemSize < k_acceptableFsSize, ErrorString.data());
}


  TEST(TestFsInstantiation, FileSystemIsLoadedWhenEqualSize) {
    RamFs NotLoadedFileSystem(RamAccess::k_RamSize, RamFileEmulator);
    // edit original fs so we can test that the second is loaded from it
    write_size1 = 2;
    NotLoadedFileSystem.CreateFile(fname1,pMyFile1,time1);
    pMyFile1->Write(WriteData, write_size1, time2);
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
    write_size1 = 2;
    NotLoadedFileSystem1.CreateFile(fname1, pMyFile1, time1);
    pMyFile1->Write(WriteData, write_size1, time2);
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

    RamFs CorrectlySizedFileSystem(RamAccess::k_RamSize, RamFileEmulator);
    RamFs TooSmallFileSystem(RamFs::GetStorableParamsSize() - 1, RamFileEmulator);

    CHECK(!TooSmallFileSystem.IsInitialized());
    CHECK(CorrectlySizedFileSystem.IsInitialized());
  }

TEST_GROUP(TestFileCreationAndFind){
  void setup(){
    defaultSetup();
  }
  void teardown(){
    defaultTeardown();
  }
};

  TEST(TestFileCreationAndFind, SingleFileCanBeCreatedAndFound) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    creation_status1 = MyFileSystem.CreateFile(fname1, pMyFile1, time1);
    find_status1 = MyFileSystem.FindFile(fname1, pMyFile2);

    CHECK_EQUAL(pMyFile1,pMyFile2);
    CHECK_EQUAL(MyFileSystem.GetFileCount(),1);
    CHECK(creation_status1 == RamFs_Status::SUCCESS);
    CHECK(find_status1 == RamFs_Status::SUCCESS);
  }

  TEST(TestFileCreationAndFind, MultipleFilesCanBeCreatedAndFound) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    creation_status1 = MyFileSystem.CreateFile(fname1, pMyFile1, time1);
    creation_status2 = MyFileSystem.CreateFile(fname2, pMyFile2, time2); //also tests that single char names work
    find_status1 = MyFileSystem.FindFile(fname1, pMyFile3);
    find_status2 = MyFileSystem.FindFile(fname2, pMyFile4);

    CHECK_EQUAL(MyFileSystem.GetFileCount(), 2);
    CHECK(creation_status1 == RamFs_Status::SUCCESS);
    CHECK(creation_status2 == RamFs_Status::SUCCESS);
    CHECK(find_status1 == RamFs_Status::SUCCESS);
    CHECK(find_status2 == RamFs_Status::SUCCESS);
    CHECK_EQUAL(pMyFile1,pMyFile3);
    CHECK_EQUAL(pMyFile2,pMyFile4);
  }
    TEST(TestFileCreationAndFind, CreateAndFindWithBadFilenames) {
      RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

      creation_status1 = MyFileSystem.CreateFile("", pMyFile1, time1);
      creation_status2 = MyFileSystem.CreateFile("filename_is_long.txt", pMyFile2, time2);
      creation_status3 = MyFileSystem.CreateFile(nullptr, pMyFile3, time3);

      find_status1 = MyFileSystem.FindFile("", pMyFile4);
      find_status2 = MyFileSystem.FindFile("filename_is_long.txt", pMyFile5);
      find_status3 = MyFileSystem.FindFile(nullptr, pMyFile6);

      CHECK(creation_status1 == RamFs_Status::INVALID_FILENAME);
      CHECK(find_status1 == RamFs_Status::INVALID_FILENAME);
      CHECK(creation_status2 == RamFs_Status::INVALID_FILENAME);
      CHECK(find_status2 == RamFs_Status::INVALID_FILENAME);
      CHECK(creation_status3 == RamFs_Status::NULL_POINTER);
      CHECK(find_status3 == RamFs_Status::NULL_POINTER);
      CHECK_EQUAL(pMyFile1,nullptr);
      CHECK_EQUAL(pMyFile2,nullptr);
      CHECK_EQUAL(pMyFile3,nullptr);
      CHECK_EQUAL(pMyFile4,nullptr);
      CHECK_EQUAL(pMyFile5,nullptr);
      CHECK_EQUAL(pMyFile6,nullptr);
    }

    TEST(TestFileCreationAndFind, CreateAndFindInexistentFile) {
      RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

      creation_status1 = MyFileSystem.CreateFile("file.txt", pMyFile1, time1);
      find_status1 = MyFileSystem.FindFile("doesnt_exist.txt", pMyFile1);

      CHECK(creation_status1 == RamFs_Status::SUCCESS);
      CHECK(find_status1 == RamFs_Status::FILE_NOT_FOUND);
      CHECK_EQUAL(pMyFile1,nullptr);
    }

    TEST(TestFileCreationAndFind, FileSlotsFull) {
      RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

      for (int i = 0; i < k_FileNr; i++) {
        creation_status1 = MyFileSystem.CreateFile(("name" + std::to_string(i)).data(), pMyFile1, time1);
        CHECK(creation_status1 == RamFs_Status::SUCCESS);
      }
      creation_status1 = MyFileSystem.CreateFile(("name" + std::to_string(10)).data(), pMyFile1, time2);
      CHECK(creation_status1 == RamFs_Status::FILE_SLOTS_FULL);
      CHECK_EQUAL(pMyFile1, nullptr);
    }

    TEST(TestFileCreationAndFind, FileAlreadyExists) {
      RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

      creation_status1 = MyFileSystem.CreateFile(fname1, pMyFile1, time1);
      creation_status2 = MyFileSystem.CreateFile(fname1, pMyFile2,time2);  // timestamp is ignored

      CHECK(creation_status1 == RamFs_Status::SUCCESS);
      CHECK(creation_status2 == RamFs_Status::SUCCESS);
      CHECK_EQUAL(pMyFile2,pMyFile1);
      CHECK_EQUAL(MyFileSystem.GetFileCount(), 1);
      /*Timestamp should be unaltered when creating a file already created*/
      CHECK_EQUAL(pMyFile1->GetCreationTimestamp(),pMyFile2->GetCreationTimestamp());
    }

TEST_GROUP(TestFileWriteRead){
  void setup(){
    defaultSetup();
  }
  void teardown(){
    defaultTeardown();
  }
};

  TEST(TestFileWriteRead, BasicWriteAndRead) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);
    write_size1 = 3;

    MyFileSystem.CreateFile(fname1, pMyFile1, time1);
    write_status1 = pMyFile1->Write(WriteData, write_size1, time2);
    free_size_after_writing1 = MyFileSystem.GetFreeSize();
    file_size_after_writing1 = pMyFile1->GetSize();

    RamFs MyFileSystem2(RamAccess::k_RamSize, RamFileEmulator);
    MyFileSystem2.FindFile(fname1, pMyFile2);
    read_status1 = pMyFile2->Read(ReadData, pMyFile2->GetSize(), 0);

    CHECK(write_status1 == RamFs_Status::SUCCESS);
    CHECK(read_status1 == RamFs_Status::SUCCESS);
    CHECK_EQUAL(file_size_after_writing1, write_size1);
    CHECK_EQUAL(free_size_after_writing1,RamAccess::k_RamSize - RamFs::GetStorableParamsSize() - write_size1);
    MEMCMP_EQUAL(WriteData, ReadData, write_size1);
  }

    TEST(TestFileWriteRead, MultipleWriteAndRead) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    write_size1 = 7;
    write_size2 = 9;
    write_size3 = 3;

    MyFileSystem.CreateFile(fname1, pMyFile1, time1);
    write_status1 = pMyFile1->Write(WriteData,write_size1,time2);
    free_size_after_writing1 = MyFileSystem.GetFreeSize();
    file_size_after_writing1 = pMyFile1->GetSize();

    RamFs MyFileSystem2(RamAccess::k_RamSize, RamFileEmulator);
    MyFileSystem2.FindFile(fname1, pMyFile2);
    read_status1 = pMyFile2->Read(ReadData, pMyFile2->GetSize(), 0);
    MyFileSystem2.CreateFile(fname2, pMyFile3, time3);
    write_status2 = pMyFile3->Write(WriteData, write_size2, time4);
    free_size_after_writing2 = MyFileSystem2.GetFreeSize();
    file_size_after_writing2 = pMyFile3->GetSize();

    RamFs MyFileSystem3(RamAccess::k_RamSize, RamFileEmulator);
    MyFileSystem3.FindFile(fname2, pMyFile4);
    read_status2 = pMyFile4->Read(ReadData+write_size1, pMyFile4->GetSize(), 0);

    MyFileSystem3.CreateFile(fname3, pMyFile5, time5);
    write_status3 = pMyFile5->Write(WriteData,write_size3,time6);
    read_status3 = pMyFile5->Read(ReadData + write_size1+write_size2, write_size3, 0);
    free_size_after_writing3 = MyFileSystem3.GetFreeSize();
    file_size_after_writing3 = pMyFile5->GetSize();

    CHECK(write_status1 == RamFs_Status::SUCCESS);
    CHECK(read_status1 == RamFs_Status::SUCCESS);
    CHECK(write_status2 == RamFs_Status::SUCCESS);
    CHECK(read_status2 == RamFs_Status::SUCCESS);
    CHECK(write_status3 == RamFs_Status::SUCCESS);
    CHECK(read_status3 == RamFs_Status::SUCCESS);
    CHECK_EQUAL(file_size_after_writing1,write_size1);
    CHECK_EQUAL(file_size_after_writing2,write_size2);
    CHECK_EQUAL(file_size_after_writing3,write_size3);
    CHECK_EQUAL(free_size_after_writing1,RamAccess::k_RamSize - RamFs::GetStorableParamsSize() - write_size1);
    CHECK_EQUAL(free_size_after_writing2,RamAccess::k_RamSize - RamFs::GetStorableParamsSize() - write_size1 - write_size2);
    CHECK_EQUAL(free_size_after_writing3,RamAccess::k_RamSize - RamFs::GetStorableParamsSize() - write_size1 - write_size2- write_size3);
    MEMCMP_EQUAL(WriteData, ReadData, write_size1);
    MEMCMP_EQUAL(WriteData, ReadData+write_size1, write_size2);
    MEMCMP_EQUAL(WriteData, ReadData+write_size1+write_size2, write_size3);
  }

  TEST(TestFileWriteRead, WriteToSameFileTwice) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    write_size1 = 9;
    write_size2 = 7;

    MyFileSystem.CreateFile(fname1, pMyFile1, time1);

    write_status1 = pMyFile1->Write(WriteData, write_size1,time1);
    free_size_after_writing1 = MyFileSystem.GetFreeSize();
    file_size_after_writing1 = pMyFile1->GetSize();
    read_status1 = pMyFile1->Read(ReadData, pMyFile1->GetSize(), 0);
    creation_time1 = pMyFile1->GetCreationTimestamp();
    modif_time1 = pMyFile1->GetModificationTimestamp();

    write_status2 = pMyFile1->Write(WriteData, write_size2, time2);
    free_size_after_writing2 = MyFileSystem.GetFreeSize();
    file_size_after_writing2 = pMyFile1->GetSize();
    read_status2 = pMyFile1->Read(ReadData+write_size1, pMyFile1->GetSize(), 0);
    modif_time2 = pMyFile1->GetModificationTimestamp();

    CHECK(write_status1 == RamFs_Status::SUCCESS);
    CHECK(write_status2 == RamFs_Status::SUCCESS);
    MEMCMP_EQUAL(WriteData, ReadData, write_size1);
    MEMCMP_EQUAL(WriteData, ReadData+write_size1, write_size2);
    CHECK_EQUAL(file_size_after_writing1, write_size1);
    CHECK_EQUAL(free_size_after_writing1, RamAccess::k_RamSize - RamFs::GetStorableParamsSize() - write_size1);
    CHECK_EQUAL(file_size_after_writing2, write_size2);
    CHECK_EQUAL(free_size_after_writing2, RamAccess::k_RamSize - RamFs::GetStorableParamsSize() - write_size2);
    CHECK_EQUAL(creation_time1, time1);
    CHECK_EQUAL(modif_time1, time1);
    CHECK_EQUAL(modif_time2,time2);

  }

  void Helper_FillByteArray(uint8_t* arr, size_t size){
    int j;
    for (int i = 0; i < size; i++) {
      arr[i] = j++;
      j = (j > 255) ? 0 : j;
    }
  }

  void Helper_WriteArrayToFileAndRead(RamFs& Fs, RamFsFile*& pFile,
                                      uint8_t*& WriteArr, uint8_t*& ReadArr,
                                      size_t size, RamFs_Status& WriteStatus,
                                      RamFs_Status& ReadStatus,
                                      size_t& FreeSizeAfterWriting,
                                      size_t& FileSizeAfterWriting) {
    WriteArr = new uint8_t[size];
    ReadArr = new uint8_t[size];
    Helper_FillByteArray(WriteArr,size);
    Fs.CreateFile("file.txt", pFile, 100);

    WriteStatus = pFile->Write(WriteArr, size, 110);
    FreeSizeAfterWriting = Fs.GetFreeSize();
    FileSizeAfterWriting = pFile->GetSize();
    ReadStatus = pFile->Read(ReadArr, pFile->GetSize(), 0);
  }

  TEST(TestFileWriteRead, Write1ByteLessThanFillingFs) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    MyFileSystem.CreateFile(fname1, pMyFile1, time1);

    write_size1 = MyFileSystem.GetFreeSize() - 1;

    write_status1 = pMyFile1->Write(WriteData, write_size1, time2);
    free_size_after_writing1 = MyFileSystem.GetFreeSize();
    file_size_after_writing1 = pMyFile1->GetSize();
    read_status1 = pMyFile1->Read(ReadData, pMyFile1->GetSize(), 0);

    CHECK(write_status1 == RamFs_Status::SUCCESS);
    CHECK(read_status1 == RamFs_Status::SUCCESS);
    MEMCMP_EQUAL(WriteData, ReadData, write_size1);
    CHECK_EQUAL(file_size_after_writing1, write_size1);
    CHECK_EQUAL(free_size_after_writing1,RamAccess::k_RamSize - RamFs::GetStorableParamsSize() - write_size1);

  }

  TEST(TestFileWriteRead, FillFs) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    MyFileSystem.CreateFile(fname1, pMyFile1, time1);

    write_size1 = MyFileSystem.GetFreeSize();

    write_status1 = pMyFile1->Write(WriteData, write_size1, time2);
    free_size_after_writing1 = MyFileSystem.GetFreeSize();
    file_size_after_writing1 = pMyFile1->GetSize();
    read_status1 = pMyFile1->Read(ReadData, pMyFile1->GetSize(), 0);

    CHECK(write_status1 == RamFs_Status::SUCCESS);
    CHECK(read_status1 == RamFs_Status::SUCCESS);
    MEMCMP_EQUAL(WriteData, ReadData, write_size1);
    CHECK_EQUAL(file_size_after_writing1, write_size1);
    CHECK_EQUAL(free_size_after_writing1,RamAccess::k_RamSize - RamFs::GetStorableParamsSize() - write_size1);
  }

    TEST(TestFileWriteRead, OverflowFs) {
      RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

      MyFileSystem.CreateFile(fname1, pMyFile1, time1);

      write_size1 = MyFileSystem.GetFreeSize()+1;

      write_status1 = pMyFile1->Write(WriteData, write_size1, time2);
      free_size_after_writing1 = MyFileSystem.GetFreeSize();
      file_size_after_writing1 = pMyFile1->GetSize();

      CHECK(write_status1 == RamFs_Status::INSUFFICIENT_STORAGE);
      CHECK_EQUAL(free_size_after_writing1,RamAccess::k_RamSize - RamFs::GetStorableParamsSize());
      CHECK_EQUAL(file_size_after_writing1, 0);
  }

  TEST(TestFileWriteRead, NullPtrWrite) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    MyFileSystem.CreateFile("file.txt", pMyFile1, time1);

    write_size1 = 10;
    write_status1 = pMyFile1->Write(nullptr, write_size1, 400);
    free_size_after_writing1 = MyFileSystem.GetFreeSize();
    file_size_after_writing1 = pMyFile1->GetSize();

    CHECK(write_status1 == RamFs_Status::NULL_POINTER);
    CHECK(free_size_after_writing1 == RamAccess::k_RamSize - RamFs::GetStorableParamsSize());
    CHECK(file_size_after_writing1 == 0);
  }

  TEST(TestFileWriteRead, Write0Bytes) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    write_size1 = 0;

    MyFileSystem.CreateFile(fname1, pMyFile1, time1);

    write_status1 = pMyFile1->Write(WriteData, write_size1, time1);
    free_size_after_writing1 = MyFileSystem.GetFreeSize();
    file_size_after_writing1 = pMyFile1->GetSize();

    CHECK(write_status1 == RamFs_Status::SUCCESS);
    CHECK_EQUAL(free_size_after_writing1,RamAccess::k_RamSize - RamFs::GetStorableParamsSize());
    CHECK_EQUAL(file_size_after_writing1, 0);

  }

  TEST(TestFileWriteRead, ReadNullPtr) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    write_size1 = 10;

    MyFileSystem.CreateFile(fname1, pMyFile1, time1);

    write_status1 = pMyFile1->Write(WriteData, write_size1, time2);
    read_status1 = pMyFile1->Read(nullptr, pMyFile1->GetSize(), 0);

    CHECK(read_status1 == RamFs_Status::NULL_POINTER);

  }

  TEST(TestFileWriteRead, ReadSize0) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    write_size1 = 3;
    read_size1 = 0;

    MyFileSystem.CreateFile(fname1, pMyFile1, time1);

    write_status1 = pMyFile1->Write(WriteData, write_size1, time2);
    read_status1 = pMyFile1->Read(ReadData, read_size1, 0);

    CHECK(read_status1 == RamFs_Status::SUCCESS);
    CHECK(memcmp(ReadData,WriteData,write_size1)); //shows that read data wasnt written to.
  }

  TEST(TestFileWriteRead, ReadOverFileSize) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    write_size1 = 3;
    read_size1 = 4;

    size_t should_match_size = write_size1;
    size_t should_not_match_size = read_size1-write_size1;

    MyFileSystem.CreateFile(fname1, pMyFile1, time1);

    write_status1 = pMyFile1->Write(WriteData, write_size1, time2);
    read_status1 = pMyFile1->Read(ReadData, read_size1, 0);

    CHECK(read_status1 == RamFs_Status::SUCCESS);
    // shows that read data was written to until byte 3.
    CHECK(!memcmp(ReadData,WriteData,should_match_size));
    // shows that read data wasnt written to beyond byte 3.
    CHECK(memcmp(ReadData+should_match_size,WriteData+should_match_size,should_not_match_size));  
  }

  TEST(TestFileWriteRead, ReadStartingFromBeyondFile) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    write_size1 = 3;
    read_pos1 = write_size1; //out of bounds read

    MyFileSystem.CreateFile(fname1, pMyFile1, time1);

    write_status1 = pMyFile1->Write(WriteData, write_size1, time2);
    read_status1 = pMyFile1->Read(ReadData, read_size1, read_pos1);

    CHECK(read_status1 == RamFs_Status::READ_OUT_OF_BOUNDS);
  }

  TEST(TestFileWriteRead, ReadStartingFromMiddle) {   
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    write_size1 = 5;
    read_size1 = 3;

    read_pos1 = 2;

    MyFileSystem.CreateFile(fname1, pMyFile1, time1);

    write_status1 = pMyFile1->Write(WriteData, write_size1, time2);
    read_status1 = pMyFile1->Read(ReadData, read_size1,read_pos1);

    CHECK(read_status1 == RamFs_Status::SUCCESS);
    MEMCMP_EQUAL(WriteData+read_pos1,ReadData,read_size1);

  }

  TEST(TestFileWriteRead, ReadStartingFromMiddleAndOverFileSize) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    write_size1 = 5;
    read_size1 = 7;
    read_pos1 = 2;

    size_t should_match_size = write_size1-read_pos1;
    size_t should_not_match_size = read_size1 - (write_size1-read_pos1);

    MyFileSystem.CreateFile(fname1, pMyFile1, time1);

    write_status1 = pMyFile1->Write(WriteData, write_size1, time2);
    read_status1 = pMyFile1->Read(ReadData, read_size1, read_pos1);

    CHECK(read_status1 == RamFs_Status::SUCCESS);
    // shows that read data was written to until byte 3 (5-2), with data starting at start of written data+2.
    CHECK(!memcmp(ReadData,WriteData+read_pos1,should_match_size));
    // shows that read data wasnt written to beyond byte 3 (5-2).
    CHECK(memcmp(ReadData+should_match_size,WriteData+should_match_size+read_pos1,should_not_match_size)); 
  }
    //test basic delete (more free size, decreased file count, file cannot be found)
    //test delete allows for 11th file, and that it can be found, wrriten to and read after instantiating new file system.
    //test fragmentation by creating small file, large file, small file. Then deleting the small ones, and having a new file take those frags 
    //(one must use up all of the fs to create a scenario where it wouldnt work without fragmenation)
    //test too fragmented file (same as above but two large files)

    //test too fragmented files (>3)
    //ignore test with fragmented files (needs delete), this includes checking status
    //ignore test with defragmentation (needs delete),this includes checking status

    //test appending to 1 byte less than full, full, over full (like we did for normal writing)

    //test deletion, appending, etc.. (deletion is essentially what is done in the beginning of write)
    //test all return messages
    //test something that would onnly be possible with defragmentation

    /* test that one can write to a file (TIMESTAMP,and variants) test that one can read from a file (and variants) test that one can get fs free size test that one can get file size test that one can get file timestamp test that one can get filename only when all of the above are done, do we think about appending,deleting and variants that require multiple fragments add feature to only store parts of the filesystem ->Add non default size test when structures are stable add doxygen comments*/

    //change addressed to user defined type so user can control adress size (in general refactor int and size types)

    //leave defragmentation for a later update (will need to shift data around).