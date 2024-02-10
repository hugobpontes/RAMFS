#pragma once

#include "RamFs.h"

constexpr int k_FilenameMaxSize = 20;

using Filename = char[k_FilenameMaxSize];
using Timestamp = unsigned long;

/*Needs forward declaration so file system class can be its friend*/
template <size_t FileNr, size_t FragmentNr>
class RamFs;

class RamFsFile {
 public:
  template <size_t FileNr, size_t FragmentNr>
  friend class RamFs;

 private: 
  Filename m_filename;
  Timestamp m_timestamp;
  bool m_isActive = false;
  char dummy;
};
