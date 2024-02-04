#include <cstddef>

class RamAccess {
    virtual void RamWrite(void* pData, size_t size) = 0;
    virtual void RamRead(void* pData, size_t size) = 0 ;
};