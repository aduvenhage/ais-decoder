#ifndef AIS_FILE_UTIL_H
#define AIS_FILE_UTIL_H


#include "ais_decoder.h"

#include <fstream>
#include <vector>
#include <string>



namespace AIS
{
    
    /**
     Lightweight buffer for processing file chunks.
     Internal buffer is allowed to grow, but not shrink.
     This avoids allocation and resize init overheads, if the buffer is resued for multiple chunks.
     */
    struct FileBuffer
    {
        FileBuffer()
            :m_uSize(0)
        {}
        
        const char *data() const {return m_data.data();}
        char *data() {return (char*)m_data.data();}
        
        size_t size() const {return m_uSize;}
        
        void resize(size_t _uSize) {
            m_uSize = _uSize;
            if (m_uSize > m_data.size())
            {
                m_data.resize(m_uSize);
            }
        }
        
        void pop_front(size_t _uSize)
        {
            if (_uSize < m_uSize)
            {
                std::memmove((char*)m_data.data(), (char*)m_data.data() + _uSize, m_uSize - _uSize);
                m_uSize -= _uSize;
            }
            else
            {
                m_uSize = 0;
            }
        }
        
        std::vector<char>       m_data;
        size_t                  m_uSize;
    };
    
    
    /**
     Read and process a file in blocks of _uBlockSize, using the provided decoder.
     
     _progressCb is called after the processing of each block. It should have the signature: func(size_t _uTotalBytesReadFromFile, const AIS::AisDecoder &_decoder)
     
     */
    template <typename progress_func_t>
    void processAisFile(const std::string &_strLogPath, AIS::AisDecoder &_decoder, size_t _uBlockSize, progress_func_t &_progressCb)
    {
        FileBuffer buffer;
        
        // open file
        FILE *pFileIn = fopen(_strLogPath.c_str(), "rb");
        if (pFileIn != nullptr)
        {
            size_t uTotalBytes = 0;
            while (feof(pFileIn) == 0)
            {
                // read one block
                size_t uOffset = buffer.size();
                buffer.resize(uOffset + _uBlockSize);
                size_t nb = fread(buffer.data() + uOffset, 1, _uBlockSize, pFileIn);
                if (nb > 0)
                {
                    // resize to actual size read
                    buffer.resize(uOffset + nb);
                    
                    // process as much of the buffer as possible
                    uOffset = 0;
                    for (;;)
                    {
                        size_t n = _decoder.decodeMsg(buffer.data(), buffer.size(), uOffset);
                        if (n > 0)
                        {
                            uOffset += n;
                        }
                        else
                        {
                            break;
                        }
                    }
                    
                    // erase processed data (some data may be left at end of buffer; should be quick -- left over data is small)
                    buffer.pop_front(uOffset);
                    uTotalBytes += nb;
                    
                    // report progress
                    _progressCb(uTotalBytes, _decoder);
                }
            }
        }
    }

};	// namespace AIS







#endif // #ifndef AIS_FILE_UTIL_H
