#include "RamFs.h"


Timestamp RamFsFile::GetCreationTimestamp() const {
  return m_storable_params.m_creationTimestamp;
}


RamFs_Status RamFsFile::Write(const void* const pData, const size_t size, const Timestamp modif_time){
    //free all used chunks (including setting end and start to 0)

    //check args
    size_t owned_memory_size = 0;
    while (owned_memory_size < size)
    {
      int new_frag_idx;
      RamFsFragment* p_new_frag;
      if (m_storable_params.m_ownedFragmentsCount >= k_MaxFragmentPerFile) {
        // set bad status
        break;
      }
      p_new_frag = m_parentFs->AllocateNewFragment(size, new_frag_idx);

      if (new_frag_idx = k_InvalidFragIdx || p_new_frag == nullptr){
        //set bad status
        break;
      } 
      m_storable_params.m_ownedFragmentsIdxs[m_storable_params.m_ownedFragmentsCount] = new_frag_idx;
      m_storable_params.m_ownedFragmentsCount++;
      owned_memory_size+= p_new_frag->GetSize();
    }

    if(1) //status success<//enough space to write
    for (int i = 0; i<m_storable_params.m_ownedFragmentsCount;i++){
      RamFsFragment* pFrag;
      m_parentFs->m_ramAccess.RamWrite(pData,pFrag->GetSize(),pFrag->GetStart());
    }
    
    return RamFs_Status::SUCCESS; //return status var
}


RamFs_Status RamFsFile::Read(const void* const pData,
                                                 const size_t size,
                                                 const size_t start_pos) const {
  // simple: go through owned frags and read into pData
  return RamFs_Status::SUCCESS;
}


size_t RamFsFile::GetSize() const {
  return m_storable_params.m_fileSize;
}

bool RamFsFile::operator == (const RamFsFile& other) const {
  return !memcmp(&m_storable_params,&(other.m_storable_params),sizeof(m_storable_params));
}

void RamFs::SetFilesParent() {
  for (int i = 0; i < k_FileNr; i++) {
    m_FSys.m_Files[i].m_parentFs = this;
  }
}

RamFs::RamFs(const size_t ramSize, const RamAccess& RamAccess)
    : m_ramAccess(RamAccess) {
  if (ramSize <= RamAccess::k_RamSize && ramSize >= sizeof(m_FSys)) {
    LoadFsFromRam();
    SetFilesParent();
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
size_t RamFs::GetUsableSize() const {
  return m_FSys.m_ramSize - sizeof(m_FSys);
}

size_t RamFs::GetFreeSize() const { return m_FSys.m_freeSize; }

size_t RamFs::GetStorableFileSystemSize() const { return sizeof(m_FSys); }

void RamFs::GetStorableFileSystem(void* const pData, size_t size) const {
  if (size > sizeof(m_FSys)) {
    size = sizeof(m_FSys);
  }
  memcpy(pData, &m_FSys, size);
}

void RamFs::LoadFsFromRam() { 
  //TO DO: Dont read pointers
  m_ramAccess.RamRead(&m_FSys, sizeof(m_FSys), 0); }

void RamFs::StoreFsInRam() const {
  //TO-DO: Dont write pointers
  // TODO: Add selective write, maybe in a different function, so not all fs is
  // re-written in ram everytime a byte is written
  m_ramAccess.RamWrite(&m_FSys, sizeof(m_FSys), 0);
}

bool RamFs::CheckFileSystem() const {
  return (m_FSys.m_freeSize <= m_FSys.m_ramSize - sizeof(m_FSys));
  /*For now this is the only way to check a correct load, consider adding a
   * signature if nothing else coomes to mind later*/
}

bool RamFs::operator==(const RamFs& other) const {
  bool filesMatch = true;
  bool fragmentsMatch = true;

  for (int i = 0; i<k_FileNr;i++){
    if (!(m_FSys.m_Files[i] == other.m_FSys.m_Files[i])) {
      filesMatch = false;
      break;
    }
  }

  for (int i = 0; i < k_FragmentNr; i++) {
    if (!(m_FSys.m_Fragments[i] == other.m_FSys.m_Fragments[i])) {
      fragmentsMatch = false;
      break;
    }
  }
  return (filesMatch &&
          fragmentsMatch &&
          m_FSys.m_freeSize == other.m_FSys.m_freeSize &&
          m_FSys.m_ramSize == other.m_FSys.m_ramSize &&
          m_FSys.m_FileCount == other.m_FSys.m_FileCount);
  }

void RamFs::_TempFileEdit_() {
  m_FSys.m_Files[0].m_storable_params.dummy = 99;
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
        if (m_FSys.m_Files[i].m_storable_params.m_isActive == false) {
          /*Found free file slot, initialize new file*/
          memcpy(&(m_FSys.m_Files[i].m_storable_params.m_filename), fname, filename_length);
          m_FSys.m_Files[i].m_storable_params.m_creationTimestamp = creation_time;
          m_FSys.m_Files[i].m_storable_params.m_isActive = true;
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
      if (!(strcmp(m_FSys.m_Files[i].m_storable_params.m_filename, fname))) {
        pFile = &(m_FSys.m_Files[i]);
        status = RamFs_Status::SUCCESS;
        break;
      }
    }
  }
  return status;
}

unsigned short RamFs::GetFileCount() const { return m_FSys.m_FileCount; }

int RamFs::FindFreeFragmentSlot() const {
  int found_slot = k_InvalidFragIdx;
  for (int i = 0; i < k_FragmentNr; i++) {
    if (m_FSys.m_Fragments[i].m_isFree) {
      found_slot = i;
      break;
    }
  }
  return found_slot;
}


void RamFs::FindFreeMemoryBlock(size_t& start, size_t& size) const {
  size_t block_start = m_FSys.m_ramSize;
  size_t block_end = m_FSys.m_ramSize;
  const RamFsFragment* pRightmostFrag = GetRightmostFrag(block_end);
  // check nullptr, return ram sizes if
  block_start = pRightmostFrag->m_end;
  if (block_start !=
      0) {  // if there is no fragment allocated, everything is a free block
    while (block_start == block_end) {
      block_end = pRightmostFrag->m_start;
      pRightmostFrag = GetRightmostFrag(block_end);
      block_start = pRightmostFrag->m_end;
    }
  }
  start = block_start;
  size = block_end - block_start;
}

const RamFsFragment* RamFs::GetRightmostFrag(
    const size_t rightmost_search_address) const {
  // can be improved if we use ordered pointers to frags
  int rightmost_frag_idx = 0;
  for (int i = 1; i < k_FragmentNr; i++) {
    size_t current_end = m_FSys.m_Fragments[i].m_end;
    if ((current_end < rightmost_search_address) &&
        (current_end > m_FSys.m_Fragments[rightmost_frag_idx].m_end)) {
      rightmost_frag_idx = i;
    }
  }
  return &(m_FSys.m_Fragments[rightmost_search_address]);
}

bool RamFsFragment::operator==(const RamFsFragment& other) const {
  return !memcmp(this, &other, sizeof(RamFsFragment));
}

RamFsFragment* RamFs::GetFragmentAt(const size_t index) {
  RamFsFragment* pFrag = nullptr;
  if (index < k_FragmentNr) {
    pFrag = &(m_FSys.m_Fragments[index]);
  }
  return pFrag;
}

size_t RamFsFragment::GetSize() const{
  return (m_end-m_start>=0) ? m_end-m_start : 0; 
}

size_t RamFsFragment::GetStart() const {
  return m_start;
}

void RamFsFragment::Allocate(const size_t start, const size_t end){
  m_start = m_start;
  m_end = m_end;
}

RamFsFragment* RamFs::AllocateNewFragment(const size_t size, int& index) {
  int allocatedFragmentIdx = k_InvalidFragIdx;
  RamFsFragment* pFrag;

  size_t frag_start = 0;
  size_t block_size = 0;
  size_t frag_end = 0;

  if (m_FSys.m_freeSize > 0) {
    allocatedFragmentIdx = FindFreeFragmentSlot();
    if (allocatedFragmentIdx != k_InvalidFragIdx) {
      FindFreeMemoryBlock(frag_start, block_size);
      frag_end =
          (block_size >= size) ? frag_start + size : frag_start + block_size;
      m_FSys.m_Fragments[allocatedFragmentIdx].Allocate(frag_start, frag_end);
      index = allocatedFragmentIdx;
      pFrag = &(m_FSys.m_Fragments[allocatedFragmentIdx]);
    }
  }

  return pFrag;
}