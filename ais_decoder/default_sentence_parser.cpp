
#include "default_sentence_parser.h"

#include <array>


using namespace AIS;



/*
 Called to find NMEA start (scan past any headers, META data, etc.; returns NMEA payload)
 This implementation will scan past META data that start and end with a '\'.  It will also stop at NMEA CRC.
 
 */
StringRef DefaultSentenceParser::onScanForNmea(const StringRef &_strSentence) const
{
    const char *pPayloadStart = _strSentence.data();
    size_t uPayloadSize = _strSentence.size();
    
    // check for common META data block headers
    const char *pCh = pPayloadStart;
    if (*pCh == '\\')
    {
        // find META data block end
        pCh = (const char*)memchr(pCh + 1, '\\', uPayloadSize - 1);
        if (pCh != nullptr)
        {
            pPayloadStart = pCh + 1;
            uPayloadSize = _strSentence.size() - (pPayloadStart - _strSentence.data());
        }
        else
        {
            uPayloadSize = 0;
        }
    }
    
    // find payload size (using crc '*' plus 2 chars for crc value)
    pCh = (const char*)memchr(pPayloadStart, '*', uPayloadSize);
    if (pCh != nullptr)
    {
        uPayloadSize = pCh + 3 - pPayloadStart;
    }
    
    return StringRef(pPayloadStart, uPayloadSize);
}

/* calc header string from original line and extracted NMEA payload */
StringRef DefaultSentenceParser::getHeader(const StringRef &_strLine, const StringRef &_strNmea) const
{
    if (_strLine.size() > _strNmea.size())
    {
        StringRef pHeader = _strLine.sub(0, _strNmea.data() - _strLine.data());
        
        // remove last '\'
        if ( (pHeader.empty() == false) &&
            (pHeader[pHeader.size() - 1] == '\\') )
        {
            pHeader.m_uSize--;
        }
        
        // remove first '\\'
        if ( (pHeader.empty() == false) &&
            (pHeader[0] == '\\') )
        {
            pHeader.m_psRef++;
            pHeader.m_uSize--;
        }
        
        return pHeader;
    }
    else
    {
        return StringRef(_strLine.data(), 0);
    }
}

/* calc footer string from original line and extracted NMEA payload */
StringRef DefaultSentenceParser::getFooter(const StringRef &_strLine, const StringRef &_strNmea) const
{
    // NOTE: '_strLine' will end with <CR><LF> or <LF>
    if (_strLine.size() > _strNmea.size())
    {
        StringRef strFooter(_strNmea.data() + _strNmea.size(),
                            _strLine.size() - (_strNmea.data() + _strNmea.size() - _strLine.data()) - 1);
        
        // remove last '<CR>'
        if ( (strFooter.empty() == false) &&
            (strFooter[strFooter.size() - 1] == '\r') )
        {
            strFooter.m_uSize--;
        }
        
        // remove first ','
        if ( (strFooter.empty() == false) &&
            (strFooter[0] == ',') )
        {
            strFooter.m_psRef++;
            strFooter.m_uSize--;
        }
        
        return strFooter;
    }
    else
    {
        return StringRef(_strLine.data(), 0);
    }
}

/* extracts the timestamp from the meta info */
uint64_t DefaultSentenceParser::getTimestamp(const AIS::StringRef &_strHeader, const AIS::StringRef &_strFooter) const
{
    // try to get timestamp from header
    // NOTE: assumes header has comma seperated fields with 'c:' identifying unix timestamp
    uint64_t uTimestamp = 0;
    if (_strHeader.size() > 0)
    {
        // seperate header into words
        std::array<AIS::StringRef, 8> words;
        size_t n = AIS::seperate(words, _strHeader);
        
        // find timestamp
        for (size_t i = 0; i < n; i++)
        {
            const auto &word = words[i];
            if ( (word.empty() == false) &&
                (word[0] == 'c') )
            {
                uTimestamp = (uint64_t)std::strtoull(word.data()+2, nullptr, 10);
            }
        }
    }
    
    // try to get timestamp from footer
    // NOTE: assumes footer first word as timestamp
    if (_strFooter.empty() == false)
    {
        uTimestamp = (uint64_t)std::strtoull(_strFooter.data()+1, nullptr, 10);
    }
    
    return uTimestamp;
}
