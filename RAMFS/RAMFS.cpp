#include "RAMFS.h"

bool RamAccess::CheckRamAccessParameters (void* pData, size_t size, size_t address){
return (pData != nullptr && size <= k_RamSize && address < k_RamSize);
}