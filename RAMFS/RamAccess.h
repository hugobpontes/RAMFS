#pragma once

#include <cstddef>

class RamAccess {
  public:
  virtual void RamWrite(const void* const pData, const size_t size, const size_t address) const = 0;
  virtual void RamRead(void* const pData, const size_t size, const size_t address) const = 0;
  static constexpr size_t k_RamSize = 8192; /*Edit depending on your underlying RAM*/
  protected: 
  bool CheckRamAccessParameters  (const void* const pData, const size_t size, const size_t address) const; 
};