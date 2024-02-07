#include <cstddef>
#include <cstdint>

class RamAccess {
  public:
  virtual void RamWrite(void* pData, size_t size, size_t address) = 0;
  virtual void RamRead(void* pData, size_t size, size_t address) = 0;
  static constexpr size_t k_RamSize = 8192; /*Edit depending on your underlying RAM*/
  protected: 
  bool CheckRamAccessParameters  (void* pData, size_t size, size_t address);
};

class RamFsFile{
  public:
  uint8_t dummy;
};

class RamFsFragment{
  public:
  uint8_t dummier;
};

template <size_t FileNr = 10, size_t FragmentNr = 10>
class RAMFS {
 public:
  RAMFS(size_t ramSize, RamAccess* pRamAccess);
  bool operator==(const RAMFS& other) const;
  size_t GetUsableSize();
  size_t GetRawFileSystemSize();
  void GetRawFileSystem(void* pData, size_t size);
  void _TempFileMangler_();
  bool WasLoaded();
  bool IsInitialized();
 private:
  RAMFS() = default;
  void LoadFsFromRam();
  void StoreFsInRam();
  bool CheckFileSystem();

  RamAccess* m_ramAccess;
  bool m_wasLoaded = false;
  bool m_isInitialized = false;

  struct FileSystem {
    size_t m_usableSize = 0;
    size_t m_ramSize = 0;
    RamFsFragment m_Fragments[FragmentNr] {};
    RamFsFile m_Files[FileNr]{};
  } m_FileSystem;
};

template <size_t FileNr, size_t FragmentNr>
RAMFS<FileNr, FragmentNr>::RAMFS(size_t ramSize, RamAccess* pRamAccess)
    : m_ramAccess(pRamAccess) {
  if (m_ramAccess != nullptr && ramSize<=RamAccess::k_RamSize) {
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
void RAMFS<FileNr, FragmentNr>::LoadFsFromRam() {
  if (m_ramAccess !=nullptr){
    m_ramAccess->RamRead(&m_FileSystem, sizeof(m_FileSystem), 0);
  }
}

template <size_t FileNr, size_t FragmentNr>
void RAMFS<FileNr, FragmentNr>::StoreFsInRam() {
  //TODO: Add selective write, so not all fs is re-written in ram everytime a byte is written
  if (m_ramAccess != nullptr) {
    m_ramAccess->RamWrite(&m_FileSystem, sizeof(m_FileSystem), 0);
  }
}

template <size_t FileNr, size_t FragmentNr>
bool RAMFS<FileNr, FragmentNr>::CheckFileSystem() {
  return (m_FileSystem.m_usableSize ==  m_FileSystem.m_ramSize - sizeof(m_FileSystem));
  /*For now this is the only way to check a correct load, consider adding a signature if nothing else coomes to mind later*/
}

template <size_t FileNr, size_t FragmentNr>
bool RAMFS<FileNr, FragmentNr>::operator==(const RAMFS& other) const
{
  return !memcmp(&m_FileSystem, &(other.m_FileSystem), sizeof(m_FileSystem));
}

template <size_t FileNr, size_t FragmentNr>
void RAMFS<FileNr, FragmentNr>::_TempFileMangler_(){
  m_FileSystem.m_Files[0].dummy=99;
  StoreFsInRam();
}

template <size_t FileNr, size_t FragmentNr>
bool RAMFS<FileNr, FragmentNr>::WasLoaded() {
  return m_wasLoaded;
}
template <size_t FileNr, size_t FragmentNr>
bool RAMFS<FileNr, FragmentNr>::IsInitialized() {
  return m_isInitialized;
}
