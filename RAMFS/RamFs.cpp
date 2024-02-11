#include "RamFs.h"

Timestamp RamFsFile::GetCreationTimestamp() const { 
  return m_creation_timestamp; 
}

RamFs_Status RamFsFile::Write(const void* const pData, const size_t size, const Timestamp modif_time){
    //free all used chunks

    //check args
    //while available size < size
    //request fragments
    //if frag size = 0 break and fail
    //store frag ptr, increase available size with fragment size, increase framgnet count
    //if available size ok just break

    //write in chunks, as per chunk size.
    //when file_size reaches requested size, stop
    return RamFs_Status::SUCCESS;
}

RamFs_Status RamFsFile::Read(const void* const pData, const size_t size, const size_t start_pos) const{
    return RamFs_Status::SUCCESS;
}

size_t RamFsFile::GetSize() const {
  return m_fileSize;
}

size_t RamFsFragment::GetSize() const{
  return (m_end-m_start>=0) ? m_end-m_start : 0; 
}