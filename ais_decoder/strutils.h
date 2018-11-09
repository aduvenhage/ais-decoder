#ifndef AIS_STR_UTIL_H
#define AIS_STR_UTIL_H


#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <algorithm>
#include <vector>
#include <memory>




namespace
{
#if !defined(_GNU_SOURCE)
    // from 'https://github.com/freebsd/freebsd/blob/master/lib/libc/string/memrchr.c'
    inline void *memrchr(const void *s, int c, size_t n)
    {
        const unsigned char *cp;
        
        if (n != 0) {
            cp = (unsigned char *)s + n;
            do {
                if (*(--cp) == (unsigned char)c) {
                    return((void *)cp);
                }
            } while (--n != 0);
        }
        
        return nullptr;
    }
#endif
};



namespace AIS
{	
    inline char ascii_toupper(char _ch)
    {
        if (_ch <= 'z' && _ch >= 'a') return _ch - 32;
        else return _ch;
    }
    
    inline int ascii_stricmp(const std::string &_a, const std::string &_b)
    {
        const char *pChA = _a.c_str();
        const char *pChB = _b.c_str();
        
        for (;;)
        {
            char chA = *pChA++;
            char chB = *pChB++;
            
            if ((chA == '\0') && (chB == '\0')) return 0;
            else if (chA == '\0') return -1;
            else if (chB == '\0') return 1;
            
            chA = ascii_toupper(chA);
            chB = ascii_toupper(chB);
            
            if (chA < chB) return -1;
            else if (chA > chB) return 1;
        }
    }
    
    inline bool ascii_isspace(char _ch)
    {
        return (_ch == ' ') || (_ch == '\t') || (_ch == '\n') || (_ch == '\r');
    }
    
    /** strip trailing chars after and including '_chStrip' */
    inline std::string &stripTrailingAll(std::string &_str, char _chStrip)
    {
        const char *pNext = (const char*)memchr(_str.data(), _chStrip, _str.size());
        if (pNext != nullptr)
        {
            _str.resize(pNext - _str.data());
        }
        
        return _str;
    }
	
    /** strip trailing chars after and including '_chStrip' */
    inline std::string stripTrailingAll(const std::string &_str, char _chStrip)
    {
        std::string ret(_str);
        stripTrailingAll((std::string&)ret, _chStrip);
        return ret;
    }
    
    /** strip trailing whitespace */
    inline std::string &stripTrailingWhitespace(std::string &_str)
    {
        while (_str.empty() == false)
        {
            if (ascii_isspace(_str.back()) == true)
            {
                _str.pop_back();
            }
            else
            {
                break;
            }
        }
        
        return _str;
    }
    
    /** strip trailing chars after and including '_chStrip' */
    inline std::string stripTrailingWhitespace(const std::string &_str)
    {
        std::string ret;
        stripTrailingWhitespace((std::string&)ret);
        return ret;
    }
    
    
    
    /// quick string object that just references data from another buffer
    struct StringRef
    {
        static const size_t npos = -1;
        
        StringRef()
            :m_psRef(nullptr),
             m_uSize(0)
        {}
        
        StringRef(const char *_psRef, size_t _uSize)
            :m_psRef(_psRef),
             m_uSize(_uSize)
        {}
        
        const char *data() const {return m_psRef;}
        
        char operator [] (size_t _i) const {
            if (_i < m_uSize) {
                return m_psRef[_i];
            }
            else {
                return 0;
            }
        }

        size_t size() const {return m_uSize;}
        bool empty() const {return m_uSize == 0;}
        void clear() {m_psRef = nullptr; m_uSize = 0;}
        
        const char *begin() const {return m_psRef;}
        const char *end() const {return m_psRef + m_uSize;}
        
        void assign(const char *_psRef, size_t _uSize) {m_psRef = _psRef; m_uSize = _uSize;}
        
        operator std::string () const {
            if (m_uSize > 0) {
                return std::string(m_psRef, m_psRef + m_uSize);
            }
            else {
                return "";
            }
        }
        
        StringRef sub(size_t _i, size_t _n = npos) const {
            
            if ( (_n > 0) &&
                 (_i < m_uSize) )
            {
                if (_i + _n > m_uSize)
                {
                    return StringRef(m_psRef + _i, m_uSize - _i);
                }
                else
                {
                    return StringRef(m_psRef + _i, _n - _i);
                }
            }
            
            return StringRef();
        }

        const char        *m_psRef;
        size_t            m_uSize;
    };

    
    /**
     Lightweight buffer for processing data chunks.
     Internal buffer is allowed to grow, but not shrink.
     This avoids allocation and resize init overheads, if the buffer is reused for multiple chunks.
     */
    struct Buffer
    {
        Buffer()
            :m_uSize(0)
        {}
        
        Buffer(size_t _uReservedSize)
            :m_data(_uReservedSize, 0),
             m_uSize(0)
        {}
        
        const char *data() const {return m_data.data();}
        char *data() {return (char*)m_data.data();}
        
        size_t size() const {return m_uSize;}
        
        void resize(size_t _uSize) {
            m_uSize = _uSize;
            if (m_uSize > m_data.size()) {
                m_data.resize(m_uSize);
            }
        }
        
        void clear() {
            m_uSize = 0;
        }
        
        void append(const char *_pData, size_t _uSize) {
            if ( (_uSize > 0) &&
                 (_pData != nullptr) )
            {
                size_t uOffset = size();
                resize(uOffset + _uSize);
                memcpy(data() + uOffset, _pData, _uSize);
            }
        }
        
        void pop_front(size_t _uSize) {
            if (_uSize < m_uSize) {
                std::memmove((char*)m_data.data(), (char*)m_data.data() + _uSize, m_uSize - _uSize);
                m_uSize -= _uSize;
            }
            else {
                m_uSize = 0;
            }
        }
        
        std::vector<char>       m_data;
        size_t                  m_uSize;
    };
    
    
    /// find the last of '_ch' in _str
    inline size_t findLastOf(const StringRef &_str, char _ch)
    {
        if (_str.size() > 0)
        {
            const char *pNext = (const char *)memrchr(_str.data(), _ch, _str.size());
            if (pNext != nullptr)
            {
                return pNext - _str.data();
            }
        }
        
        return -1;
    }
    
    /// Converts string to an integer. Returns 0 if conversion failed.
    inline int strtoi(const StringRef &_str)
    {
        // \note: this is a bit of a hack (might scan past end of _str, since not zero terminated, but will be terminated by a comma or end-of-line)
        return (int)std::strtol(_str.data(), nullptr, 10);
    }
    
    /// Converts a single-digit string to an integer. Quick and dirty with no error checking, but guaranteed to at
    /// least clamp the result to the range [0,9]
    inline int single_digit_strtoi(const StringRef &_str)
    {
        return ((_str.data()[0] - '0') & 0x0f) % 10;
    }
    
    /**
         Appends first line of text from input, starting at _uOffset (works with <LF> "\n" and <CR><LF> "\r\n").
         Returns the number of bytes processed (output includes CR & LF chars).
     */
    inline size_t getLine(StringRef &_strOutput, const char *_pInput, size_t _uInputSize, size_t _uOffset)
    {
        const size_t n = _uInputSize - _uOffset;
        const char *pData = _pInput + _uOffset;

        // find NL/LF
        const char* sentinel = pData + n;
        const char* next = (const char*)memchr(pData, '\n', n);
        
        if (next == nullptr || next >= sentinel) {
            return 0;
        } else {
            // \note getLine() output includes <CR> and <LF> chars
            int nb = next - pData + 1;
            _strOutput.m_psRef = pData;
            _strOutput.m_uSize = nb;
            
            return nb;
        }

        return 0;
    }
    
 	/**
         Separate input string into words using commas.
         The output vector is not resized and this function will not return more words than the size of the output.
         Returns the number of words added to output, starting at index 0.
     */
    template <typename output_t>
    size_t seperate(output_t &_output, const StringRef &_strInput)
	{
        const char *pCh = _strInput.data();
        const char *pChEnd = pCh + _strInput.size();
        size_t uWordCount = 0;
        
        while ( (pCh < pChEnd) &&
                (uWordCount < _output.size()) ) {
                
            const char* next =  (const char*)memchr(pCh, ',', pChEnd - pCh);
            if (next == nullptr || next > pChEnd) {
                // no comma found, assume we are in the last word
                next = pChEnd;
            }
            
            _output[uWordCount].assign(pCh, next - pCh);
            uWordCount++;

            pCh = next + 1; // continue after comma
        }
        
        return uWordCount;
	}
    
    
};	// namespace AIS







#endif // #ifndef AIS_STR_UTIL_H
