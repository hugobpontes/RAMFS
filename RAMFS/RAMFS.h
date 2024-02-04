#include <cstddef>

#define RAM_SIZE 512

class RamAccess {
  public:
  virtual void RamWrite(void* pData, size_t size, size_t address) = 0;
  virtual void RamRead(void* pData, size_t size, size_t address) = 0;
};