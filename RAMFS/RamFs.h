#pragma once

#include <cstddef>
#include <cstring>


#include "RamAccess.h"

constexpr int k_MaxFilenameSize = 20;
constexpr int k_MaxFragmentPerFile = 3;

using Filename = char[k_MaxFilenameSize];
using Timestamp = unsigned long;

enum class RamFs_Status {
  SUCCESS,
  INVALID_FILENAME,
  FILE_SLOTS_FULL,
  FILE_NOT_FOUND,
};

class RamFsFile;
class RamFsFragment;

template <size_t FileNr = 10, size_t FragmentNr = 10>
class RamFs {
 
 public:
  RamFs(const size_t ramSize, const RamAccess& RamAccess);
  bool operator==(const RamFs& other) const;
  size_t GetUsableSize() const;
  size_t GetFreeSize() const;
  size_t GetStorableFileSystemSize() const;
  void GetStorableFileSystem(void* const pData, const size_t size) const;
  void _TempFileEdit_();
  bool WasLoaded() const;
  bool IsInitialized() const;
  RamFs_Status CreateFile(const char* const& fname, RamFsFile*& pFile, const Timestamp creation_time);
  RamFs_Status FindFile(const char* const& fname, RamFsFile*& pFile);
  unsigned short GetFileCount() const;

 private:
  RamFs() = default;
  void LoadFsFromRam();
  void StoreFsInRam() const;
  bool CheckFileSystem() const;

  const RamAccess& m_ramAccess;
  bool m_wasLoaded = false;
  bool m_isInitialized = false;

  struct StorableFileSystem {
    size_t m_freeSize = 0;
    size_t m_ramSize = 0;
    RamFsFragment m_Fragments[FragmentNr] {};
    RamFsFile m_Files[FileNr]{};
    unsigned short m_FileCount = 0;
  } m_FSys;
};

template <size_t FileNr, size_t FragmentNr>
RamFs<FileNr, FragmentNr>::RamFs(const size_t ramSize, const RamAccess& RamAccess)
    : m_ramAccess(RamAccess) {
  if (ramSize<=RamAccess::k_RamSize && ramSize >= sizeof(m_FSys)) {
    LoadFsFromRam();
    if (!CheckFileSystem() || m_FSys.m_ramSize != ramSize) {
      m_FSys.m_ramSize = ramSize;
      m_FSys.m_freeSize = ramSize - sizeof(m_FSys);
      StoreFsInRam();
    } else {
      m_wasLoaded = true;
    }
    m_isInitialized = true;
  }
}
template <size_t FileNr, size_t FragmentNr>
size_t RamFs<FileNr, FragmentNr>::GetUsableSize() const {
  return m_FSys.m_ramSize - sizeof(m_FSys);
}

template <size_t FileNr, size_t FragmentNr>
size_t RamFs<FileNr, FragmentNr>::GetFreeSize() const {
  return m_FSys.m_freeSize;
}

template <size_t FileNr, size_t FragmentNr>
size_t RamFs<FileNr, FragmentNr>::GetStorableFileSystemSize() const {
  return sizeof(m_FSys);
}

template <size_t FileNr, size_t FragmentNr>
void RamFs<FileNr, FragmentNr>::GetStorableFileSystem(void* const pData, size_t size) const {
  if (size > sizeof(m_FSys)) {
    size = sizeof(m_FSys);
  }
  memcpy(pData, &m_FSys, size);
}

template <size_t FileNr, size_t FragmentNr>
void RamFs<FileNr, FragmentNr>::LoadFsFromRam() {
  m_ramAccess.RamRead(&m_FSys, sizeof(m_FSys), 0);
}

template <size_t FileNr, size_t FragmentNr>
void RamFs<FileNr, FragmentNr>::StoreFsInRam() const {
  //TODO: Add selective write, maybe in a different function, so not all fs is re-written in ram everytime a byte is written
  m_ramAccess.RamWrite(&m_FSys, sizeof(m_FSys), 0);
}

template <size_t FileNr, size_t FragmentNr>
bool RamFs<FileNr, FragmentNr>::CheckFileSystem() const {
  return (m_FSys.m_freeSize <=  m_FSys.m_ramSize - sizeof(m_FSys));
  /*For now this is the only way to check a correct load, consider adding a signature if nothing else coomes to mind later*/
}

template <size_t FileNr, size_t FragmentNr>
bool RamFs<FileNr, FragmentNr>::operator==(const RamFs& other) const{
  return !memcmp(&m_FSys, &(other.m_FSys), sizeof(m_FSys));
}

template <size_t FileNr, size_t FragmentNr>
void RamFs<FileNr, FragmentNr>::_TempFileEdit_(){
  m_FSys.m_Files[0].dummy=99;
  StoreFsInRam();
}

template <size_t FileNr, size_t FragmentNr>
bool RamFs<FileNr, FragmentNr>::WasLoaded() const {
  return m_wasLoaded;
}
template <size_t FileNr, size_t FragmentNr>
bool RamFs<FileNr, FragmentNr>::IsInitialized() const {
  return m_isInitialized;
}
template <size_t FileNr, size_t FragmentNr>
RamFs_Status RamFs<FileNr, FragmentNr>::CreateFile(const char* const& fname, RamFsFile*& pFile, const Timestamp creation_time) {

  /*First check if filename can be found, and if so, just return a pointer to the found file*/
  RamFsFile* found_file;
  RamFs_Status find_status = FindFile(fname, found_file);
  if (find_status == RamFs_Status::SUCCESS){
    pFile = found_file;
    return find_status;
  } else {
    /*Default output values returned when free file slot cannot be found, to be changed if that's not the case*/
    RamFs_Status status = RamFs_Status::FILE_SLOTS_FULL;
    pFile = nullptr;

    size_t filename_length = strlen(fname);
    if (filename_length <= 0 || filename_length >= k_MaxFilenameSize) { 
      status = RamFs_Status::INVALID_FILENAME;
      pFile = nullptr;
    } else {
      for (int i = 0; i < FileNr; i++) {
        if (m_FSys.m_Files[i].m_isActive == false){
          /*Found free file slot, initialize new file*/
          memcpy(&(m_FSys.m_Files[i].m_filename), fname, filename_length);
          m_FSys.m_Files[i].m_creation_timestamp=creation_time;
          m_FSys.m_Files[i].m_isActive = true;
          m_FSys.m_FileCount++;

          pFile = &(m_FSys.m_Files[i]);
          status = RamFs_Status::SUCCESS;
          break;
        } 
      }
    }
    return status;
  }
}
template <size_t FileNr, size_t FragmentNr>
RamFs_Status RamFs<FileNr, FragmentNr>::FindFile(const char* const& fname, RamFsFile*& pFile) {
  /*Default output values returned when file cannot be found, to be changed if that's not the case*/
  RamFs_Status status = RamFs_Status::FILE_NOT_FOUND;
  pFile = nullptr;
  size_t filename_length = strlen(fname);
  if (filename_length <= 0 || filename_length >= k_MaxFilenameSize) {
    pFile = nullptr;
    status = RamFs_Status::INVALID_FILENAME;
  } else {
    for (int i = 0; i < FileNr; i++) {
      if (!(strcmp(m_FSys.m_Files[i].m_filename,fname))) {
        pFile = &(m_FSys.m_Files[i]);
        status = RamFs_Status::SUCCESS;
        break;
      }
    }
  }
  return status;
}
template <size_t FileNr, size_t FragmentNr>
unsigned short RamFs<FileNr, FragmentNr>::GetFileCount() const {
  return m_FSys.m_FileCount;
}

class RamFsFile {
 public:
  template <size_t FileNr, size_t FragmentNr>
  friend class RamFs;
  Timestamp GetCreationTimestamp() const;
  RamFs_Status Write (const void* const pData, const size_t size, const Timestamp modif_time);
  RamFs_Status Read(const void* const pData, const size_t size, const size_t start_pos) const;
  size_t GetSize() const;

 private:
  Filename m_filename;
  size_t m_fileSize;
  Timestamp m_creation_timestamp;
  bool m_isActive = false;
  char dummy;
};

class RamFsFragment{
  public:
  template <size_t FileNr, size_t FragmentNr>
  friend class RamFs;
 private:
  RamFsFragment() = default;
  ~RamFsFragment() = default;
  size_t GetSize() const;
  void Free();
  void Allocate(size_t start, size_t end);
  size_t m_start;
  size_t m_end;
  bool m_isFree = true;
  
};