#include "RamFs.h"

const RamFsFragment* RamFs::GetRightmostFrag(
    const size_t rightmost_search_address) const {
  // can be improved if we use ordered pointers to frags
  int rightmost_frag_idx = 0;
  for (int i = 1; i < k_FragmentNr; i++) {
    size_t current_end = m_storable_params.m_Fragments[i].m_end;
    if ((current_end < rightmost_search_address) &&
        (current_end >
         m_storable_params.m_Fragments[rightmost_frag_idx].m_end)) {
      rightmost_frag_idx = i;
    }
  }
  return &(m_storable_params.m_Fragments[rightmost_frag_idx]);
}

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

size_t RamFsFragment::GetStart() const { return m_start; }

void RamFsFragment::Allocate(const size_t start, const size_t end) {
  m_start = start;
  m_end = end;
  m_isFree = false;
}

RamFsFragment* RamFs::AllocateNewFragment(const size_t size, int& index) {
  int allocatedFragmentIdx = k_InvalidFragIdx;
  RamFsFragment* pFrag;

  size_t frag_start = 0;
  size_t block_size = 0;
  size_t frag_end = 0;

  if (m_storable_params.m_freeSize > 0) {
    allocatedFragmentIdx = FindFreeFragmentSlot();
    if (allocatedFragmentIdx != k_InvalidFragIdx) {
      FindFreeMemoryBlock(frag_start, block_size);
      frag_end =
          (block_size >= size) ? frag_start + size : frag_start + block_size;
      m_storable_params.m_Fragments[allocatedFragmentIdx].Allocate(frag_start,
                                                                   frag_end);
      index = allocatedFragmentIdx;
      pFrag = &(m_storable_params.m_Fragments[allocatedFragmentIdx]);
    }
  }

  return pFrag;
}