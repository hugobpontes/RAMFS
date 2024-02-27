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
size_t free_size1;
size_t free_size2;
size_t free_size3;
Timestamp modif_time1;
Timestamp modif_time2;
Timestamp modif_time3;
Timestamp creation_time1;
Timestamp creation_time2;
Timestamp creation_time3;
size_t file_size1;
size_t file_size2;
size_t file_size3;
size_t write_size1;
size_t write_size2;
size_t write_size3;
size_t read_size1;
size_t read_size2;
size_t read_size3;
size_t read_pos1;
size_t read_pos2;
size_t read_pos3;
size_t file_nr1;
size_t file_nr2;
size_t file_nr3;

uint8_t* WriteData;
uint8_t* ReadData;

/*one more than the usable file system space*/
int DataArraySize = RamAccess::k_RamSize;
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
  int j = 0;
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
  free_size1 = 0;
  free_size2 = 0;
  free_size3 = 0;
  file_size1 = 0;
  file_size2 = 0;
  file_size3 = 0;
  write_size1 = 0;
  write_size2 = 0;
  write_size3 = 0;
  read_size1 = 0;
  read_size2 = 0;
  read_size3 = 0;
  read_pos1 = 0;
  read_pos2 = 0;
  read_pos3 = 0;
  file_nr1 = 0;
  file_nr2 = 0;
  file_nr3 = 0;
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
    free_size1 = MyFileSystem.GetFreeSize();
    file_size1 = pMyFile1->GetSize();

    RamFs MyFileSystem2(RamAccess::k_RamSize, RamFileEmulator);
    MyFileSystem2.FindFile(fname1, pMyFile2);
    read_status1 = pMyFile2->Read(ReadData, pMyFile2->GetSize(), 0);

    CHECK(write_status1 == RamFs_Status::SUCCESS);
    CHECK(read_status1 == RamFs_Status::SUCCESS);
    CHECK_EQUAL(file_size1, write_size1);
    CHECK_EQUAL(free_size1,RamAccess::k_RamSize - RamFs::GetStorableParamsSize() - write_size1);
    MEMCMP_EQUAL(WriteData, ReadData, write_size1);
  }

    TEST(TestFileWriteRead, MultipleWriteAndRead) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    write_size1 = 7;
    write_size2 = 9;
    write_size3 = 3;

    MyFileSystem.CreateFile(fname1, pMyFile1, time1);
    write_status1 = pMyFile1->Write(WriteData,write_size1,time2);
    free_size1 = MyFileSystem.GetFreeSize();
    file_size1 = pMyFile1->GetSize();

    RamFs MyFileSystem2(RamAccess::k_RamSize, RamFileEmulator);
    MyFileSystem2.FindFile(fname1, pMyFile2);
    read_status1 = pMyFile2->Read(ReadData, pMyFile2->GetSize(), 0);
    MyFileSystem2.CreateFile(fname2, pMyFile3, time3);
    write_status2 = pMyFile3->Write(WriteData, write_size2, time4);
    free_size2 = MyFileSystem2.GetFreeSize();
    file_size2 = pMyFile3->GetSize();

    RamFs MyFileSystem3(RamAccess::k_RamSize, RamFileEmulator);
    MyFileSystem3.FindFile(fname2, pMyFile4);
    read_status2 = pMyFile4->Read(ReadData+write_size1, pMyFile4->GetSize(), 0);

    MyFileSystem3.CreateFile(fname3, pMyFile5, time5);
    write_status3 = pMyFile5->Write(WriteData,write_size3,time6);
    read_status3 = pMyFile5->Read(ReadData + write_size1+write_size2, write_size3, 0);
    free_size3 = MyFileSystem3.GetFreeSize();
    file_size3 = pMyFile5->GetSize();

    CHECK(write_status1 == RamFs_Status::SUCCESS);
    CHECK(read_status1 == RamFs_Status::SUCCESS);
    CHECK(write_status2 == RamFs_Status::SUCCESS);
    CHECK(read_status2 == RamFs_Status::SUCCESS);
    CHECK(write_status3 == RamFs_Status::SUCCESS);
    CHECK(read_status3 == RamFs_Status::SUCCESS);
    CHECK_EQUAL(file_size1,write_size1);
    CHECK_EQUAL(file_size2,write_size2);
    CHECK_EQUAL(file_size3,write_size3);
    CHECK_EQUAL(free_size1,RamAccess::k_RamSize - RamFs::GetStorableParamsSize() - write_size1);
    CHECK_EQUAL(free_size2,RamAccess::k_RamSize - RamFs::GetStorableParamsSize() - write_size1 - write_size2);
    CHECK_EQUAL(free_size3,RamAccess::k_RamSize - RamFs::GetStorableParamsSize() - write_size1 - write_size2- write_size3);
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
    free_size1 = MyFileSystem.GetFreeSize();
    file_size1 = pMyFile1->GetSize();
    read_status1 = pMyFile1->Read(ReadData, pMyFile1->GetSize(), 0);
    creation_time1 = pMyFile1->GetCreationTimestamp();
    modif_time1 = pMyFile1->GetModificationTimestamp();

    write_status2 = pMyFile1->Write(WriteData, write_size2, time2);
    free_size2 = MyFileSystem.GetFreeSize();
    file_size2 = pMyFile1->GetSize();
    read_status2 = pMyFile1->Read(ReadData+write_size1, pMyFile1->GetSize(), 0);
    modif_time2 = pMyFile1->GetModificationTimestamp();

    CHECK(write_status1 == RamFs_Status::SUCCESS);
    CHECK(write_status2 == RamFs_Status::SUCCESS);
    MEMCMP_EQUAL(WriteData, ReadData, write_size1);
    MEMCMP_EQUAL(WriteData, ReadData+write_size1, write_size2);
    CHECK_EQUAL(file_size1, write_size1);
    CHECK_EQUAL(free_size1, RamAccess::k_RamSize - RamFs::GetStorableParamsSize() - write_size1);
    CHECK_EQUAL(file_size2, write_size2);
    CHECK_EQUAL(free_size2, RamAccess::k_RamSize - RamFs::GetStorableParamsSize() - write_size2);
    CHECK_EQUAL(creation_time1, time1);
    CHECK_EQUAL(modif_time1, time1);
    CHECK_EQUAL(modif_time2,time2);

  }

  TEST(TestFileWriteRead, Write1ByteLessThanFillingFs) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    MyFileSystem.CreateFile(fname1, pMyFile1, time1);

    write_size1 = MyFileSystem.GetFreeSize() - 1;

    write_status1 = pMyFile1->Write(WriteData, write_size1, time2);
    free_size1 = MyFileSystem.GetFreeSize();
    file_size1 = pMyFile1->GetSize();
    read_status1 = pMyFile1->Read(ReadData, pMyFile1->GetSize(), 0);

    CHECK(write_status1 == RamFs_Status::SUCCESS);
    CHECK(read_status1 == RamFs_Status::SUCCESS);
    MEMCMP_EQUAL(WriteData, ReadData, write_size1);
    CHECK_EQUAL(file_size1, write_size1);
    CHECK_EQUAL(free_size1,RamAccess::k_RamSize - RamFs::GetStorableParamsSize() - write_size1);

  }

  TEST(TestFileWriteRead, FillFs) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    MyFileSystem.CreateFile(fname1, pMyFile1, time1);

    write_size1 = MyFileSystem.GetFreeSize();

    write_status1 = pMyFile1->Write(WriteData, write_size1, time2);
    free_size1 = MyFileSystem.GetFreeSize();
    file_size1 = pMyFile1->GetSize();
    read_status1 = pMyFile1->Read(ReadData, pMyFile1->GetSize(), 0);

    CHECK(write_status1 == RamFs_Status::SUCCESS);
    CHECK(read_status1 == RamFs_Status::SUCCESS);
    MEMCMP_EQUAL(WriteData, ReadData, write_size1);
    CHECK_EQUAL(file_size1, write_size1);
    CHECK_EQUAL(free_size1,RamAccess::k_RamSize - RamFs::GetStorableParamsSize() - write_size1);
  }

    TEST(TestFileWriteRead, OverflowFs) {
      RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

      MyFileSystem.CreateFile(fname1, pMyFile1, time1);

      write_size1 = MyFileSystem.GetFreeSize()+1;

      write_status1 = pMyFile1->Write(WriteData, write_size1, time2);
      free_size1 = MyFileSystem.GetFreeSize();
      file_size1 = pMyFile1->GetSize();

      CHECK(write_status1 == RamFs_Status::INSUFFICIENT_STORAGE);
      CHECK_EQUAL(free_size1,RamAccess::k_RamSize - RamFs::GetStorableParamsSize());
      CHECK_EQUAL(file_size1, 0);
  }

  TEST(TestFileWriteRead, NullPtrWrite) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    MyFileSystem.CreateFile("file.txt", pMyFile1, time1);

    write_size1 = 10;
    write_status1 = pMyFile1->Write(nullptr, write_size1, 400);
    free_size1 = MyFileSystem.GetFreeSize();
    file_size1 = pMyFile1->GetSize();

    CHECK(write_status1 == RamFs_Status::NULL_POINTER);
    CHECK(free_size1 == RamAccess::k_RamSize - RamFs::GetStorableParamsSize());
    CHECK(file_size1 == 0);
  }

  TEST(TestFileWriteRead, Write0Bytes) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    write_size1 = 0;

    MyFileSystem.CreateFile(fname1, pMyFile1, time1);

    write_status1 = pMyFile1->Write(WriteData, write_size1, time1);
    free_size1 = MyFileSystem.GetFreeSize();
    file_size1 = pMyFile1->GetSize();

    CHECK(write_status1 == RamFs_Status::SUCCESS);
    CHECK_EQUAL(free_size1,RamAccess::k_RamSize - RamFs::GetStorableParamsSize());
    CHECK_EQUAL(file_size1, 0);

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

TEST_GROUP(TestDelete){
  void setup(){
    defaultSetup();
  }
  void teardown(){
    defaultTeardown();
  }
};

TEST(TestDelete, BasicDelete) {
  RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);
  MyFileSystem.CreateFile(fname1, pMyFile1, time1);
  MyFileSystem.CreateFile(fname2, pMyFile2, time1);

  write_size1 = 10;
  write_size2 = 20;

  pMyFile1->Write(WriteData, write_size1, time2);
  pMyFile2->Write(WriteData, write_size2, time3);

  file_nr1 = MyFileSystem.GetFileCount();
  free_size1 = MyFileSystem.GetFreeSize();

  pMyFile1->Delete();

  file_nr2 = MyFileSystem.GetFileCount();
  free_size2 = MyFileSystem.GetFreeSize();

  find_status1 = MyFileSystem.FindFile(fname1,pMyFile3);

  CHECK_EQUAL(file_nr1 - file_nr2, 1);
  CHECK_EQUAL(free_size2 - free_size1, write_size1);
  CHECK(find_status1 == RamFs_Status::FILE_NOT_FOUND);
}

TEST(TestDelete, Use11thFile) {
  RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

  write_size1 = 5;

  for (int i = 0; i < k_FileNr; i++) {
    creation_status1 = MyFileSystem.CreateFile(("name" + std::to_string(i)).data(), pMyFile1, time1);
    CHECK(creation_status1 == RamFs_Status::SUCCESS);
  }
  file_nr1 = MyFileSystem.GetFileCount();
  pMyFile1->Delete();
  file_nr2 = MyFileSystem.GetFileCount();
  creation_status2 = MyFileSystem.CreateFile(fname1, pMyFile2, time2);
  file_nr3 = MyFileSystem.GetFileCount();
  write_status1 = pMyFile2->Write(WriteData, write_size1, time3);

  RamFs MyFileSystem2(RamAccess::k_RamSize, RamFileEmulator);
  find_status1 = MyFileSystem2.FindFile(fname1,pMyFile3);
  read_status1 = pMyFile3->Read(ReadData, write_size1, 0);
  CHECK(creation_status2 == RamFs_Status::SUCCESS);
  CHECK(find_status1 == RamFs_Status::SUCCESS);
  CHECK(write_status1 == RamFs_Status::SUCCESS);
  CHECK(read_status1 == RamFs_Status::SUCCESS);
  MEMCMP_EQUAL(WriteData, ReadData, write_size1);
}

TEST(TestDelete, FragmentationByDelete) {
  RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

  /*This tests if fragmentation works after a fragmentation is caused by a delete
  To that end, a 20 byte file needs to be written in a file system that doesnt have 20 continous free bytes
  To achieve that we create a 5 byte file and then a file with size only 20 bytes less the fs storage size
  when we delete the original file we are left with two free spaces: a 5 byte block that used to
  belong to the first file, and a 15 byte free block after the large file.
  We we try to write 20 bytes now, that file will have to be fragmented*/

  write_size1 = 5;
  write_size3 = 20;
  write_size2 = MyFileSystem.GetFreeSize()-write_size3; 
  //size2 is size required to only leave size3-size1 contiguous bytes
  //|5B|Large block|(20-5)B|

  MyFileSystem.CreateFile(fname1, pMyFile1, time1);
  MyFileSystem.CreateFile(fname2, pMyFile2, time1);
  MyFileSystem.CreateFile(fname3, pMyFile3, time1);

  write_status1 = pMyFile1->Write(WriteData, write_size1, time2);
  write_status2 = pMyFile2->Write(WriteData+100, write_size2, time2);
  delete_status1 = pMyFile1->Delete();
  write_status3 = pMyFile3->Write(WriteData+200, write_size3, time3);
  read_status1 = pMyFile3->Read(ReadData, write_size3,0);

  CHECK(write_status3 == RamFs_Status::SUCCESS);
  MEMCMP_EQUAL(WriteData+200, ReadData, write_size3);
  CHECK_EQUAL(MyFileSystem.GetFreeSize(),0);
  CHECK_EQUAL(pMyFile3->GetSize(), write_size3);

}

TEST(TestDelete, FragmentationByDelete_StartReadInOtherFrag) {

  /*This tests that starting to read a file after its first
  fragment works*/


  RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

  write_size1 = 5;
  write_size3 = 20;
  write_size2 = MyFileSystem.GetFreeSize() - write_size3;

  read_pos1 = 16;
  read_size1 = 3;

  MyFileSystem.CreateFile(fname1, pMyFile1, time1);
  MyFileSystem.CreateFile(fname2, pMyFile2, time1);
  MyFileSystem.CreateFile(fname3, pMyFile3, time1);

  write_status1 = pMyFile1->Write(WriteData, write_size1, time2);
  write_status2 = pMyFile2->Write(WriteData + 100, write_size2, time2);
  delete_status1 = pMyFile1->Delete();
  write_status3 = pMyFile3->Write(WriteData + 200, write_size3, time3);
  read_status1 = pMyFile3->Read(ReadData, read_size1, read_pos1);

  MEMCMP_EQUAL(WriteData + 200+read_pos1, ReadData, read_size1);
}

TEST(TestDelete, FragmentationByDelete_UnfilledFsAndLoad) {
  RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

  write_size1 = 5;
  write_size3 = 20;
  int margin = 2;
  write_size2 = MyFileSystem.GetFreeSize() - write_size3 - margin;
  // size2 is size required to only size3-size1+margin contiguous bytes
  //|5B|Large block|(20-5+2)B|

  MyFileSystem.CreateFile(fname1, pMyFile1, time1);
  MyFileSystem.CreateFile(fname2, pMyFile2, time1);
  MyFileSystem.CreateFile(fname3, pMyFile3, time1);

  write_status1 = pMyFile1->Write(WriteData, write_size1, time2);
  write_status2 = pMyFile2->Write(WriteData + 100, write_size2, time2);
  delete_status1 = pMyFile1->Delete();
  write_status3 = pMyFile3->Write(WriteData + 300, write_size3, time3);

  RamFs MyFileSystem2(RamAccess::k_RamSize, RamFileEmulator);
  MyFileSystem2.FindFile(fname3,pMyFile4);
  read_status1 = pMyFile4->Read(ReadData, write_size3, 0);

  CHECK(write_status3 == RamFs_Status::SUCCESS);
  MEMCMP_EQUAL(WriteData + 300, ReadData, write_size3);
  CHECK_EQUAL(MyFileSystem2.GetFreeSize(), margin);
  CHECK_EQUAL(pMyFile3->GetSize(), write_size3);

}

TEST(TestDelete, FragmentationByDelete_TooFragmentedWrite) {  
  
  /*For this test we want to fragment the file system as such:
  |5B|XB|5B|XB|5B|XB|5B and then delete the 5B files and then try to create a 20B file.
  Since this requires more fragments (4) than a file can hold (3), an error message should be 
  returned when writing.
  Additionally, the file should be reset to its original state. */

  constexpr int kTotalFiles = 7;

  RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

  write_size1 = 20;
  free_size1 = MyFileSystem.GetFreeSize();
  size_t small_file_size = 5;
  size_t big_file_size = (free_size1 - write_size1)/3;

  for (int i = 0; i < kTotalFiles; i++) {
    creation_status1 = MyFileSystem.CreateFile(("name" + std::to_string(i)).data(), pMyFile1, time1);
    CHECK(creation_status1 == RamFs_Status::SUCCESS);
    if (i%2 == 0){ //index of small files as decribed in the beginning of this test
      pMyFile1->Write(WriteData,small_file_size,time1+i);
    } else { 
      pMyFile1->Write(WriteData,big_file_size,time1+i);
    }
  }

  for (int i = 0; i < kTotalFiles; i++) {
    find_status1 = MyFileSystem.FindFile(("name" + std::to_string(i)).data(), pMyFile1);
    CHECK(find_status1 == RamFs_Status::SUCCESS);
    if (i%2 == 0){
      pMyFile1->Delete();
    }
  }

  creation_status2 = MyFileSystem.CreateFile(fname1, pMyFile2, time3);
  write_status1 = pMyFile2->Write(WriteData, 2, time5); //this is just to show later that file wasnt altered
  file_size1 = pMyFile2->GetSize();
  free_size2 = MyFileSystem.GetFreeSize();
  write_status2 = pMyFile2->Write(WriteData, write_size1, time6);
  file_size2 = pMyFile2->GetSize();
  free_size3 = MyFileSystem.GetFreeSize();

  CHECK(creation_status2 == RamFs_Status::SUCCESS);
  CHECK(write_status1 == RamFs_Status::SUCCESS);
  CHECK(write_status2 == RamFs_Status::FILE_TOO_FRAGMENTED);
  CHECK_EQUAL(file_size1,file_size2);
  CHECK_EQUAL(free_size2, free_size3);
}
  //->test too fragmented file (same as above but two large files leading to more fragments than the max(Also Check status))

  //->test appending to 1 byte less than full, full, over full (like we did for
  // normal writing)

//->same tests for write as for append pretty much
//->test no more frag slots left in file for append
//->test no more fragments slots available in fs even though files have the appropriate amount of
//frags (will i need to fragment 10 files???? maybe i better wait for appending)
//->repeat fragmentation tests but fragmente with append instead of delete

//->test filename get

//->general refactoring: clean up ugly code, remove repetitions, try to create
//helper functions...
//->improvements: hash table filename lookup, partial fs store, check ther to
//do's
//->create flexible types, check type coherence (for addresses, for indexes,
//etc...)
//->check const correctness of everything
//->add doxygen comments

// leave defragmentation for a later update (will need to shift data around).