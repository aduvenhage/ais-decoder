#ifndef AIS_FILE_UTIL_H
#define AIS_FILE_UTIL_H


#include "ais_decoder.h"

#include <fstream>
#include <vector>
#include <string>
#include <cstring>


namespace AIS
{

    /// Process as much of the data in buffer as possible. Data used is erased from buffer and data not processed is left at front of buffer.
    inline void processAisBuffer(AIS::AisDecoder &_decoder, const SentenceParser &_parser, Buffer &_buffer)
    {
        size_t uOffset = 0;
        for (;;)
        {
            // NOTE: have to keep on calling until return is 0, to force multi-line sentence backup
            size_t n = _decoder.decodeMsg(_buffer.data(), _buffer.size(), uOffset, _parser);
            if (n > 0)
            {
                uOffset += n;
            }
            else    // n == 0
            {
                break;
            }
        }
        
        // erase all data that was processed (some unprocessed data may remain in the buffer)
        _buffer.pop_front(uOffset);
    }
    
    
    /**
     Read and process a file in blocks of _uBlockSize, using the provided decoder.
     
     _progressCb is called after the processing of each block. It should have the signature: func(size_t _uTotalBytesReadFromFile, const AIS::AisDecoder &_decoder)
     
     NOTE: will throw std::exception on file open/read error.
     
     */
    template <typename progress_func_t>
    void processAisFile(const std::string &_strLogPath, AIS::AisDecoder &_decoder, const SentenceParser &_parser, size_t _uBlockSize, progress_func_t &_progressCb)
    {
        Buffer buffer(_uBlockSize + 512);
        
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
                    processAisBuffer(_decoder, _parser, buffer);
                    
                    // report progress
                    _progressCb(uTotalBytes, _decoder);
                    uTotalBytes += nb;
                }
            }
            
            fclose(pFileIn);
        }
        else
        {
            throw std::runtime_error("Failed to open file '" + _strLogPath + "' for reading!");
        }
    }

};	// namespace AIS







#endif // #ifndef AIS_FILE_UTIL_H
