#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "RAMFS.h"

class RamAccessFile : public RamAccess {

}

TEST_GROUP(TestRamAccessFile) {
  void setup() {}

  void teardown() {}
};

TEST(TestRamAccessFile, FirstTest) {
  int ret;
  ret = testfunc();
  CHECK (ret == 1);
}