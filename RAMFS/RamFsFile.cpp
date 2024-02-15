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
      pFrag->initialize(); //free fragment
  }
  m_storable_params.m_ownedFragmentsCount = 0;
}

RamFs_Status RamFsFile::TakeHoldOfRequiredFragments(const size_t size) {
  
  size_t owned_memory_size = 0;
  RamFs_Status status = RamFs_Status::SUCCESS;
  int new_frag_idx = k_InvalidFragIdx;
  RamFsFragment* p_new_frag;

  while (owned_memory_size < size) {
    if (m_storable_params.m_ownedFragmentsCount >= k_MaxFragmentPerFile) {
      // this is actually just for append
      //  set bad status
      break;
    }

    new_frag_idx = m_parentFs->AllocateNewFragment(size);

    if (new_frag_idx == k_InvalidFragIdx || p_new_frag == nullptr) {
      // set bad status
      break;
    }
    m_storable_params.m_ownedFragmentsIdxs[m_storable_params.m_ownedFragmentsCount] = new_frag_idx;
    m_storable_params.m_ownedFragmentsCount++;
    p_new_frag = m_parentFs->GetFragmentAt(new_frag_idx);
    owned_memory_size += p_new_frag->GetSize();
  }
  return status;
}

RamFs_Status RamFsFile::Write(const void* const pData, const size_t size,
                                const Timestamp modif_time) {
    // check args
    RamFs_Status status = RamFs_Status::SUCCESS;
    size_t written_size = 0;

    FreeOwnedFragments();
    m_parentFs->IncrementFreeSize(m_storable_params.m_fileSize);

    status = TakeHoldOfRequiredFragments(size);

    if (status == RamFs_Status::SUCCESS){
      for (int i = 0; i < m_storable_params.m_ownedFragmentsCount; i++) {
        RamFsFragment* pFrag;
        pFrag = m_parentFs->GetFragmentAt(m_storable_params.m_ownedFragmentsIdxs[i]);
        m_parentFs->m_ramAccess.RamWrite(static_cast<const char* const>(pData) + written_size,pFrag->GetSize(), pFrag->GetStart());
        written_size += pFrag->GetSize();
      }
      m_storable_params.m_fileSize = written_size;
      m_parentFs->IncrementFreeSize(-written_size);
      m_parentFs->StoreFileInRam(this);
      m_storable_params.m_modifTimestamp = modif_time;
    }

    return RamFs_Status::SUCCESS;  // return status var
  }

  RamFs_Status RamFsFile::Read(void* const pData, const size_t size,
                               const size_t start_pos) const {
    size_t read_size = 0;

    if (0) {  // check params and also check frags are appropriate

    } else {
      for (int i = 0; i < m_storable_params.m_ownedFragmentsCount; i++) {
        RamFsFragment* pFrag;
        pFrag = m_parentFs->GetFragmentAt(
            m_storable_params.m_ownedFragmentsIdxs[i]);
        m_parentFs->m_ramAccess.RamRead(
            static_cast<char* const>(pData) + read_size, pFrag->GetSize(),
            pFrag->GetStart());
        read_size += pFrag->GetSize();
      }
    }
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
    for (int i = 0; i < k_MaxFragmentPerFile; i++) {
      m_storable_params.m_ownedFragmentsIdxs[i] = 0;
    }
    m_storable_params.m_ownedFragmentsCount = 0;
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