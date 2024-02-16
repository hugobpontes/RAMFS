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
  pFile = nullptr;
  if (fname == nullptr) {
    return RamFs_Status::NULL_POINTER;
  }
  RamFsFile* found_file;
  RamFs_Status find_status = FindFile(fname, found_file);
  if (find_status == RamFs_Status::SUCCESS) {
    pFile = found_file;
    return RamFs_Status::SUCCESS;
  } 
  size_t filename_length = strlen(fname);
  if (filename_length <= 0 || filename_length >= k_MaxFilenameSize) {
    return RamFs_Status::INVALID_FILENAME;
  } else {
    for (int i = 0; i < k_FileNr; i++) {
      if (m_storable_params.m_Files[i].m_storable_params.m_isActive == false) {
        /*Found free file slot, initialize new file*/
        m_storable_params.m_Files[i].initialize(fname,filename_length,creation_time);
        m_storable_params.m_Files[i].setActiveState(true);
        m_storable_params.m_FileCount++;

        pFile = &(m_storable_params.m_Files[i]);
        return RamFs_Status::SUCCESS;
      }
    }
  }
  return RamFs_Status::FILE_SLOTS_FULL;
}

RamFs_Status RamFs::FindFile(const char* const& fname, RamFsFile*& pFile) {


  //TODO: instead of comparing strings, there should be an hashtable to convert from string to id, and then compare that instead.

  pFile = nullptr;
  if (fname == nullptr){
    return RamFs_Status::NULL_POINTER;
  }
  
  size_t filename_length = strlen(fname);
  if (filename_length <= 0 || filename_length >= k_MaxFilenameSize) {
    return RamFs_Status::INVALID_FILENAME;
  } else {
    for (int i = 0; i < k_FileNr; i++) {
      if (!(strcmp(m_storable_params.m_Files[i].m_storable_params.m_filename, fname))) {
        pFile = &(m_storable_params.m_Files[i]);
        return RamFs_Status::SUCCESS;
      }
    }
  }
  return RamFs_Status::FILE_NOT_FOUND;
}

unsigned short RamFs::GetFileCount() const { return m_storable_params.m_FileCount; }

int RamFs::FindFreeFragmentSlot() const {
  int found_slot = k_InvalidFragIdx;
  for (int i = 0; i < k_FragmentNr; i++) {
    if (m_storable_params.m_Fragments[i].m_storable_params.m_isFree) {
      found_slot = i;
      break;
    }
  }
  return found_slot;
}

size_t RamFs::GetBlockStart(const size_t end_of_closest_frag, const size_t requested_size, const size_t block_end) const {
  
  size_t block_start;

  if (end_of_closest_frag == block_end){ //there is no space between end of closest frag and end of proposed block, so block has size 0
    block_start = block_end;
  } else {
    if (end_of_closest_frag + 1 + requested_size <= block_end) {
      block_start = block_end - requested_size;
    } else {
      block_start = end_of_closest_frag + 1;
    }
  }

  return block_start;
}

void RamFs::FindFreeMemoryArea(const size_t requested_size, size_t& start, size_t& end) const {

  /*Finding logic is as follows: if there is at least one fragment taken, we will have to look
  for a free area, so we start by finding the fragment closest to the end of RAM. When that is found,
  we check that between the end of that fragment and the end of RAM, there is enough space for a new fragment.
  If there is, the while loop never starts. If there isnt we look in the space between that fragment and the previous one
  If not enough space is ever found, the largest possible area is returned */


  /*block locations if number of allocated fragments is 0*/

  const size_t init_block_end = m_storable_params.m_ramSize - 1;
  const size_t init_block_start = init_block_end - requested_size;
  size_t best_block_end = init_block_end;
  size_t best_block_start = init_block_start;

  const RamFsFragment * pClosestFrag;

  if (m_storable_params.m_FragCount != 0){
    
    size_t tentative_block_start = 0;
    size_t tentative_block_end = 0;

    tentative_block_end = init_block_end;

    pClosestFrag = GetFragEndingClosestTo(m_storable_params.m_ramSize);

    tentative_block_start = GetBlockStart(pClosestFrag->GetEnd(),requested_size, tentative_block_end);

    best_block_start = tentative_block_start;
    best_block_end = tentative_block_end;

    while ((tentative_block_end - tentative_block_start) < requested_size) {

      tentative_block_end = pClosestFrag->GetStart()-1;
      pClosestFrag = GetFragEndingClosestTo(tentative_block_end);

      tentative_block_start = GetBlockStart(pClosestFrag->GetEnd(),requested_size, tentative_block_end);

      /*try to find the largest block possible*/
      if ((tentative_block_end - tentative_block_start) > (best_block_end - best_block_start)) {
        best_block_start = tentative_block_start;
        best_block_end = tentative_block_end;
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
    }
  }

  return allocatedFragmentIdx;
}

const RamFsFragment* RamFs::GetFragEndingClosestTo(const size_t location) const {
  // can be improved if we use ordered pointers to frags
  int closest_frag_idx = 0;
  size_t closest_frag_end = 0;
  size_t candidate_end = 0;

    for (int i = 0; i < k_FragmentNr; i++) {
    candidate_end = m_storable_params.m_Fragments[i].GetEnd();
    if ((candidate_end < location) && (candidate_end > closest_frag_end)) {
      closest_frag_end = candidate_end;
      closest_frag_idx = i;
    }
  }
  return &(m_storable_params.m_Fragments[closest_frag_idx]);
}

unsigned short RamFs::GetTakenFragsCount() const {
  return m_storable_params.m_FragCount;
}

size_t RamFs::GetRamSize() const {
  return m_storable_params.m_ramSize;
}
