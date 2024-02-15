#pragma once

#include <cstddef>
#include <cstring>


#include "RamAccess.h"

constexpr int k_MaxFilenameSize = 20;
constexpr int k_MaxFragmentPerFile = 3;
constexpr int k_InvalidFragIdx = -1;
constexpr int k_FileNr = 10;
constexpr int k_FragmentNr = 10;

using Filename = char[k_MaxFilenameSize];
using Timestamp = unsigned long;

enum class RamFs_Status {
  SUCCESS,
  INVALID_FILENAME,
  FILE_SLOTS_FULL,
  FILE_NOT_FOUND,
};

class RamFs;

class RamFsFile {
 public:
  friend class RamFs;
  Timestamp GetCreationTimestamp() const;
  Timestamp GetModificationTimestamp() const;
  RamFs_Status Write (const void* const pData, const size_t size, const Timestamp modif_time);
  RamFs_Status Read(void* const pData, const size_t size, const size_t start_pos) const;
  size_t GetSize() const;
  bool operator==(const RamFsFile& other) const;
  private : 
  RamFsFile() = default;
  ~RamFsFile() = default; 
  void initializeCommon(const char* const& fname, const size_t fname_size,Timestamp creation_time);
  void initialize(const char* const& fname, const size_t fname_size,const Timestamp creation_time);
  void initialize();
  void setActiveState(const bool state);
  void FreeOwnedFragments();
  RamFs_Status TakeHoldOfRequiredFragments(const size_t size);

      RamFs* m_parentFs;
  struct StorableFile{
    Filename m_filename;
    size_t m_fileSize;
    Timestamp m_creationTimestamp;
    Timestamp m_modifTimestamp;
    bool m_isActive = false;
    int m_ownedFragmentsIdxs[k_MaxFragmentPerFile] = {0};
    int m_ownedFragmentsCount;
    char dummy;
  } m_storable_params;
};

class RamFsFragment{
  public:
  
  friend class RamFs;
  friend class RamFsFile;

 private:  
  size_t GetSize() const;
  void Free();
  void Allocate(const size_t start, const size_t end);
  size_t GetStart() const;
  size_t GetEnd() const;
  bool operator==(const RamFsFragment& other) const;
  void initialize();
  RamFsFragment() = default;
  ~RamFsFragment() = default;

  RamFs* m_parentFs;
  struct StorableFileSystem {
    size_t m_start;
    size_t m_end;
    bool m_isFree = true;    
  } m_storable_params;
};

  class RamFs {
   public:
    friend class RamFsFile;
    friend class RamFsFragment;
    RamFs(const size_t ramSize, const RamAccess& RamAccess);
    bool operator==(const RamFs& other) const;
    size_t GetUsableSize() const;
    size_t GetFreeSize() const;
    static size_t GetStorableParamsSize();
    void _TempFileEdit_();
    bool WasLoaded() const;
    bool IsInitialized() const;
    RamFs_Status CreateFile(const char* const& fname, RamFsFile*& pFile,
                            const Timestamp creation_time);
    RamFs_Status FindFile(const char* const& fname, RamFsFile*& pFile);
    unsigned short GetFileCount() const;
    unsigned short GetTakenFragsCount() const;
    void StoreFileInRam(RamFsFile* pFile) const;

   private:
    RamFs() = default;
    void LoadFsFromRam();
    void StoreFsInRam() const;
    bool CheckFileSystem() const;
    int FindFreeFragmentSlot() const;
    void FindFreeMemoryArea(const size_t requested_size, size_t& start,
                            size_t& end) const;
    const RamFsFragment* GetFragEndingClosestTo(const size_t location) const;
    void SetParents();
    void initialize(const size_t ramSize);
    int AllocateNewFragment(const size_t size);
    RamFsFragment* GetFragmentAt(const size_t index);
    void IncrementFreeSize(const int increment);

    const RamAccess& m_ramAccess;
    bool m_wasLoaded = false;
    bool m_isInitialized = false;

    struct StorableFileSystem {
      size_t m_freeSize;
      size_t m_ramSize;
      RamFsFragment m_Fragments[k_FragmentNr]{};
      RamFsFile m_Files[k_FileNr]{};
      unsigned short m_FileCount;
      unsigned short m_FragCount;
    } m_storable_params;
};

