#include "RamFs.h"


bool RamFsFragment::operator==(const RamFsFragment& other) const {
    return !memcmp(&m_storable_params, &(other.m_storable_params),sizeof(m_storable_params));
}

size_t RamFsFragment::GetSize() const {
  size_t frag_size;
  if (m_storable_params.m_end == 0 || m_storable_params.m_start == 0){
    frag_size = 0;
  } else {
    frag_size = BlockSize(m_storable_params.m_start, m_storable_params.m_end);
  }
  return (frag_size > 0)? frag_size: 0;
}

void RamFsFragment::initialize() {
  m_storable_params.m_start = 0;
  m_storable_params.m_end = 0;
  m_storable_params.m_isFree = true;
}

void RamFsFragment::Free() {
 initialize();
 m_parentFs->m_storable_params.m_FragCount--;
}

size_t RamFsFragment::BlockSize(const size_t start, const size_t end){ 
  return end - start + 1; 
}

size_t RamFsFragment::GetStart() const { return m_storable_params.m_start; }
size_t RamFsFragment::GetEnd() const { return m_storable_params.m_end; }

void RamFsFragment::Allocate(const size_t start, const size_t end) {

  int x = RamFs::GetStorableParamsSize();
  int y = m_parentFs->GetRamSize();

  if (start >= RamFs::GetStorableParamsSize() && end > start &&
      end < m_parentFs->GetRamSize()) {
    m_storable_params.m_start = start;
    m_storable_params.m_end = end;
    m_storable_params.m_isFree = false;
    m_parentFs->m_storable_params.m_FragCount++;
  }
  }