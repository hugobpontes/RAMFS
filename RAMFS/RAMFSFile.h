#pragma once

#include "RAMFS.h"

constexpr int k_FilenameMaxSize = 20;

using Filename = char[k_FilenameMaxSize];
using Timestamp = unsigned long;

template <size_t FileNr, size_t FragmentNr>
class RAMFS;

class RamFsFile {
 public:
  template <size_t FileNr, size_t FragmentNr>
  friend class RAMFS;

 private: 
  Filename m_filename;
  Timestamp m_timestamp;
  bool m_isActive = false;
  char dummy;
};
