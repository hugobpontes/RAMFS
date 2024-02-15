#include "RamFs.h"


bool RamFsFragment::operator==(const RamFsFragment& other) const {
  return !memcmp(this, &other, sizeof(RamFsFragment));
}

RamFsFragment* RamFs::GetFragmentAt(const size_t index) {
  RamFsFragment* pFrag = nullptr;
  if (index < k_FragmentNr) {
    pFrag = &(m_storable_params.m_Fragments[index]);
  }
  return pFrag;
}

size_t RamFsFragment::GetSize() const {
  return (m_end - m_start >= 0) ? m_end - m_start : 0;
}

void RamFsFragment::initialize() {
  m_start = RamFs::GetStorableParamsSize();
  m_end = RamFs::GetStorableParamsSize();
  m_isFree = true;
}

void RamFsFragment::Free() {
 initialize();
 m_parentFs->m_storable_params.m_FragCount--;
}
  size_t RamFsFragment::GetStart() const { return m_start; }

  void RamFsFragment::Allocate(const size_t start, const size_t end) {
    if (start >= end && start >= RamFs::GetStorableParamsSize() &&
        end >= RamFs::GetStorableParamsSize()) {
      m_start = start;
      m_end = end;
      m_isFree = false;
      m_parentFs->m_storable_params.m_FragCount++;
    }
  }