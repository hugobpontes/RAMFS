#include "RamFs.h"




void RamFs::IncrementFreeSize(const int increment) {
  m_storable_params.m_freeSize+=increment;
}

void RamFs::SetParents() {
  for (int i = 0; i < k_FileNr; i++) {
    m_storable_params.m_Files[i].m_parentFs = this;
  }
  for (int i = 0; i < k_FragmentNr; i++) {
    m_storable_params.m_Fragments[i].m_parentFs = this;
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
        m_storable_params.m_Fragments[i].initialize();
      }

}

RamFs::RamFs(const size_t ramSize, const RamAccess& RamAccess)
    : m_ramAccess(RamAccess) {
  if (ramSize <= RamAccess::k_RamSize && ramSize >= sizeof(m_storable_params)) {
    LoadFsFromRam();
    SetParents();
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

size_t RamFs::GetStorableParamsSize() { return sizeof(StorableFileSystem); }

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
    /*TODO: For now this is the only way to check a correct load, consider adding a
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
        m_storable_params.m_FileCount == other.m_storable_params.m_FileCount &&
        m_storable_params.m_FragCount == other.m_storable_params.m_FragCount);
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

void RamFs::FindFreeMemoryArea(const size_t requested_size, size_t& start, size_t& end) const {

  /*Finding logic is as follows: if there is at least one fragment taken, we will have to look
  for a free area, so we start by finding the fragment closest to the end of RAM. When that is found,
  we check that between the end of that fragment and the end of RAM, there is enough space for a new fragment.
  If there is, the while loop never starts. If there isnt we look in the space between that fragment and the previous one
  If not enough space is ever found, the largest possible area is returned */


  /*block locations if number of allocated fragments is 0*/
  size_t best_block_start = RamFs::GetStorableParamsSize();
  size_t best_block_end = best_block_start+requested_size;

  const RamFsFragment* pClosestFrag;

  if (m_storable_params.m_FragCount != 0){
    
    size_t current_block_start = 0;
    size_t current_block_end = 0;

    pClosestFrag = GetFragEndingClosestTo(m_storable_params.m_ramSize);
    current_block_end = m_storable_params.m_ramSize;
    current_block_start = pClosestFrag->m_end < current_block_end-requested_size?current_block_end-requested_size:pClosestFrag->m_end;
    best_block_start = current_block_start;
    best_block_end = current_block_end;

    while ((current_block_end - current_block_start) < requested_size) {
      current_block_end = pClosestFrag->m_start;
      pClosestFrag = GetFragEndingClosestTo(current_block_end);
      current_block_start = pClosestFrag->m_end < current_block_end-requested_size?current_block_end-requested_size:pClosestFrag->m_end;
      if ((current_block_end - current_block_start) > (best_block_end - best_block_start)) {
        best_block_start = current_block_start;
        best_block_end = current_block_end;
      }
    }
  } 

  start = best_block_start;
  end = best_block_end;
}

int RamFs::AllocateNewFragment(const size_t size) {
  int allocatedFragmentIdx = k_InvalidFragIdx;

  size_t frag_start = 0;
  size_t frag_end = 0;

  if (m_storable_params.m_freeSize > 0) {
    allocatedFragmentIdx = FindFreeFragmentSlot();
    if (allocatedFragmentIdx != k_InvalidFragIdx) {
      FindFreeMemoryArea(size, frag_start, frag_end);
      m_storable_params.m_Fragments[allocatedFragmentIdx].Allocate(frag_start,frag_end);
      allocatedFragmentIdx = allocatedFragmentIdx;
      m_storable_params.m_FragCount++;
    }
  }

  return allocatedFragmentIdx;
}

const RamFsFragment* RamFs::GetFragEndingClosestTo(const size_t location) const {
  // can be improved if we use ordered pointers to frags
  int closest_frag_idx = 0;
  for (int i = 1; i < k_FragmentNr; i++) {
    size_t candidate_end = m_storable_params.m_Fragments[i].m_end;
    if ((candidate_end <= location) && (candidate_end > m_storable_params.m_Fragments[closest_frag_idx].m_end)) {
      closest_frag_idx = i;
    }
  }
  return &(m_storable_params.m_Fragments[closest_frag_idx]);
}

unsigned short RamFs::GetTakenFragsCount() const {
  return m_storable_params.m_FragCount;
}
