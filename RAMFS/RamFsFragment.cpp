#include "RamFs.h"


bool RamFsFragment::operator==(const RamFsFragment& other) const {
    return !memcmp(&m_storable_params, &(other.m_storable_params),sizeof(m_storable_params));
}

RamFsFragment* RamFs::GetFragmentAt(const size_t index) {
  RamFsFragment* pFrag = nullptr;
  if (index < k_FragmentNr) {
    pFrag = &(m_storable_params.m_Fragments[index]);
  }
  return pFrag;
}

size_t RamFsFragment::GetSize() const {
  return (m_storable_params.m_end - m_storable_params.m_start >= 0)? m_storable_params.m_end - m_storable_params.m_start: 0;
}

void RamFsFragment::initialize() {
  m_storable_params.m_start = RamFs::GetStorableParamsSize();
  m_storable_params.m_end = RamFs::GetStorableParamsSize();
  m_storable_params.m_isFree = true;
}

void RamFsFragment::Free() {
 initialize();
 m_parentFs->m_storable_params.m_FragCount--;
}
size_t RamFsFragment::GetStart() const { return m_storable_params.m_start; }
size_t RamFsFragment::GetEnd() const { return m_storable_params.m_end; }

void RamFsFragment::Allocate(const size_t start, const size_t end) {
  if (start >= end && start >= RamFs::GetStorableParamsSize() &&
      end >= RamFs::GetStorableParamsSize()) {
    m_storable_params.m_start = start;
    m_storable_params.m_end = end;
    m_storable_params.m_isFree = false;
    m_parentFs->m_storable_params.m_FragCount++;
  }
  }