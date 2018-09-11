#ifndef AIS_STR_UTIL_H
#define AIS_STR_UTIL_H


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <algorithm>
#include <vector>
#include <regex>
#include <array>
#include <memory>
#include <sstream>



// define own version of va_copy on MSVC platforms (only VS2013+ seems to have moved to the new VA standard)
#if defined(_MSC_VER) && (_MSC_VER < 1800) || defined(__BCPLUSPLUS__)
#ifndef va_copy
#define va_copy(dst, src) ((void)((dst) = (src)))
#endif
#endif

// define snprintf on MSVC platforms
#if defined(_MSC_VER)
#ifndef snprintf
#define snprintf _snprintf
#endif
#endif

// define own version of vsnprintf on MSVC platforms
#if defined(_MSC_VER)
#define vsnprintf_(s, n, format, arg) vsnprintf_s(s, (n)+1, (n), format, arg)
#else
#define vsnprintf_(s, n, format, arg) vsnprintf(s, (n), format, arg)
#endif



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
        const char *pCh = _str.data();
        const size_t n = _str.size();
        
        for (size_t i = 0; i < n; i++)
        {
            const char ch = *pCh++;
            if (ch == _chStrip)
            {
                _str.resize(i);
                break;
            }
        }
        
        return _str;
    }
	
    /** strip trailing chars after and including '_chStrip' */
    inline std::string stripTrailingAll(const std::string &_str, char _chStrip)
    {
        std::string ret;
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

        size_t size() const {return m_uSize;}
        bool empty() const {return m_uSize == 0;}
        
        const char *begin() const {return m_psRef;}
        const char *end() const {return m_psRef + m_uSize;}
        
        StringRef &assign(const char *_psRef, size_t _uSize) {m_psRef = _psRef; m_uSize = _uSize; return *this;}
        
        operator std::string () const {return std::string(m_psRef, m_psRef + m_uSize);}

        const char        *m_psRef;
        size_t            m_uSize;
    };

    
    /// find the last of '_ch' in _str
    inline size_t findLastOf(const StringRef &_str, char _ch)
    {
        if (_str.size() > 0)
        {
            const char *pCh = _str.data() + _str.size();
            while (pCh > _str.data())
            {
                const char ch = *pCh;
                if (ch == _ch)
                {
                    return pCh - _str.data();
                }
                
                pCh--;
            }
        }
        
        return -1;
    }
    
    /// Converts string to an integer. Returns 0 if conversion failed.
    inline int strtoi(const StringRef &_str)
    {
        // \todo: this is a bit of a hack, since strtol might scan past end of _str (not zero terminated)
        return (int)std::strtol(_str.data(), nullptr, 10);
    }
    
    /**
         Appends first line of text from input, starting at _uOffset (works with "\n" and "\r\n").
         Returns the number of bytes processed (EOL chars are not included in output, but counted).
     */
    inline size_t getLine(StringRef &_strOutput, const char *_pInput, size_t _uInputSize, size_t _uOffset)
    {
        size_t chCount = 0;
        const size_t n = _uInputSize - _uOffset;
        const char *pData = _pInput + _uOffset;
        const char *pCh = pData;
        
        for (size_t i = 0; i < n; i++)
        {
            const char ch = *pCh++;

            if (ch == '\n')
            {
                if (chCount > 0)
                {
                    _strOutput = StringRef(pData, chCount);
                }
                
                return i + 1;
            }
            else if (ch != '\r')
            {
                chCount++;
            }
        }

        return 0;
    }
    
    
 	/**
         Separate input string into words using commas.
         Removes spaces from start and end of words.
     */
    inline size_t seperate(std::vector<StringRef> &_output, const StringRef &_strInput)
	{
        const char *pWordStart = _strInput.data();
        const char *pWordEnd = pWordStart;
        const char *pCh = pWordStart;
        const char *pChEnd = pWordStart + _strInput.size();
        size_t uWordCount = 0;

        while ( (pCh < pChEnd) &&
                (uWordCount < _output.size()) )
        {
            const char ch = *pCh;
            
            if ( (pCh == pWordStart) &&
                 (ch == ' ') )
            {
                pCh++;
                pWordEnd++;
                pWordStart++;
            }
            else if (ch == ',')
            {
                _output[uWordCount].assign(pWordStart, pWordEnd - pWordStart);
                uWordCount++;

                pCh++;
                pWordStart = pCh;
                pWordEnd = pCh;
            }
            else
            {
                pCh++;
                
                if (ch != ' ')
                {
                    pWordEnd = pCh;
                }
            }
        }
        
        if (pWordEnd > pWordStart)
        {
            _output[uWordCount].assign(pWordStart, pWordEnd - pWordStart);
            uWordCount++;
        }
        
        return uWordCount;
	}
    
    
};	// namespace AIS







#endif // #ifndef AIS_STR_UTIL_H
