
#include "default_sentence_parser.h"

#include <array>
#include <cstring>


using namespace AIS;



/*
 Called to find NMEA start (scan past any headers, META data, etc.; returns NMEA payload)
 This implementation will scan past META data that start and end with a '\'.  It will also stop at NMEA CRC.
 \s:66,c:1572472771*3D\!AIVDM,1,1,,,19mKBo0P0>SlaQolmBLI9wvn0<00,0*76\n
 * 
 * returns 
 * 	pPayloadStart = where the AIS NMEA starts
 * 	uPayloadSize = The length of the Header
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
    else 
    {
		if (std::strncmp(pCh, "$P", 2) == 0)
		{
			uPayloadSize = 0;
		} 
		else 
		{
			const char *up_start = (const char*)memchr(pCh + 1, '\\', uPayloadSize - 1);
			
			if (up_start != nullptr) {
				// find META data block end; next '\' character
			    pCh = (const char*)memchr(up_start + 1, '\\', uPayloadSize - 1 - (up_start - _strSentence.data()));
			    
			    // if second forward slash exists: 
			    if (pCh != nullptr)
				{
					pPayloadStart = pCh + 1;
					uPayloadSize = _strSentence.size() - (pPayloadStart - _strSentence.data());
				}
				// if no second forward slash exists...
				else
				{
					pPayloadStart = up_start + 1;
					uPayloadSize = _strSentence.size() - (pPayloadStart - _strSentence.data());
				}
			} else {
				uPayloadSize = 0;
			}
			
		}
			
	}

    // find payload size (using crc '*' plus 2 chars for crc value)
    if (uPayloadSize > 0)
    {
        pCh = (const char*)memchr(pPayloadStart, '*', uPayloadSize);
        if (pCh != nullptr)
        {
            uPayloadSize = pCh + 3 - pPayloadStart;
        }
        else
        {
            uPayloadSize = 0;
        }
    }
    
    return StringRef(pPayloadStart, uPayloadSize);
}

/* calc header string from original line and extracted NMEA payload */
StringRef DefaultSentenceParser::getHeader(const StringRef &_strLine, const StringRef &_strNmea) const
{
    StringRef strHeader(_strLine.data(), 0);
    
    if (_strNmea.data() > _strLine.data())
    {
        strHeader = _strLine.substr(0, _strNmea.data() - _strLine.data());
        
        // remove last '\'
        if ( (strHeader.empty() == false) &&
             (strHeader[strHeader.size() - 1] == '\\') )
        {
            strHeader.remove_suffix(1);
        }
        
        // remove first '\\'
        if ( (strHeader.empty() == false) &&
             (strHeader[0] == '\\') )
        {
            strHeader.remove_prefix(1);
        }
    }
    
    return strHeader;
}

/* calc footer string from original line and extracted NMEA payload */
StringRef DefaultSentenceParser::getFooter(const StringRef &_strLine, const StringRef &_strNmea) const
{
    StringRef strFooter(_strLine.data(), 0);
    
    const char *pLineEnd = _strLine.data() + _strLine.size();
    const char *pNmeaEnd = _strNmea.data() + _strNmea.size();
    
    if (pLineEnd > pNmeaEnd)
    {
        // NOTE: '_strLine' will end with <CR><LF> or <LF>
        strFooter = StringRef(pNmeaEnd, pLineEnd - pNmeaEnd - 1);
        
        // remove last '<CR>'
        if ( (strFooter.empty() == false) &&
             (strFooter[strFooter.size() - 1] == '\r') )
        {
            strFooter.remove_suffix(1);
        }
        
        // remove first ','
        if ( (strFooter.empty() == false) &&
             (strFooter[0] == ',') )
        {
            strFooter.remove_prefix(1);
        }
    }
    
    return strFooter;
}

/* extracts the timestamp from the meta info */
uint64_t DefaultSentenceParser::getTimestamp(const AIS::StringRef &_strHeader, const AIS::StringRef &_strFooter) const
{
    uint64_t uTimestamp = 0;
    
    // try to get timestamp from header
    // NOTE: assumes header has comma seperated fields with 'c:' identifying unix timestamp
    if (_strHeader.size() > 0)
    {
        // seperate header into words
        std::array<AIS::StringRef, 8> words;
        size_t n = AIS::seperate<','>(words, _strHeader);
        
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
    if ( (_strFooter.empty() == false) &&
         (uTimestamp == 0) )
    {
        uTimestamp = (uint64_t)std::strtoull(_strFooter.data()+1, nullptr, 10);
    }
    
    return uTimestamp;
}
