#include <cstddef>

#define RAM_SIZE 512

class RamAccess {
  public:
  virtual void RamWrite(void* pData, size_t size, size_t address) = 0;
  virtual void RamRead(void* pData, size_t size, size_t address) = 0;
  protected: 
  bool CheckRamAccessParameters  (void* pData, size_t size, size_t address);
};

class RamFsFile{
  private:
  int dummy;
};

class RamFsFragment{
  private:
  int dummier;
};

template <size_t FileNr = 10, size_t FragmentNr = 10>
class RAMFS {
 public:
  RAMFS(size_t ramSize, RamAccess* pRamAccess);
  bool operator==(const RAMFS& other) const;
  size_t GetUsableSize();
  size_t GetRawFileSystemSize();
  void GetRawFileSystem(void* pData, size_t size);
 private:
  RAMFS() = default;
  void LoadFromRam();
  void StoreInRam();
  bool CheckFileSystem();
  RamAccess* m_ramAccess;
  struct FileSystem {
    size_t m_usableSize;
    size_t m_ramSize;
    RamFsFragment m_Fragments[FragmentNr];
    RamFsFragment m_Files[FileNr];
  } m_FileSystem;
};

template <size_t FileNr, size_t FragmentNr>
RAMFS<FileNr, FragmentNr>::RAMFS(size_t ramSize, RamAccess* pRamAccess)
    : m_ramAccess(pRamAccess) {
  LoadFromRam();
  if (!CheckFileSystem()) {
    m_FileSystem.m_ramSize = ramSize;
    m_FileSystem.m_usableSize = ramSize - sizeof(RAMFS<FileNr, FragmentNr>);
    StoreInRam();
  }
}
template <size_t FileNr, size_t FragmentNr>
size_t RAMFS<FileNr, FragmentNr>::GetUsableSize() {
  return m_FileSystem.m_usableSize;
}

template <size_t FileNr, size_t FragmentNr>
size_t RAMFS<FileNr, FragmentNr>::GetRawFileSystemSize() {
  return sizeof(m_FileSystem);
}

template <size_t FileNr, size_t FragmentNr>
void RAMFS<FileNr, FragmentNr>::GetRawFileSystem(void* pData, size_t size) {
  if (size > sizeof(m_FileSystem)) {
    size = sizeof(m_FileSystem);
  }
  memcpy(pData, &m_FileSystem, size);
}

template <size_t FileNr, size_t FragmentNr>
void RAMFS<FileNr, FragmentNr>::LoadFromRam() {
  m_ramAccess->RamRead(&m_FileSystem, sizeof(m_FileSystem), 0);
}

template <size_t FileNr, size_t FragmentNr>
void RAMFS<FileNr, FragmentNr>::StoreInRam() {
  m_ramAccess->RamWrite(&m_FileSystem, sizeof(m_FileSystem), 0);
}

template <size_t FileNr, size_t FragmentNr>
bool RAMFS<FileNr, FragmentNr>::CheckFileSystem() {
  return (m_FileSystem.m_usableSize ==  m_FileSystem.m_ramSize - sizeof(RAMFS<FileNr, FragmentNr>));
  /*For now this is the only way to check a correct load, consider adding a signature if nothing else coomes to mind later*/
}

template <size_t FileNr, size_t FragmentNr>
bool RAMFS<FileNr, FragmentNr>::operator==(const RAMFS& other) const
{
  return !memcmp(&m_FileSystem,&(other.m_FileSystem),sizeof(m_FileSystem));
}
