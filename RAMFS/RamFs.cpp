#include "RamFs.h"




void RamFs::IncrementFreeSize(const int increment) {
  m_storable_params.m_freeSize+=increment;
}

void RamFs::SetFilesParent() {
  for (int i = 0; i < k_FileNr; i++) {
    m_storable_params.m_Files[i].m_parentFs = this;
  }
}

void RamFs::initialize(const size_t ramSize){
      m_storable_params.m_ramSize = ramSize;
      m_storable_params.m_freeSize = ramSize - sizeof(m_storable_params);
      m_storable_params.m_FileCount = 0;
      for (int i = 0; i < k_FileNr; i++) {
        m_storable_params.m_Files[i].initialize();
      }
      for (int i = 0; i < k_FileNr; i++) {
        m_storable_params.m_Fragments[i].initialize(sizeof(m_storable_params));
      }

}

RamFs::RamFs(const size_t ramSize, const RamAccess& RamAccess)
    : m_ramAccess(RamAccess) {
  if (ramSize <= RamAccess::k_RamSize && ramSize >= sizeof(m_storable_params)) {
    LoadFsFromRam();
    SetFilesParent();
    if (!CheckFileSystem() || m_storable_params.m_ramSize != ramSize) {
      initialize(ramSize);
      StoreFsInRam();
    } else {
      m_wasLoaded = true;
    }
    m_isInitialized = true;
  }
}
size_t RamFs::GetUsableSize() const {
  return m_storable_params.m_ramSize - sizeof(m_storable_params);
}

size_t RamFs::GetFreeSize() const { return m_storable_params.m_freeSize; }

size_t RamFs::GetStorableParamsSize() const { return sizeof(m_storable_params); }

void RamFs::LoadFsFromRam() { 
  //TO DO: Dont read pointers
  m_ramAccess.RamRead(&m_storable_params, sizeof(m_storable_params), 0); 
}

void RamFs::StoreFileInRam(RamFsFile* pFile) const {
  //figure out how to write only the file
  StoreFsInRam();
}
  void RamFs::StoreFsInRam() const {
    // TO-DO: Dont write pointers
    //  TODO: Add selective write, maybe in a different function, so not all fs
    //  is re-written in ram everytime a byte is written
    m_ramAccess.RamWrite(&m_storable_params, sizeof(m_storable_params), 0);
  }

  bool RamFs::CheckFileSystem() const {
    return (m_storable_params.m_freeSize <=
            m_storable_params.m_ramSize - sizeof(m_storable_params));
    /*For now this is the only way to check a correct load, consider adding a
     * signature if nothing else coomes to mind later*/
  }

  bool RamFs::operator==(const RamFs& other) const {
    bool filesMatch = true;
    bool fragmentsMatch = true;

    for (int i = 0; i < k_FileNr; i++) {
      if (!(m_storable_params.m_Files[i] ==
            other.m_storable_params.m_Files[i])) {
        filesMatch = false;
        break;
      }
    }

    for (int i = 0; i < k_FragmentNr; i++) {
      if (!(m_storable_params.m_Fragments[i] ==
            other.m_storable_params.m_Fragments[i])) {
        fragmentsMatch = false;
        break;
      }
    }
    return (
        filesMatch && fragmentsMatch &&
        m_storable_params.m_freeSize == other.m_storable_params.m_freeSize &&
        m_storable_params.m_ramSize == other.m_storable_params.m_ramSize &&
        m_storable_params.m_FileCount == other.m_storable_params.m_FileCount);
  }

void RamFs::_TempFileEdit_() {
  m_storable_params.m_Files[0].m_storable_params.dummy = 99;
  StoreFsInRam();
}

bool RamFs::WasLoaded() const { return m_wasLoaded; }

bool RamFs::IsInitialized() const { return m_isInitialized; }

RamFs_Status RamFs::CreateFile(const char* const& fname, RamFsFile*& pFile,
                               const Timestamp creation_time) {
  /*First check if filename can be found, and if so, just return a pointer to
   * the found file*/
  RamFsFile* found_file;
  RamFs_Status find_status = FindFile(fname, found_file);
  if (find_status == RamFs_Status::SUCCESS) {
    pFile = found_file;
    return find_status;
  } else {
    /*Default output values returned when free file slot cannot be found, to be
     * changed if that's not the case*/
    RamFs_Status status = RamFs_Status::FILE_SLOTS_FULL;
    pFile = nullptr;

    size_t filename_length = strlen(fname);
    if (filename_length <= 0 || filename_length >= k_MaxFilenameSize) {
      status = RamFs_Status::INVALID_FILENAME;
      pFile = nullptr;
    } else {
      for (int i = 0; i < k_FileNr; i++) {
        if (m_storable_params.m_Files[i].m_storable_params.m_isActive == false) {
          /*Found free file slot, initialize new file*/
          m_storable_params.m_Files[i].initialize(fname,filename_length,creation_time);
          m_storable_params.m_Files[i].setActiveState(true);
          m_storable_params.m_FileCount++;

          pFile = &(m_storable_params.m_Files[i]);
          status = RamFs_Status::SUCCESS;
          break;
        }
      }
    }
    return status;
  }
}

RamFs_Status RamFs::FindFile(const char* const& fname, RamFsFile*& pFile) {
  /*Default output values returned when file cannot be found, to be changed if
   * that's not the case*/
  RamFs_Status status = RamFs_Status::FILE_NOT_FOUND;
  pFile = nullptr;
  size_t filename_length = strlen(fname);
  if (filename_length <= 0 || filename_length >= k_MaxFilenameSize) {
    pFile = nullptr;
    status = RamFs_Status::INVALID_FILENAME;
  } else {
    for (int i = 0; i < k_FileNr; i++) {
      if (!(strcmp(m_storable_params.m_Files[i].m_storable_params.m_filename, fname))) {
        pFile = &(m_storable_params.m_Files[i]);
        status = RamFs_Status::SUCCESS;
        break;
      }
    }
  }
  return status;
}

unsigned short RamFs::GetFileCount() const { return m_storable_params.m_FileCount; }

int RamFs::FindFreeFragmentSlot() const {
  int found_slot = k_InvalidFragIdx;
  for (int i = 0; i < k_FragmentNr; i++) {
    if (m_storable_params.m_Fragments[i].m_isFree) {
      found_slot = i;
      break;
    }
  }
  return found_slot;
}


void RamFs::FindFreeMemoryBlock(size_t& start, size_t& size) const {
  size_t block_start = m_storable_params.m_ramSize;
  size_t block_end = m_storable_params.m_ramSize;
  const RamFsFragment* pRightmostFrag = GetRightmostFrag(block_end);
  // check nullptr, return ram sizes if
  block_start = pRightmostFrag->m_end;
  if (block_start !=
      0) {  // if there is no fragment allocated, everything is a free block
      //also should not be checking for 0 but for usable start 
    while (block_start == block_end) {
      block_end = pRightmostFrag->m_start;
      pRightmostFrag = GetRightmostFrag(block_end);
      block_start = pRightmostFrag->m_end;
    }
  }
  start = block_start;
  size = block_end - block_start;
}

