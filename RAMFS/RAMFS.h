#pragma once

#include <cstddef>
#include <cstring>


#include "RamAccess.h"
#include "RAMFSFile.h"
#include "RAMFSFragment.h"

enum class RAMFS_Status {
  SUCCESS,
  INVALID_FILENAME,
  FILE_SLOTS_FULL,
  FILE_NOT_FOUND,
};
template <size_t FileNr = 10, size_t FragmentNr = 10>
class RAMFS {
 
 public:
  RAMFS(const size_t ramSize, const RamAccess& RamAccess);
  bool operator==(const RAMFS& other) const;
  size_t GetUsableSize() const;
  size_t GetRawFileSystemSize() const;
  void GetRawFileSystem(void* const pData, const size_t size) const;
  void _TempFileEdit_();
  bool WasLoaded() const;
  bool IsInitialized() const;
  RAMFS_Status CreateFile(const char* const& fname, RamFsFile*& pFile, const Timestamp time);
  RAMFS_Status FindFile(const char* const& fname, RamFsFile*& pFile);
  unsigned short GetFileCount() const;

 private:
  RAMFS() = default;
  void LoadFsFromRam();
  void StoreFsInRam() const;
  bool CheckFileSystem() const;

  const RamAccess& m_ramAccess;
  bool m_wasLoaded = false;
  bool m_isInitialized = false;

  struct FileSystem {
    size_t m_usableSize = 0;
    size_t m_ramSize = 0;
    RamFsFragment m_Fragments[FragmentNr] {};
    RamFsFile m_Files[FileNr]{};
    unsigned short m_FileCount = 0;
  } m_FileSystem;
};

template <size_t FileNr, size_t FragmentNr>
RAMFS<FileNr, FragmentNr>::RAMFS(const size_t ramSize, const RamAccess& RamAccess)
    : m_ramAccess(RamAccess) {
  if (ramSize<=RamAccess::k_RamSize && ramSize >= sizeof(m_FileSystem)) {
    LoadFsFromRam();
    if (!CheckFileSystem() || m_FileSystem.m_ramSize != ramSize) {
      m_FileSystem.m_ramSize = ramSize;
      m_FileSystem.m_usableSize = ramSize - sizeof(m_FileSystem);
      StoreFsInRam();
    } else {
      m_wasLoaded = true;
    }
    m_isInitialized = true;
  }
}
template <size_t FileNr, size_t FragmentNr>
size_t RAMFS<FileNr, FragmentNr>::GetUsableSize() const {
  return m_FileSystem.m_usableSize;
}

template <size_t FileNr, size_t FragmentNr>
size_t RAMFS<FileNr, FragmentNr>::GetRawFileSystemSize() const {
  return sizeof(m_FileSystem);
}

template <size_t FileNr, size_t FragmentNr>
void RAMFS<FileNr, FragmentNr>::GetRawFileSystem(void* const pData, size_t size) const {
  if (size > sizeof(m_FileSystem)) {
    size = sizeof(m_FileSystem);
  }
  memcpy(pData, &m_FileSystem, size);
}

template <size_t FileNr, size_t FragmentNr>
void RAMFS<FileNr, FragmentNr>::LoadFsFromRam() {
  m_ramAccess.RamRead(&m_FileSystem, sizeof(m_FileSystem), 0);
}

template <size_t FileNr, size_t FragmentNr>
void RAMFS<FileNr, FragmentNr>::StoreFsInRam() const {
  //TODO: Add selective write, maybe in a different function, so not all fs is re-written in ram everytime a byte is written
  m_ramAccess.RamWrite(&m_FileSystem, sizeof(m_FileSystem), 0);
}

template <size_t FileNr, size_t FragmentNr>
bool RAMFS<FileNr, FragmentNr>::CheckFileSystem() const {
  return (m_FileSystem.m_usableSize ==  m_FileSystem.m_ramSize - sizeof(m_FileSystem));
  /*For now this is the only way to check a correct load, consider adding a signature if nothing else coomes to mind later*/
}

template <size_t FileNr, size_t FragmentNr>
bool RAMFS<FileNr, FragmentNr>::operator==(const RAMFS& other) const{
  return !memcmp(&m_FileSystem, &(other.m_FileSystem), sizeof(m_FileSystem));
}

template <size_t FileNr, size_t FragmentNr>
void RAMFS<FileNr, FragmentNr>::_TempFileEdit_(){
  m_FileSystem.m_Files[0].dummy=99;
  StoreFsInRam();
}

template <size_t FileNr, size_t FragmentNr>
bool RAMFS<FileNr, FragmentNr>::WasLoaded() const {
  return m_wasLoaded;
}
template <size_t FileNr, size_t FragmentNr>
bool RAMFS<FileNr, FragmentNr>::IsInitialized() const {
  return m_isInitialized;
}
template <size_t FileNr, size_t FragmentNr>
RAMFS_Status RAMFS<FileNr, FragmentNr>::CreateFile(const char* const& fname, RamFsFile*& pFile, const Timestamp time) {
  RAMFS_Status status = RAMFS_Status::FILE_SLOTS_FULL;
  size_t filename_length = strlen(fname);
  if (filename_length <= 0 || filename_length >= k_FilenameMaxSize) { 
    status = RAMFS_Status::INVALID_FILENAME;
    pFile = nullptr;
  } else {
    for (int i = 0; i < FileNr; i++) {
      if (m_FileSystem.m_Files[i].m_isActive == false){
        memcpy(&(m_FileSystem.m_Files[i].m_filename), fname, filename_length);
        m_FileSystem.m_Files[i].m_timestamp=time;
        m_FileSystem.m_Files[i].m_isActive = true;
        m_FileSystem.m_FileCount++;
        pFile = &(m_FileSystem.m_Files[i]);
        status = RAMFS_Status::SUCCESS;
        break;
      } 
    }
  }
  return status;
}
template <size_t FileNr, size_t FragmentNr>
RAMFS_Status RAMFS<FileNr, FragmentNr>::FindFile(const char* const& fname, RamFsFile*& pFile) {
  RAMFS_Status status = RAMFS_Status::FILE_NOT_FOUND;
  size_t filename_length = strlen(fname);
  if (filename_length <= 0 || filename_length >= k_FilenameMaxSize) {
    pFile = nullptr;
    status = RAMFS_Status::INVALID_FILENAME;
  } else {
    for (int i = 0; i < FileNr; i++) {
      if (!(strcmp(m_FileSystem.m_Files[i].m_filename,fname))) {
        pFile = &(m_FileSystem.m_Files[i]);
        status = RAMFS_Status::SUCCESS;
        break;
      }
    }
  }
  return status;
}
template <size_t FileNr, size_t FragmentNr>
unsigned short RAMFS<FileNr, FragmentNr>::GetFileCount() const {
  return m_FileSystem.m_FileCount;
}
