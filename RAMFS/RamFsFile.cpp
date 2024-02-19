#include "RamFs.h"

Timestamp RamFsFile::GetCreationTimestamp() const {
  return m_storable_params.m_creationTimestamp;
}

Timestamp RamFsFile::GetModificationTimestamp() const {
  return m_storable_params.m_modifTimestamp;
}

void RamFsFile::FreeOwnedFragments() {
  for (int i = 0; i < m_storable_params.m_ownedFragmentsCount; i++) {
      RamFsFragment* pFrag = m_parentFs->GetFragmentAt(m_storable_params.m_ownedFragmentsIdxs[i]);
      pFrag->Free(); 
  }
  for (int i = 0; i < k_MaxFragmentPerFile; i++) {
    m_storable_params.m_ownedFragmentsIdxs[i] = 0;
  }
  m_storable_params.m_ownedFragmentsCount = 0;
}

int RamFsFile::ComputeRequiredFragments(const size_t size) const {
  //break condition, -1 if 1) not enough frags are available 2) frags goes over frag limit
  return 0;
}

RamFs_Status RamFsFile::TakeHoldOfRequiredFragments(const size_t size) {
  
  size_t owned_memory_size = 0;

  int new_frag_idx = k_InvalidFragIdx;
  RamFsFragment* p_new_frag;

  while (owned_memory_size < size) {
    if (m_storable_params.m_ownedFragmentsCount >= k_MaxFragmentPerFile) {
      // this is actually just for append
      //  set bad status
      break;
    }

    new_frag_idx = m_parentFs->AllocateNewFragment(size-owned_memory_size);

    if (new_frag_idx == k_InvalidFragIdx || p_new_frag == nullptr) {
      // set bad status
      break;
    }
    m_storable_params.m_ownedFragmentsIdxs[m_storable_params.m_ownedFragmentsCount] = new_frag_idx;
    m_storable_params.m_ownedFragmentsCount++;
    p_new_frag = m_parentFs->GetFragmentAt(new_frag_idx);
    owned_memory_size += p_new_frag->GetSize();
  }

  m_storable_params.m_fileSize = owned_memory_size;
  m_parentFs->IncrementFreeSize(-owned_memory_size);

  return RamFs_Status::SUCCESS;
}

RamFs_Status RamFsFile::Write(const void* const pData, const size_t requested_size,
                                const Timestamp modif_time) {
    
    size_t already_written_size = 0;

    if (pData == nullptr){
      return RamFs_Status::NULL_POINTER;
    }
    if (requested_size > m_parentFs->GetFreeSize()) {
      return RamFs_Status::INSUFFICIENT_STORAGE;
    }

    FreeHeldData();

    RamFs_Status take_status = TakeHoldOfRequiredFragments(requested_size);

    if (take_status == RamFs_Status::SUCCESS){
      for (int i = 0; i < m_storable_params.m_ownedFragmentsCount; i++) {
        RamFsFragment* pFrag;
        pFrag = m_parentFs->GetFragmentAt(m_storable_params.m_ownedFragmentsIdxs[i]);
        m_parentFs->m_ramAccess.RamWrite(static_cast<const char* const>(pData) + already_written_size,pFrag->GetSize(), pFrag->GetStart());
        already_written_size += pFrag->GetSize();
      }
      m_storable_params.m_modifTimestamp = modif_time;
      m_parentFs->StoreFileInRam(this);

    } else {
      return take_status;
    }

    return RamFs_Status::SUCCESS;  
  }

  void RamFsFile::ReadFromFileFrags(void* const pData,
                                            const int starting_frag,
                                            const int starting_frag_pos,
                                            const size_t requested_size) const {
    size_t size_yet_to_read_from_file;
    size_t size_to_read_from_current_frag;
    size_t already_read_size = 0;
    size_t frag_read_pos = starting_frag_pos;

    RamFsFragment* pCurrentFrag;

    for (int i = starting_frag; i < m_storable_params.m_ownedFragmentsCount;i++) {
      pCurrentFrag = m_parentFs->GetFragmentAt(m_storable_params.m_ownedFragmentsIdxs[i]);

      size_yet_to_read_from_file = requested_size - already_read_size;

      if (size_yet_to_read_from_file > pCurrentFrag->GetSize() - frag_read_pos) {
        size_to_read_from_current_frag = pCurrentFrag->GetSize() - frag_read_pos;
      } else {
        size_to_read_from_current_frag = size_yet_to_read_from_file;
      }

      m_parentFs->m_ramAccess.RamRead(static_cast<char* const>(pData) + already_read_size,size_to_read_from_current_frag,pCurrentFrag->GetStart() + starting_frag_pos);
      already_read_size += size_to_read_from_current_frag;
      frag_read_pos = 0;

      if (already_read_size == requested_size){
        break;
      }
    }
  }

  RamFs_Status RamFsFile::Read(void* const pData, const size_t requested_size,
                               const size_t start_pos) const {
    
    size_t already_read_size = 0;

    if (pData == nullptr) {
      return RamFs_Status::NULL_POINTER;
    }
    if (start_pos >= m_storable_params.m_fileSize){
      return RamFs_Status::READ_OUT_OF_BOUNDS;
    }    

    /*Needs to be improved to take fragmented files into account
    this needs to be computed instead of just assigned*/
    int starting_frag = 0;
    int starting_frag_pos = start_pos;

    ReadFromFileFrags(pData,starting_frag,starting_frag_pos,requested_size);

    return RamFs_Status::SUCCESS;
}

  void RamFsFile::initializeCommon(const char* const& fname,
                                   const size_t fname_size,
                                   const Timestamp creation_time) {
    memcpy(&(m_storable_params.m_filename), fname, fname_size);
    m_storable_params.m_fileSize = 0;
    m_storable_params.m_creationTimestamp = creation_time;
    m_storable_params.m_modifTimestamp = creation_time;
    m_storable_params.m_isActive = false;

  }
  void RamFsFile::setActiveState(const bool state) {
    m_storable_params.m_isActive = state;
  }
  void RamFsFile::initialize(const char* const& fname, const size_t fname_size,
                             const Timestamp creation_time) {
    initializeCommon(fname, fname_size, creation_time);
  }

  void RamFsFile::initialize() {
    char emptyFilename[k_MaxFilenameSize] = {};
    initializeCommon(emptyFilename, k_MaxFilenameSize, 0);
  }

  size_t RamFsFile::GetSize() const { return m_storable_params.m_fileSize; }

  bool RamFsFile::operator==(const RamFsFile& other) const {
    return !memcmp(&m_storable_params, &(other.m_storable_params),
                   sizeof(m_storable_params));
  }

  void RamFsFile::FreeHeldData() {
    m_parentFs->IncrementFreeSize(m_storable_params.m_fileSize);
    FreeOwnedFragments(); 
  }    

  RamFs_Status RamFsFile::Delete() {

    FreeHeldData();
    m_parentFs->m_storable_params.m_FileCount--;
    m_parentFs->StoreFileInRam(this); 
    initialize();

    return RamFs_Status::SUCCESS;
  }