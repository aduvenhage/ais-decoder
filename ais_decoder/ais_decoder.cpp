
#include "ais_decoder.h"
#include "strutils.h"



using namespace AIS;


/// private / detail namespace
namespace
{
    /// calc header string from original line and extracted NMEA payload
    StringRef getHeader(const StringRef &_strLine, const StringRef &_strNmea)
    {
        if (_strLine.size() > _strNmea.size())
        {
            return _strLine.sub(0, _strNmea.data() - _strLine.data());
        }
        else
        {
            return StringRef(_strLine.data(), 0);
        }
    }
    
    /// calc footer string from original line and extracted NMEA payload
    StringRef getFooter(const StringRef &_strLine, const StringRef &_strNmea)
    {
        // NOTE: '_strLine' will end with <CR><LF> or <LF>
        if (_strLine.size() > _strNmea.size())
        {
            StringRef strFooter(_strNmea.data() + _strNmea.size(),
                                _strLine.size() - (_strNmea.data() + _strNmea.size() - _strLine.data()) - 1);
            
            // remove last '<CR>'
            if (strFooter.empty() == false)
            {
                const char *pCh = strFooter.data() + strFooter.size() - 1;
                if (*pCh == '\r')
                {
                    strFooter.m_uSize--;
                }
            }
            
            return strFooter;
        }
        else
        {
            return StringRef(_strLine.data(), 0);
        }
    }
}


PayloadBuffer::PayloadBuffer()
    :m_data{},
     m_iBitIndex(0)
{}

/* set bit index back to zero */
void PayloadBuffer::resetBitIndex()
{
    m_iBitIndex = 0;
}

/* unpack next _iBits (most significant bit is packed first) */
unsigned int PayloadBuffer::getUnsignedValue(int _iBits)
{  
    const unsigned char *lptr = &m_data[m_iBitIndex >> 3];
    uint64_t bits;
    
    bits = (uint64_t)lptr[0] << 40;
    bits |= (uint64_t)lptr[1] << 32;
    
    if (_iBits > 9) {
        bits |= (unsigned int)lptr[2] << 24;
        bits |= (unsigned int)lptr[3] << 16;
        bits |= (unsigned int)lptr[4] << 8;
        bits |= (unsigned int)lptr[5];
    }
    
    bits <<= 16 + (m_iBitIndex & 7);
    m_iBitIndex += _iBits;

    return (unsigned int)(bits >> (64 - _iBits));
}

/* unpack next _iBits (most significant bit is packed first; with sign check/conversion) */
int PayloadBuffer::getSignedValue(int _iBits)
{
    const unsigned char *lptr = &m_data[m_iBitIndex >> 3];
    uint64_t bits;

    bits = (uint64_t)lptr[0] << 40;
    bits |= (uint64_t)lptr[1] << 32;
    
    if (_iBits > 9) {
        bits |= (unsigned int)lptr[2] << 24;
        bits |= (unsigned int)lptr[3] << 16;
        bits |= (unsigned int)lptr[4] << 8;
        bits |= (unsigned int)lptr[5];
    }
    
    bits <<= 16 + (m_iBitIndex & 7);
    m_iBitIndex += _iBits;

    return (int)((int64_t)bits >> (64 - _iBits));
}

/* unback string (6 bit characters) -- already cleans string (removes trailing '@' and trailing spaces) */
std::string PayloadBuffer::getString(int _iNumBits)
{
    std::string ret;
    int iNumChars = _iNumBits/6;
    ret.reserve(iNumChars);
    
    int32_t iStartBitIndex = m_iBitIndex;
    
    for (int i = 0; i < iNumChars; i++)
    {
        int ch = getUnsignedValue(6);
        
        if (ch > 0) // stop on '@'
        {
            ret.push_back(ASCII_CHARS[ch & 0x3F]);
        }
        else
        {
            break;
        }
    }
    
    // remove trailing spaces
    stripTrailingWhitespace(ret);
    
    // make sure bit index is correct
    m_iBitIndex = iStartBitIndex + _iNumBits;
    
    return ret;
}

/* convert payload to decimal (de-armour) and concatenate 6bit decimal values into payload buffer */
int AIS::decodeAscii(PayloadBuffer &_buffer, const StringRef &_strPayload, int _iFillBits)
{
    static const unsigned char dLUT[256] = {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
        17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,33,34,35,36,37,38,39,40,
        41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    };
    
    const unsigned char* in_ptr = (unsigned char*)_strPayload.data();
    const unsigned char* in_sentinel = in_ptr + _strPayload.size();
    const unsigned char* in_sentinel4 = in_sentinel - 4;
    unsigned char* out_ptr = _buffer.getData();
    
    uint64_t accumulator = 0;
    unsigned int acc_bitcount = 0;
    
    while (in_ptr < in_sentinel4) {
        uint64_t val = (dLUT[*in_ptr] << 18) | (dLUT[*(in_ptr+1)] << 12) | (dLUT[*(in_ptr+2)] << 6) | dLUT[*(in_ptr+3)];
        
        constexpr unsigned int nbits = 24;
        
        int remainder = int(64 - acc_bitcount) - nbits;
        if (remainder <= 0) {
            // accumulator will fill up, commit to output buffer
            accumulator |= (uint64_t(val) >> -remainder);
            *((uint64_t*)out_ptr) = __builtin_bswap64(accumulator);
            out_ptr += 8;
            
            if (remainder < 0) { 
                accumulator = uint64_t(val) << (64 + remainder); // remainder is negative
                acc_bitcount = -remainder;
            } else {
                accumulator = 0;  // shifting right by 64 bits (above) does not yield zero?
                acc_bitcount = 0; 
            }
            
        } else {
            // we still have enough room in the accumulator
            accumulator |= uint64_t(val) << (64 - acc_bitcount - nbits);
            acc_bitcount += nbits;
        }
        
        in_ptr += 4;
    }
    
    while (in_ptr < in_sentinel) {
        uint64_t val = dLUT[*in_ptr];
        
        constexpr unsigned int nbits = 6;
        
        int remainder = int(64 - acc_bitcount) - nbits;
        if (remainder <= 0) {
            // accumulator will fill up, commit to output buffer
            accumulator |= (uint64_t(val) >> -remainder);
            *((uint64_t*)out_ptr) = __builtin_bswap64(accumulator);
            out_ptr += 8;
            
            if (remainder < 0) { 
                accumulator = uint64_t(val) << (64 + remainder); // remainder is negative
                acc_bitcount = -remainder;
            } else {
                accumulator = 0;  // shifting right by 64 bits (above) does not yield zero?
                acc_bitcount = 0; 
            }
            
        } else {
            // we still have enough room in the accumulator
            accumulator |= uint64_t(val) << (64 - acc_bitcount - nbits);
            acc_bitcount += nbits;
        }
        
        in_ptr++;
    }
    *((uint64_t*)out_ptr) = __builtin_bswap64(accumulator);
    
    return (int)(_strPayload.size() * 6 - _iFillBits);
}


/* calc CRC */
uint8_t AIS::crc(const StringRef &_strNmea)
{
    const unsigned char* in_ptr = (const unsigned char*)_strNmea.data();
    const unsigned char* in_sentinel  = in_ptr + _strNmea.size();
    const unsigned char* in_sentinel4 = in_sentinel - 4;
    
    uint8_t checksum = 0;
    while ((intptr_t(in_ptr) & 3) && in_ptr < in_sentinel) {
        checksum ^= *in_ptr++;
    }
    
    uint32_t checksum4 = checksum;
    while (in_ptr < in_sentinel4) {
        checksum4 ^= *((uint32_t*)in_ptr);
        in_ptr += 4;
    }
    
    checksum = (checksum4 & 0xff) ^ ((checksum4 >> 8) & 0xff) ^ ((checksum4 >> 16) & 0xff) ^ ((checksum4 >> 24) & 0xff);
    
    while (in_ptr < in_sentinel) {
        checksum ^= *in_ptr++;
    }
    
    return checksum;

}

/*
 Default implementation to scan through a sentence and extract NMEA string.
 Look at 'onScanForNmea' user defined method on AisDecoder class.
 
 This implementation will scan past META data that start and end with a '\'.  It will also stop at NMEA CRC.
 
 */
StringRef AIS::defaultScanForNmea(const StringRef &_strSentence)
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




MultiSentence::MultiSentence(int _iFragmentCount, const StringRef &_strFirstFragment, const StringRef &_strHeader, const StringRef &_strFooter)
    :m_iFragmentCount(_iFragmentCount),
     m_iFragmentNum(1)
{
    m_vecStrData.reserve(_strHeader.size() + _strFooter.size() + MAX_FRAGMENTS * MAX_CHARS_PER_FRAGMENT);
    
    // add string data to internal buffer
    if (_strHeader.size() > 0) {
        m_vecStrData.insert(m_vecStrData.end(), _strHeader.data(), _strHeader.data() + _strHeader.size());
    }
    
    if (_strFooter.size() > 0) {
        m_vecStrData.insert(m_vecStrData.end(), _strFooter.data(), _strFooter.data() + _strFooter.size());
    }
    
    m_vecStrData.insert(m_vecStrData.end(), _strFirstFragment.data(), _strFirstFragment.data() + _strFirstFragment.size());

    // setup string references
    m_strHeader = StringRef(m_vecStrData.data(), _strHeader.size());
    m_strFooter = StringRef(m_vecStrData.data() + _strHeader.size(), _strFooter.size());
    m_strPayload = StringRef(m_vecStrData.data() + _strHeader.size() + _strFooter.size(), _strFirstFragment.size());
}

bool MultiSentence::addFragment(int _iFragmentNum, const StringRef &_strFragment)
{
    // check that fragments are added in order (out of order is an error)
    if (m_iFragmentNum == _iFragmentNum-1)
    {
        // append data
        m_vecStrData.insert(m_vecStrData.end(), _strFragment.data(), _strFragment.data() + _strFragment.size());
        m_iFragmentNum++;

        // setup string references
        m_strHeader = StringRef(m_vecStrData.data(), m_strHeader.size());
        m_strFooter = StringRef(m_vecStrData.data() + m_strHeader.size(), m_strFooter.size());
        m_strPayload = StringRef(m_vecStrData.data() + m_strHeader.size() + m_strFooter.size(), m_vecStrData.size() - m_strHeader.size() - m_strFooter.size());
        
        return true;
    }
    else
    {
        return false;
    }
}

bool MultiSentence::isComplete() const
{
    return m_iFragmentCount == m_iFragmentNum;
}



AisDecoder::AisDecoder(int _iIndex)
    :m_iIndex(_iIndex),
     m_multiSentences{},
     m_words(MAX_MSG_WORDS),
     m_msgCounts{},
     m_uTotalMessages(0),
     m_uTotalBytes(0),
     m_uCrcErrors(0),
     m_uDecodingErrors(0),
     m_vecMsgCallbacks{}
{
    // attach callbacks to relevant message types
    // NOTE: some callbacks attach to multiple message types
    m_vecMsgCallbacks[1] = &AisDecoder::decodeType123;
    m_vecMsgCallbacks[2] = &AisDecoder::decodeType123;
    m_vecMsgCallbacks[3] = &AisDecoder::decodeType123;
    m_vecMsgCallbacks[4] = &AisDecoder::decodeType411;
    m_vecMsgCallbacks[5] = &AisDecoder::decodeType5;
    m_vecMsgCallbacks[9] = &AisDecoder::decodeType9;
    m_vecMsgCallbacks[11] = &AisDecoder::decodeType411;
    m_vecMsgCallbacks[18] = &AisDecoder::decodeType18;
    m_vecMsgCallbacks[19] = &AisDecoder::decodeType19;
    m_vecMsgCallbacks[21] = &AisDecoder::decodeType21;
    m_vecMsgCallbacks[24] = &AisDecoder::decodeType24;
    m_vecMsgCallbacks[27] = &AisDecoder::decodeType27;
}

/* decode Position Report (class A) */
void AisDecoder::decodeType123(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits)
{
    if (_iPayloadSizeBits < 168)
    {
        throw std::runtime_error("Invalid payload size (" + std::to_string(_iPayloadSizeBits) + " bits) for messag type " + std::to_string(_uMsgType));
    }
    
    // decode message fields (binary buffer has to go through all fields, but some fields are not used)
    _buffer.getUnsignedValue(2);                 // repeatIndicator
    auto mmsi = _buffer.getUnsignedValue(30);
    auto navstatus = _buffer.getUnsignedValue(4);
    auto rot = _buffer.getSignedValue(8);
    auto sog = _buffer.getUnsignedValue(10);
    auto posAccuracy = _buffer.getBoolValue();
    auto posLon = _buffer.getSignedValue(28);
    auto posLat = _buffer.getSignedValue(27);
    auto cog = (int)_buffer.getUnsignedValue(12);
    auto heading = (int)_buffer.getUnsignedValue(9);
    
    _buffer.getUnsignedValue(6);     // timestamp
    _buffer.getUnsignedValue(2);     // maneuver indicator
    _buffer.getUnsignedValue(3);     // spare
    _buffer.getBoolValue();          // RAIM
    _buffer.getUnsignedValue(19);     // radio status

    onType123(_uMsgType, mmsi, navstatus, rot, sog, posAccuracy, posLon, posLat, cog, heading);
}

/* decode Base Station Report (type nibble already pulled from buffer) */
void AisDecoder::decodeType411(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits)
{
    if (_iPayloadSizeBits < 168)
    {
        throw std::runtime_error("Invalid payload size (" + std::to_string(_iPayloadSizeBits) + " bits) for messag type " + std::to_string(_uMsgType));
    }

    // decode message fields (binary buffer has to go through all fields, but some fields are not used)
    _buffer.getUnsignedValue(2);                 // repeatIndicator
    auto mmsi = _buffer.getUnsignedValue(30);
    auto year = _buffer.getUnsignedValue(14);    // year UTC, 1-9999, 0 = N/A
    auto month = _buffer.getUnsignedValue(4);    // month (1-12), 0 = N/A
    auto day = _buffer.getUnsignedValue(5);      // day (1-31), 0 = N/A
    auto hour = _buffer.getUnsignedValue(5);     // hour (0 - 23), 24 = N/A
    auto minute = _buffer.getUnsignedValue(6);   // minute (0-59), 60 = N/A
    auto second = _buffer.getUnsignedValue(6);   // second 0-59, 60 = N/A
    auto posAccuracy = _buffer.getBoolValue();
    auto posLon = _buffer.getSignedValue(28);
    auto posLat = _buffer.getSignedValue(27);
    
    _buffer.getUnsignedValue(4);     // epfd type
    _buffer.getUnsignedValue(10);    // spare
    _buffer.getBoolValue();          // RAIM
    _buffer.getUnsignedValue(19);    // radio status
    
    onType411(_uMsgType, mmsi, year, month, day, hour, minute, second, posAccuracy, posLon, posLat);
}

/* decode Voyage Report (type nibble already pulled from buffer) */
void AisDecoder::decodeType5(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits)
{
    if (_iPayloadSizeBits < 420)
    {
        throw std::runtime_error("Invalid payload size (" + std::to_string(_iPayloadSizeBits) + " bits) for messag type " + std::to_string(_uMsgType));
    }
    
    // decode message fields (binary buffer has to go through all fields, but some fields are not used)
    _buffer.getUnsignedValue(2);                 // repeatIndicator
    auto mmsi = _buffer.getUnsignedValue(30);
    _buffer.getUnsignedValue(2);                 // AIS version
    auto imo = _buffer.getUnsignedValue(30);
    auto callsign = _buffer.getString(42);
    auto name = _buffer.getString(120);
    auto type = _buffer.getUnsignedValue(8);
    if (type > 99) {
        type = 0;
    }
    
    auto toBow = _buffer.getUnsignedValue(9);
    auto toStern = _buffer.getUnsignedValue(9);
    auto toPort = _buffer.getUnsignedValue(6);
    auto toStarboard = _buffer.getUnsignedValue(6);
    auto fixType = _buffer.getUnsignedValue(4);
    auto etaMonth = _buffer.getUnsignedValue(4);    // month (1-12), 0 = N/A
    auto etaDay = _buffer.getUnsignedValue(5);      // day (1-31), 0 = N/A
    auto etaHour = _buffer.getUnsignedValue(5);     // hour (0 - 23), 24 = N/A
    auto etaMinute = _buffer.getUnsignedValue(6);   // minute (0-59), 60 = N/A
    auto draught = _buffer.getUnsignedValue(8);
    auto destination = _buffer.getString(120);
    
    _buffer.getBoolValue();                         // dte
    _buffer.getUnsignedValue(1);                    // spare

    onType5(mmsi, imo, callsign, name, type, toBow, toStern, toPort, toStarboard, fixType, etaMonth, etaDay, etaHour, etaMinute, draught, destination);
}

/* decode Standard SAR Aircraft Position Report */
void AisDecoder::decodeType9(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits)
{
    if (_iPayloadSizeBits < 168)
    {
        throw std::runtime_error("Invalid payload size (" + std::to_string(_iPayloadSizeBits) + " bits) for messag type " + std::to_string(_uMsgType));
    }
    
    // decode message fields (binary buffer has to go through all fields, but some fields are not used)
    _buffer.getUnsignedValue(2);                 // repeatIndicator
    auto mmsi = _buffer.getUnsignedValue(30);
    auto altitude = _buffer.getUnsignedValue(12);
    auto sog = _buffer.getUnsignedValue(10);
    auto posAccuracy = _buffer.getBoolValue();
    auto posLon = _buffer.getSignedValue(28);
    auto posLat = _buffer.getSignedValue(27);
    auto cog = (int)_buffer.getUnsignedValue(12);
    
    _buffer.getUnsignedValue(6);     // timestamp
    _buffer.getUnsignedValue(8);     // reserved
    _buffer.getBoolValue();          // dte
    _buffer.getUnsignedValue(3);     // spare
    _buffer.getBoolValue();          // assigned
    _buffer.getBoolValue();          // RAIM
    _buffer.getUnsignedValue(20);    // radio status

    onType9(mmsi, sog, posAccuracy, posLon, posLat, cog, altitude);
}

/* decode Position Report (class B; type nibble already pulled from buffer) */
void AisDecoder::decodeType18(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits)
{
    if (_iPayloadSizeBits < 168)
    {
        throw std::runtime_error("Invalid payload size (" + std::to_string(_iPayloadSizeBits) + " bits) for messag type " + std::to_string(_uMsgType));
    }
    
    // decode message fields (binary buffer has to go through all fields, but some fields are not used)
    _buffer.getUnsignedValue(2);                 // repeatIndicator
    auto mmsi = _buffer.getUnsignedValue(30);
    _buffer.getUnsignedValue(8);                 // reserved
    auto sog = _buffer.getUnsignedValue(10);
    auto posAccuracy = _buffer.getBoolValue();
    auto posLon = _buffer.getSignedValue(28);
    auto posLat = _buffer.getSignedValue(27);
    auto cog = (int)_buffer.getUnsignedValue(12);
    auto heading = (int)_buffer.getUnsignedValue(9);
    
    _buffer.getUnsignedValue(6);     // timestamp
    _buffer.getUnsignedValue(2);     // reserved
    _buffer.getBoolValue();          // cs unit
    _buffer.getBoolValue();          // display
    _buffer.getBoolValue();          // dsc
    _buffer.getBoolValue();          // band
    _buffer.getBoolValue();          // msg22
    _buffer.getBoolValue();          // assigned
    _buffer.getBoolValue();          // RAIM
    _buffer.getUnsignedValue(20);     // radio status
    
    onType18(mmsi, sog, posAccuracy, posLon, posLat, cog, heading);
}

/* decode Position Report (class B; type nibble already pulled from buffer) */
void AisDecoder::decodeType19(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits)
{
    if (_iPayloadSizeBits < 312)
    {
        throw std::runtime_error("Invalid payload size (" + std::to_string(_iPayloadSizeBits) + " bits) for messag type " + std::to_string(_uMsgType));
    }
    
    // decode message fields (binary buffer has to go through all fields, but some fields are not used)
    _buffer.getUnsignedValue(2);                 // repeatIndicator
    auto mmsi = _buffer.getUnsignedValue(30);
    _buffer.getUnsignedValue(8);                 // reserved
    auto sog = _buffer.getUnsignedValue(10);
    auto posAccuracy = _buffer.getBoolValue();
    auto posLon = _buffer.getSignedValue(28);
    auto posLat = _buffer.getSignedValue(27);
    auto cog = (int)_buffer.getUnsignedValue(12);
    auto heading = (int)_buffer.getUnsignedValue(9);
    _buffer.getUnsignedValue(6);                 // timestamp
    _buffer.getUnsignedValue(4);                 // reserved
    auto name = _buffer.getString(120);
    auto type = _buffer.getUnsignedValue(8);
    if (type > 99) {
        type = 0;
    }
    
    auto toBow = _buffer.getUnsignedValue(9);
    auto toStern = _buffer.getUnsignedValue(9);
    auto toPort = _buffer.getUnsignedValue(6);
    auto toStarboard = _buffer.getUnsignedValue(6);
    
    _buffer.getUnsignedValue(4);     // fix type
    _buffer.getBoolValue();          // RAIM
    _buffer.getBoolValue();          // dte
    _buffer.getBoolValue();          // assigned
    _buffer.getUnsignedValue(4);     // spare

    onType19(mmsi, sog, posAccuracy, posLon, posLat, cog, heading, name, type, toBow, toStern, toPort, toStarboard);
}

/* decode Aid-to-Navigation Report */
void AisDecoder::decodeType21(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits)
{
    if (_iPayloadSizeBits < 272)
    {
        throw std::runtime_error("Invalid payload size (" + std::to_string(_iPayloadSizeBits) + " bits) for messag type " + std::to_string(_uMsgType));
    }
    
    // decode message fields (binary buffer has to go through all fields, but some fields are not used)
    _buffer.getUnsignedValue(2);                 // repeatIndicator
    auto mmsi = _buffer.getUnsignedValue(30);
    auto aidType = _buffer.getUnsignedValue(5);
    auto name = _buffer.getString(120);
    auto posAccuracy = _buffer.getBoolValue();
    auto posLon = _buffer.getSignedValue(28);
    auto posLat = _buffer.getSignedValue(27);
    auto toBow = _buffer.getUnsignedValue(9);
    auto toStern = _buffer.getUnsignedValue(9);
    auto toPort = _buffer.getUnsignedValue(6);
    auto toStarboard = _buffer.getUnsignedValue(6);
    
    _buffer.getUnsignedValue(4);        // epfd type
    _buffer.getUnsignedValue(6);        // timestamp
    _buffer.getBoolValue();             // off position
    _buffer.getUnsignedValue(8);        // reserved
    _buffer.getBoolValue();             // RAIM
    _buffer.getBoolValue();             // virtual aid
    _buffer.getBoolValue();             // assigned mode
    _buffer.getUnsignedValue(1);        // spare
    
    std::string nameExt;
    if (_iPayloadSizeBits > 272)
    {
        nameExt = _buffer.getString(88);
    }
    
    onType21(mmsi, aidType, name + nameExt, posAccuracy, posLon, posLat, toBow, toStern, toPort, toStarboard);
}

/* decode Voyage Report and Static Data (type nibble already pulled from buffer) */
void AisDecoder::decodeType24(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits)
{
    // decode message fields (binary buffer has to go through all fields, but some fields are not used)
    _buffer.getUnsignedValue(2);                 // repeatIndicator
    auto mmsi = _buffer.getUnsignedValue(30);
    auto partNo = _buffer.getUnsignedValue(2);
    
    // decode part A
    if (partNo == 0)
    {
        if (_iPayloadSizeBits < 160)
        {
            throw std::runtime_error("Invalid payload size (" + std::to_string(_iPayloadSizeBits) + " bits) for messag type " + std::to_string(_uMsgType));
        }
        
        auto name = _buffer.getString(120);
        _buffer.getUnsignedValue(8);            // spare
        
        onType24A(mmsi, name);
    }
    
    // decode part B
    else if (partNo == 1)
    {
        if (_iPayloadSizeBits < 168)
        {
            throw std::runtime_error("Invalid payload size (" + std::to_string(_iPayloadSizeBits) + " bits) for messag type " + std::to_string(_uMsgType));
        }
        
        auto type = _buffer.getUnsignedValue(8);
        if (type > 99) {
            type = 0;
        }
        
        _buffer.getString(18);                       // vendor ID
        _buffer.getUnsignedValue(4);                 // unit model code
        _buffer.getUnsignedValue(20);                // serial number
        auto callsign = _buffer.getString(42);
        auto toBow = _buffer.getUnsignedValue(9);
        auto toStern = _buffer.getUnsignedValue(9);
        auto toPort = _buffer.getUnsignedValue(6);
        auto toStarboard = _buffer.getUnsignedValue(6);
        
        // FvdB: Note that 'Mothership MMSI' field overlaps the previous 4 fields, total message length is 168 bits
        // _buffer.getUnsignedValue(30);                // Mothership MMSI
        
        _buffer.getUnsignedValue(6);        // spare
        
        onType24B(mmsi, callsign, type, toBow, toStern, toPort, toStarboard);
    }
    
    // invalid part
    else
    {
        throw std::runtime_error("Invalid part number (" + std::to_string(partNo) + ") for messag type " + std::to_string(_uMsgType));
    }
}

/* decode Long Range AIS Broadcast message (type nibble already pulled from buffer) */
void AisDecoder::decodeType27(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits)
{
    if (_iPayloadSizeBits < 96)
    {
        throw std::runtime_error("Invalid payload size (" + std::to_string(_iPayloadSizeBits) + " bits) for messag type " + std::to_string(_uMsgType));
    }
    
    // decode message fields (binary buffer has to go through all fields, but some fields are not used)
    _buffer.getUnsignedValue(2);                 // repeatIndicator
    auto mmsi = _buffer.getUnsignedValue(30);
    auto posAccuracy = _buffer.getBoolValue();
    
    _buffer.getBoolValue();         // RAIM
    
    auto navstatus = _buffer.getUnsignedValue(4);
    auto posLon = _buffer.getSignedValue(18);
    auto posLat = _buffer.getSignedValue(17);
    auto sog = _buffer.getUnsignedValue(6);
    auto cog = (int)_buffer.getUnsignedValue(9);
    
    _buffer.getUnsignedValue(1);        // GNSS
    _buffer.getUnsignedValue(1);        // spare

    onType27(mmsi, navstatus, sog, posAccuracy, posLon, posLat, cog);
}

/* decode Mobile AIS station message */
void AisDecoder::decodeMobileAisMsg(const StringRef &_strPayload, int _iFillBits)
{
    // de-armour string and back bits into buffer
    m_binaryBuffer.resetBitIndex();
    int iBitsUsed = decodeAscii(m_binaryBuffer, _strPayload, _iFillBits);
    m_binaryBuffer.resetBitIndex();

    // check message type
    auto msgType = m_binaryBuffer.getUnsignedValue(6);
    if ( (msgType == 0) ||
         (msgType > 27) )
    {
        onDecodeError(_strPayload, "Invalid message type (" + std::to_string(msgType) + ").");
    }
    else
    {
        // decode message
        m_msgCounts[msgType]++;
        m_uTotalMessages++;
    
        auto pFnDecoder = m_vecMsgCallbacks[msgType];
        if (pFnDecoder != nullptr)
        {
            (this->*pFnDecoder)(m_binaryBuffer, msgType, iBitsUsed);
        }
        else
        {
            onNotDecoded(_strPayload, msgType);
        }
    }
}

/* check sentence CRC */
bool AisDecoder::checkCrc(const StringRef &_strPayload)
{
    size_t n = findLastOf(_strPayload, '*');
    if (n != StringRef::npos)
    {
        // for some reason the std::strol function is quite slow, so just conver the checksum manually
        const uint16_t ascii_t[32] = {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
            257, 257, 257, 257, 257, 257, 257,
            10, 11, 12, 13, 14, 15, 
            257, 257, 257, 257, 257, 257, 257, 257, 257
        };
        
        uint16_t iCrc = ascii_t[(_strPayload.data()[n+1] - '0') & 31]*16 + ascii_t[(_strPayload.data()[n+2] - '0') & 31];
        
        if (*_strPayload.data() == '!')
        {
            uint16_t iCrcCalc = (int)AIS::crc(StringRef(_strPayload.data() + 1, n - 1));
            return iCrc == iCrcCalc;
        }
        else
        {
            uint16_t iCrcCalc = (int)AIS::crc(StringRef(_strPayload.data(), n));
            return iCrc == iCrcCalc;
        }
    }
    else
    {
        return false;
    }
}

/* decode next sentence (starts reading from input buffer with the specified offset; returns the number of bytes processed) */
size_t AisDecoder::decodeMsg(const char *_pNmeaBuffer, size_t _uBufferSize, size_t _uOffset)
{
    // process and decode AIS strings
    StringRef strLine;
    size_t n = getLine(strLine, _pNmeaBuffer, _uBufferSize, _uOffset);
    if (n > 0)
    {
        // provide raw data back to user
        onSentence(strLine);
        
        // pull out NMEA sentence and META data from line
        auto strNmea = onScanForNmea(strLine);
        
        // check sentence CRC
        if (checkCrc(strNmea) == true)
        {
            // decode sentence
            size_t uWordCount = seperate(m_words, strNmea);
            
            // \todo What AIS talker IDs (words[0]) should we support? For now just allowing all of them..
            bool bValidMsg = (uWordCount >= 7) &&
                             (m_words[5].size() > 1) &&
                             (m_words[5].size() <= MAX_MSG_PAYLOAD_LENGTH);
            
            // try to decode sentence
            if (bValidMsg == true)
            {
                int iFragmentCount = single_digit_strtoi(m_words[1]);
                int iFillBits = single_digit_strtoi(m_words[6]);

                // check for valid sentence
                if ( (iFragmentCount == 0) || (iFragmentCount > MAX_MSG_FRAGMENTS) )
                {
                    m_uDecodingErrors++;
                    onDecodeError(strNmea, "Invalid fragment count value (" + std::to_string(iFragmentCount) + ").");
                }
                    
                // decode simple sentence
                else if (iFragmentCount == 1)
                {
                    try
                    {
                        decodeMobileAisMsg(m_words[5], iFillBits);
                        onMessage(strNmea, getHeader(strLine, strNmea), getFooter(strLine, strNmea));
                    }
                    catch (std::exception &ex)
                    {
                        m_uDecodingErrors++;
                        onDecodeError(strNmea, ex.what());
                    }
                }
                
                // build up multi-sentence payloads
                else if (iFragmentCount > 1)
                {
                    int iMsgId = strtoi(m_words[3]);
                    int iFragmentNum = single_digit_strtoi(m_words[2]);

                    // check for valid message
                    if (iMsgId >= (int)m_multiSentences.size())
                    {
                        m_uDecodingErrors++;
                        onDecodeError(strNmea, "Invalid message sequence id (" + std::to_string(iMsgId) + ").");
                    }
                    
                    // check for valid message
                    else if ( (iFragmentNum == 0) || (iFragmentNum > iFragmentCount) )
                    {
                        m_uDecodingErrors++;
                        onDecodeError(strNmea, "Invalid message fragment number (" + std::to_string(iFragmentNum) + ").");
                    }
                    
                    // create multi-sentence object with first message
                    else if (iFragmentNum == 1)
                    {
                        m_multiSentences[iMsgId] = std::make_unique<MultiSentence>(iFragmentCount, m_words[5], getHeader(strLine, strNmea), getFooter(strLine, strNmea));
                    }
                    
                    // update multi-sentence object with more fragments
                    else
                    {
                        // add to existing payload
                        auto &pMultiSentence = m_multiSentences[iMsgId];
                        if (pMultiSentence != nullptr)
                        {
                            // add new fragment and check for any message payload/fragment errors
                            bool bSuccess = pMultiSentence->addFragment(iFragmentNum, m_words[5]);
                            
                            if (bSuccess == true)
                            {
                                // check if all fragments have been received
                                if (pMultiSentence->isComplete() == true)
                                {
                                    // decode whole payload and reset
                                    try
                                    {
                                        decodeMobileAisMsg(pMultiSentence->payload(), iFillBits);
                                        onMessage(pMultiSentence->payload(), pMultiSentence->header(), pMultiSentence->footer());
                                    }
                                    catch (std::exception &ex)
                                    {
                                        m_uDecodingErrors++;
                                        onDecodeError(pMultiSentence->payload(), ex.what());
                                    }
                                    
                                    m_multiSentences[iMsgId] = nullptr;
                                }
                            }
                            else
                            {
                                // sentence error, so just reset
                                m_uDecodingErrors++;
                                m_multiSentences[iMsgId] = nullptr;
                                onDecodeError(strNmea, "Multi-sentence decoding failed.");
                            }
                        }
                    }
                }
            }
            else
            {
                m_uDecodingErrors++;
                onDecodeError(strNmea, "Sentence not a valid.");
            }
        }
        else
        {
            m_uCrcErrors++;
            onDecodeError(strNmea, "Sentence decoding error. CRC check failed.");
        }
        
        m_uTotalBytes += n;
    }
    
    return n;
}

