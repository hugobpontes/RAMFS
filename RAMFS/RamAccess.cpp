#include "RAMFS.h"

bool RamAccess::CheckRamAccessParameters (const void* const pData, const size_t size, const size_t address) const {
    return (pData != nullptr && size <= k_RamSize && address < k_RamSize);
}