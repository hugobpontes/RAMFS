#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <fstream>
#include <iostream>

#include "RamFs.h"
#include "RamAccessFile.h"

#include <stdlib.h>

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

  size_t StorableFileSystemSize = RamFs::GetStorableParamsSize();

  std::string ErrorString = "File System Size over limit("+std::to_string(k_acceptableFsSize)+"): " + std::to_string(StorableFileSystemSize);

  CHECK_TEXT(StorableFileSystemSize < k_acceptableFsSize, ErrorString.data());
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

    RamFs CorrectlySizedFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    size_t FsSize = RamFs::GetStorableParamsSize();

    RamFs TooSmallFileSystem(FsSize - 1, RamFileEmulator);

    CHECK(!TooSmallFileSystem.IsInitialized());
    CHECK(CorrectlySizedFileSystem.IsInitialized());
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

    CHECK_EQUAL(pMyFile1,pMyFile2);
    CHECK_EQUAL(MyFileSystem.GetFileCount(),1);
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
    creation_status2 = MyFileSystem.CreateFile("b", pMyFile2, 110); //also tests that single char names work
    find_status1 = MyFileSystem.FindFile("a.txt", pMyFile3);
    find_status2 = MyFileSystem.FindFile("b", pMyFile4);

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

      RamFsFile* pMyFile1;
      RamFsFile* pMyFile2;
      RamFsFile* pMyFile3;
      RamFsFile* pMyFile4;
      RamFsFile* pMyFile5;
      RamFsFile* pMyFile6;
      RamFs_Status creation_status1;
      RamFs_Status creation_status2;
      RamFs_Status creation_status3;
      RamFs_Status find_status1;
      RamFs_Status find_status2;
      RamFs_Status find_status3;

      creation_status1 = MyFileSystem.CreateFile("", pMyFile1, 100);
      creation_status2 = MyFileSystem.CreateFile("filename_is_long.txt", pMyFile2, 100);
      creation_status3 = MyFileSystem.CreateFile(nullptr, pMyFile3, 100);

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

      RamFsFile* pMyFile;
      RamFs_Status creation_status;
      RamFs_Status find_status;

      creation_status = MyFileSystem.CreateFile("file.txt", pMyFile, 100);
      find_status = MyFileSystem.FindFile("doesnt_exist.txt", pMyFile);

      CHECK(creation_status == RamFs_Status::SUCCESS);
      CHECK(find_status == RamFs_Status::FILE_NOT_FOUND);
      CHECK_EQUAL(pMyFile,nullptr);
    }

    TEST(TestFileCreationAndFind, FileSlotsFull) {
      RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

      RamFsFile* pMyFile;
      RamFs_Status creation_status;

      for (int i = 0; i < 10; i++) {
        creation_status = MyFileSystem.CreateFile(("name" + std::to_string(i)).data(), pMyFile, 100);
        CHECK(creation_status == RamFs_Status::SUCCESS);
      }
      creation_status = MyFileSystem.CreateFile(("name" + std::to_string(10)).data(), pMyFile, 100);
      CHECK(creation_status == RamFs_Status::FILE_SLOTS_FULL);
      CHECK_EQUAL(pMyFile, nullptr);
    }

    TEST(TestFileCreationAndFind, FileAlreadyExists) {
      RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

      RamFsFile* pMyFile1;
      RamFsFile* pMyFile2;
      RamFs_Status creation_status1;
      RamFs_Status creation_status2;

      Timestamp t_creation = 100;

      creation_status1 = MyFileSystem.CreateFile("file.txt", pMyFile1, t_creation);
      creation_status2 = MyFileSystem.CreateFile("file.txt", pMyFile2,t_creation + 100);  // timestamp could also have been 100

      CHECK(creation_status1 == RamFs_Status::SUCCESS);
      CHECK(creation_status2 == RamFs_Status::SUCCESS);
      CHECK_EQUAL(pMyFile2,pMyFile1);
      CHECK_EQUAL(MyFileSystem.GetFileCount(), 1);
      /*Timestamp should be unaltered when creating a file already created*/
      CHECK_EQUAL(pMyFile1->GetCreationTimestamp(),pMyFile2->GetCreationTimestamp());
    }

    TEST_GROUP(TestFileWriteRead){void setup(){RamFileEmulator.RamClear();
  }
  void teardown() {

  }
};

  TEST(TestFileWriteRead, BasicWriteAndRead) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    std::string filename = "file.txt";
    Timestamp t_creation = 100;

    RamFsFile* pMyFile;
    RamFs_Status write_status;
    RamFs_Status read_status;

    uint8_t WriteData [] = {1,2,3,4,5,6,7};
    uint8_t ReadData[sizeof(WriteData)] = {};

    MyFileSystem.CreateFile(filename.data(), pMyFile, t_creation);
    write_status = pMyFile->Write(WriteData,sizeof(WriteData),t_creation+10);
    size_t free_size_after_writing = MyFileSystem.GetFreeSize();
    size_t file_size_after_writing = pMyFile->GetSize();

    RamFs MyFileSystem2(RamAccess::k_RamSize, RamFileEmulator);
    MyFileSystem2.FindFile(filename.data(), pMyFile);
    read_status = pMyFile->Read(ReadData, pMyFile->GetSize(), 0);

    CHECK(write_status == RamFs_Status::SUCCESS);
    CHECK(read_status == RamFs_Status::SUCCESS);
    CHECK_EQUAL(file_size_after_writing,sizeof(WriteData));
    CHECK_EQUAL(free_size_after_writing,RamAccess::k_RamSize - RamFs::GetStorableParamsSize() - sizeof(WriteData));
    MEMCMP_EQUAL(WriteData,ReadData,sizeof(WriteData));
  }

    TEST(TestFileWriteRead, MultipleWriteAndRead) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    std::string filename1 = "file1.txt";
    std::string filename2 = "file2.txt";
    std::string filename3 = "file3.txt";
    Timestamp t_creation1 = 100;
    Timestamp t_creation2 = 200;
    Timestamp t_creation3 = 300;

    RamFsFile* pMyFile1;
    RamFsFile* pMyFile2;
    RamFsFile* pMyFile3;
    RamFsFile* pMyFile4;
    RamFsFile* pMyFile5;
    RamFs_Status write_status1;
    RamFs_Status read_status1;
    RamFs_Status write_status2;
    RamFs_Status read_status2;
    RamFs_Status write_status3;
    RamFs_Status read_status3;

    uint8_t WriteData1 [] = {1,2,3,4,5,6,7};
    uint8_t WriteData2 [] = {7,6,5,4,3,2,1,0,1};
    uint8_t WriteData3[] = {17, 16, 15};
    uint8_t ReadData1[sizeof(WriteData1)] = {};
    uint8_t ReadData2[sizeof(WriteData2)] = {};
    uint8_t ReadData3[sizeof(WriteData3)] = {};

    MyFileSystem.CreateFile(filename1.data(), pMyFile1, t_creation1);
    write_status1 = pMyFile1->Write(WriteData1,sizeof(WriteData1),t_creation1+10);
    size_t free_size_after_writing1 = MyFileSystem.GetFreeSize();
    size_t file_size_after_writing1 = pMyFile1->GetSize();

    RamFs MyFileSystem2(RamAccess::k_RamSize, RamFileEmulator);
    MyFileSystem2.FindFile(filename1.data(), pMyFile2);
    read_status1 = pMyFile2->Read(ReadData1, pMyFile2->GetSize(), 0);
    MyFileSystem2.CreateFile(filename2.data(), pMyFile3, t_creation2);
    write_status2 = pMyFile3->Write(WriteData2, sizeof(WriteData2), t_creation2 + 10);
    size_t free_size_after_writing2 = MyFileSystem2.GetFreeSize();
    size_t file_size_after_writing2 = pMyFile3->GetSize();

    RamFs MyFileSystem3(RamAccess::k_RamSize, RamFileEmulator);
    MyFileSystem3.FindFile(filename2.data(), pMyFile4);
    read_status2 = pMyFile4->Read(ReadData2, pMyFile4->GetSize(), 0);

    MyFileSystem3.CreateFile(filename3.data(), pMyFile5, t_creation3);
    write_status3 = pMyFile5->Write(WriteData3,sizeof(WriteData3),t_creation3+10);
    read_status3 = pMyFile5->Read(ReadData3, sizeof(WriteData3), 0);
    size_t free_size_after_writing3 = MyFileSystem3.GetFreeSize();
    size_t file_size_after_writing3 = pMyFile5->GetSize();

    CHECK(write_status1 == RamFs_Status::SUCCESS);
    CHECK(read_status1 == RamFs_Status::SUCCESS);
    CHECK(write_status2 == RamFs_Status::SUCCESS);
    CHECK(read_status2 == RamFs_Status::SUCCESS);
    CHECK(write_status3 == RamFs_Status::SUCCESS);
    CHECK(read_status3 == RamFs_Status::SUCCESS);
    CHECK_EQUAL(file_size_after_writing1,sizeof(WriteData1));
    CHECK_EQUAL(file_size_after_writing2,sizeof(WriteData2));
    CHECK_EQUAL(file_size_after_writing3,sizeof(WriteData3));
    CHECK_EQUAL(free_size_after_writing1,RamAccess::k_RamSize - RamFs::GetStorableParamsSize() - sizeof(WriteData1));
    CHECK_EQUAL(free_size_after_writing2,RamAccess::k_RamSize - RamFs::GetStorableParamsSize() - sizeof(WriteData1) - sizeof(WriteData2));
    CHECK_EQUAL(free_size_after_writing3,RamAccess::k_RamSize - RamFs::GetStorableParamsSize() - sizeof(WriteData1) - sizeof(WriteData2)- sizeof(WriteData3));
    MEMCMP_EQUAL(WriteData1,ReadData1,sizeof(WriteData1));
    MEMCMP_EQUAL(WriteData2,ReadData2,sizeof(WriteData2));
    MEMCMP_EQUAL(WriteData3, ReadData3,sizeof(WriteData3));
  }

  TEST(TestFileWriteRead, WriteToSameFileTwice) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    std::string filename = "file1.txt";
    Timestamp t_creation = 100;
    RamFsFile* pMyFile;
    RamFs_Status write_status1;
    RamFs_Status write_status2;
    RamFs_Status read_status1;
    RamFs_Status read_status2;

    uint8_t WriteData1[] = {1, 2, 3, 4, 5, 6, 7};
    uint8_t WriteData2[] = {7, 6, 5, 4, 3, 2, 1, 0, 1};
    uint8_t ReadData1[sizeof(WriteData1)] = {};
    uint8_t ReadData2[sizeof(WriteData2)] = {};

    Timestamp creation_time;
    Timestamp modif_time1;
    Timestamp modif_time2;

    MyFileSystem.CreateFile(filename.data(), pMyFile, t_creation);

    write_status1 = pMyFile->Write(WriteData1, sizeof(WriteData1), t_creation + 10);
    size_t free_size_after_writing1 = MyFileSystem.GetFreeSize();
    size_t file_size_after_writing1 = pMyFile->GetSize();
    read_status1 = pMyFile->Read(ReadData1, pMyFile->GetSize(), 0);
    creation_time = pMyFile->GetCreationTimestamp();
    modif_time1 = pMyFile->GetModificationTimestamp();

    write_status2 = pMyFile->Write(WriteData2, sizeof(WriteData2), t_creation + 20);
    size_t free_size_after_writing2 = MyFileSystem.GetFreeSize();
    size_t file_size_after_writing2 = pMyFile->GetSize();
    read_status2 = pMyFile->Read(ReadData2, pMyFile->GetSize(), 0);
    modif_time2 = pMyFile->GetModificationTimestamp();

    CHECK(write_status1 == RamFs_Status::SUCCESS);
    CHECK(write_status2 == RamFs_Status::SUCCESS);
    MEMCMP_EQUAL(WriteData1, ReadData1, sizeof(WriteData1));
    MEMCMP_EQUAL(WriteData2, ReadData2, sizeof(WriteData2));
    CHECK_EQUAL(file_size_after_writing1 ,sizeof(WriteData1));
    CHECK_EQUAL(free_size_after_writing1, RamAccess::k_RamSize - RamFs::GetStorableParamsSize() - sizeof(WriteData1));
    CHECK_EQUAL(file_size_after_writing2, sizeof(WriteData2));
    CHECK_EQUAL(free_size_after_writing2, RamAccess::k_RamSize - RamFs::GetStorableParamsSize() - sizeof(WriteData2));
    CHECK_EQUAL(creation_time, t_creation);
    CHECK_EQUAL(modif_time1, t_creation+10);
    CHECK_EQUAL(modif_time2,t_creation+20);

  }

void FillArrayWithBytes(uint8_t* Arr, size_t size){
  int j;
  for (int i = 0; i < size; i++) {
    Arr[i] = j++;
    j = (j > 255) ? 0 : j;
  }
}

  TEST(TestFileWriteRead, Write1ByteBeforeFullFs) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);
    RamFsFile* pMyFile;
    RamFs_Status read_status;
    RamFs_Status write_status;

    size_t writeSize = MyFileSystem.GetFreeSize()-1;
    uint8_t* WriteData = new uint8_t[writeSize];
    uint8_t* ReadData = new uint8_t[writeSize];

    FillArrayWithBytes(WriteData, writeSize);
    MyFileSystem.CreateFile("myfile.txt", pMyFile, 100);

    write_status = pMyFile->Write(WriteData, writeSize, 110);
    size_t free_size_after_writing = MyFileSystem.GetFreeSize();
    size_t file_size_after_writing = pMyFile->GetSize();
    read_status = pMyFile->Read(ReadData, pMyFile->GetSize(), 0);


    CHECK(write_status == RamFs_Status::SUCCESS);
    CHECK(read_status == RamFs_Status::SUCCESS);
    MEMCMP_EQUAL(WriteData, ReadData, writeSize);
    CHECK_EQUAL(file_size_after_writing,writeSize);
    CHECK_EQUAL(free_size_after_writing,RamAccess::k_RamSize - RamFs::GetStorableParamsSize() - writeSize);

    delete[] WriteData;
    delete[] ReadData;
  }
  TEST(TestFileWriteRead, NullPtrWrite) {
    RamFs MyFileSystem(RamAccess::k_RamSize, RamFileEmulator);

    RamFsFile* pMyFile;
    RamFs_Status write_status;

    MyFileSystem.CreateFile("file.txt", pMyFile, 300);

    write_status = pMyFile->Write(nullptr, 10, 400);
    size_t free_size_after_writing = MyFileSystem.GetFreeSize();
    size_t file_size_after_writing = pMyFile->GetSize();

    CHECK(write_status == RamFs_Status::NULL_POINTER);
    CHECK(free_size_after_writing ==
          RamAccess::k_RamSize - RamFs::GetStorableParamsSize());
    CHECK(file_size_after_writing == 0);
  }

    //test writing data size that doesnt fit, or size 0
    //test invalid pointer
    //test all return messages (includi)
    //test with full file slots
    //test too fragmented files (>3)
    //ignore test with fragmented files (needs delete), this includes checking status
    //ignore test with defragmentation (needs delete),this includes checking status

    //test deletion, appending, etc.. (deletion is essentially what is done in the beginning of write)
    //test all return messages
    //test something that would onnly be possible with defragmentation

    /* test that one can write to a file (TIMESTAMP,and variants) test that one can read from a file (and variants) test that one can get fs free size test that one can get file size test that one can get file timestamp test that one can get filename only when all of the above are done, do we think about appending,deleting and variants that require multiple fragments add feature to only store parts of the filesystem ->Add non default size test when structures are stable add doxygen comments*/

    //change addressed to user defined type so user can control adress size (in general refactor int and size types)