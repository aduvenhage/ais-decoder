
#include "ais_decoder.h"
#include "strutils.h"




using namespace AIS;

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

    // we're done, return data
    return (uint64_t)(bits >> (64 - _iBits));
}

/* unpack next _iBits (most significant bit is packed first; with sign check/conversion) */
int PayloadBuffer::getSignedValue(int _iBits)
{
    const unsigned char *lptr = &m_data[m_iBitIndex >> 3];
    uint64_t bits;

    bits = (uint64_t)lptr[0] << 40;
    bits |= (uint64_t)lptr[1] << 32;
    
    if (_iBits > 9) {
        bits = (uint64_t)lptr[0] << 40;
        bits |= (uint64_t)lptr[1] << 32;
        bits |= (unsigned int)lptr[2] << 24;
        bits |= (unsigned int)lptr[3] << 16;
        bits |= (unsigned int)lptr[4] << 8;
        bits |= (unsigned int)lptr[5];
    }
    bits <<= 16 + (m_iBitIndex & 7);

    m_iBitIndex += _iBits;

    return (int64_t(bits) >> (64 - _iBits));
}

/* unback string (6 bit characters) */
std::string PayloadBuffer::getString(int _iNumBits)
{
    std::string ret(_iNumBits/6, ' ');
    int iNumChars = _iNumBits/6;
    int32_t iStartBitIndex = m_iBitIndex;
    
    for (int i = 0; i < iNumChars; i++)
    {
        int ch = getUnsignedValue(6);
        ret[i] = ASCII_CHARS[ch & 0x3F];
    }
    
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
    
    
    const char* in_ptr = _strPayload.data();
    const char* in_sentinel = in_ptr + _strPayload.size();
    const char* in_sentinel4 = in_ptr + _strPayload.size() - (_strPayload.size() & 3);
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
    
    return _strPayload.size() * 6 - _iFillBits;
}


/* calc CRC */
uint8_t AIS::crc(const StringRef &_strPayload)
{
    unsigned char* in_ptr = (unsigned char*)_strPayload.data();
    const unsigned char* in_sentinel  = in_ptr + _strPayload.size();
    const unsigned char* in_sentinel4 = in_sentinel - (_strPayload.size() & 3);
    
    uint32_t crc4 = 0;
    while (in_ptr < in_sentinel4) {
        crc4 ^= *((uint32_t*)in_ptr);
        in_ptr += 4;
    }
    while (in_ptr < in_sentinel) {
        crc4 ^= *in_ptr++;
    }
    uint8_t crc = (crc4 & 0xff) ^ ((crc4 >> 8) & 0xff) ^ ((crc4 >> 16) & 0xff) ^ ((crc4 >> 24) & 0xff);
    
    return crc;
}




MultiSentence::MultiSentence(int _iFragmentCount, const StringRef &_strFirstFragment)
    :m_iFragmentCount(_iFragmentCount),
     m_iFragmentNum(1)
{
    m_vecStrData.insert(m_vecStrData.end(), _strFirstFragment.data(), _strFirstFragment.data() + _strFirstFragment.size());
    m_strPayload = StringRef(m_vecStrData.data(), m_vecStrData.size());
}

bool MultiSentence::addFragment(int _iFragmentNum, const StringRef &_strFragment)
{
    // check that fragments are added in order (out of order is an error)
    if (m_iFragmentNum == _iFragmentNum-1)
    {
        m_vecStrData.insert(m_vecStrData.end(), _strFragment.data(), _strFragment.data() + _strFragment.size());
        m_strPayload = StringRef(m_vecStrData.data(), m_vecStrData.size());
        m_iFragmentNum++;
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
     m_uDecodingErrors(0)
{}

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
    auto cog = _buffer.getSignedValue(12);
    auto heading = _buffer.getUnsignedValue(9);
    
    onType123(_uMsgType, mmsi, navstatus, rot, sog, posAccuracy, posLon, posLat, cog, heading);
}

/* decode Base Station Report (type nibble already pulled from buffer) */
void AisDecoder::decodeType4(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits)
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
    
    onType4(mmsi, year, month, day, hour, minute, second, posAccuracy, posLon, posLat);
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
    auto toBow = _buffer.getUnsignedValue(9);
    auto toStern = _buffer.getUnsignedValue(9);
    auto toPort = _buffer.getUnsignedValue(6);
    auto toStarboard = _buffer.getUnsignedValue(6);
    auto fixType = _buffer.getUnsignedValue(4);
    auto etaMonth = _buffer.getUnsignedValue(4);    // month (1-12), 0 = N/A
    auto etaDay = _buffer.getUnsignedValue(5);      // day (1-31), 0 = N/A
    auto etaHour = _buffer.getUnsignedValue(5);     // hour (0 - 23), 24 = N/A
    auto etaMinute = _buffer.getUnsignedValue(6);   // minute (0-59), 60 = N/A
    auto draught = _buffer.getUnsignedValue(8);                        // draught
    auto destination = _buffer.getString(120);
    
    onType5(mmsi, imo, callsign, name, type, toBow, toStern, toPort, toStarboard, fixType, etaMonth, etaDay, etaHour, etaMinute, draught, destination);
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
    auto cog = _buffer.getSignedValue(12);
    auto heading = _buffer.getSignedValue(9);
    
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
    auto cog = _buffer.getSignedValue(12);
    auto heading = _buffer.getSignedValue(9);
    _buffer.getUnsignedValue(6);                 // timestamp
    _buffer.getUnsignedValue(4);                 // reserved
    auto name = _buffer.getString(120);
    auto type = _buffer.getUnsignedValue(8);
    auto toBow = _buffer.getUnsignedValue(9);
    auto toStern = _buffer.getUnsignedValue(9);
    auto toPort = _buffer.getUnsignedValue(6);
    auto toStarboard = _buffer.getUnsignedValue(6);
    _buffer.getUnsignedValue(4);                 // fix type
    
    onType19(mmsi, sog, posAccuracy, posLon, posLat, cog, heading, name, type, toBow, toStern, toPort, toStarboard);
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
        _buffer.getString(18);                       // vendor ID
        _buffer.getUnsignedValue(4);                 // unit model code
        _buffer.getUnsignedValue(20);                // serial number
        auto callsign = _buffer.getString(42);
        auto toBow = _buffer.getUnsignedValue(9);
        auto toStern = _buffer.getUnsignedValue(9);
        auto toPort = _buffer.getUnsignedValue(6);
        auto toStarboard = _buffer.getUnsignedValue(6);
        // FvdB: Note that this field overlaps the previous 4 fields, total message length is 162 bits
        // _buffer.getUnsignedValue(30);                // Mothership MMSI
        
        onType24B(mmsi, callsign, type, toBow, toStern, toPort, toStarboard);
    }
}

/* decode Mobile AIS station message */
void AisDecoder::decodeMobileAisMsg(const StringRef &_strPayload, int _iFillBits)
{
    // de-armour string and back bits into buffer
    m_binaryBuffer.resetBitIndex();
    int iBitsUsed = decodeAscii(m_binaryBuffer, _strPayload, _iFillBits);
    m_binaryBuffer.resetBitIndex();
    
    // decode message
    auto msgType = m_binaryBuffer.getUnsignedValue(6);
    m_msgCounts[msgType]++;
    m_uTotalMessages++;
    
    // position report (class A)
    if ( (msgType == 1) ||
         (msgType == 2) ||
         (msgType == 3) )
    {
        decodeType123(m_binaryBuffer, msgType, iBitsUsed);
    }
    
    // base station report
    else if (msgType == 4)
    {
        decodeType4(m_binaryBuffer, msgType, iBitsUsed);
    }
    
    // voyage report
    else if (msgType == 5)
    {
        decodeType5(m_binaryBuffer, msgType, iBitsUsed);
    }
    
    // position report (class B)
    else if (msgType == 18)
    {
        decodeType18(m_binaryBuffer, msgType, iBitsUsed);
    }
    
    // position report (class B)
    else if (msgType == 19)
    {
        decodeType19(m_binaryBuffer, msgType, iBitsUsed);
    }
    
    // static data report (class B)
    else if (msgType == 24)
    {
        decodeType24(m_binaryBuffer, msgType, iBitsUsed);
    }
    
    // message not decoded
    else
    {
        onNotDecoded(_strPayload, msgType);
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
        
        int iCrc = ascii_t[(_strPayload.data()[n+1] - '0') & 31]*16 + ascii_t[(_strPayload.data()[n+2] - '0') & 31];
        
        if (*_strPayload.data() == '!')
        {
            unsigned iCrcCalc = (int)AIS::crc(StringRef(_strPayload.data() + 1, n - 1));
            return iCrc == iCrcCalc;
        }
        else
        {
            unsigned iCrcCalc = (int)AIS::crc(StringRef(_strPayload.data(), n));
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
        // check sentence CRC
        if (checkCrc(strLine) == true)
        {
            // decode sentence
            size_t uWordCount = seperate(m_words, strLine);
            
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
                    onDecodeError(strLine, "Invalid fragment count value (" + std::to_string(iFragmentCount) + ").");
                }
                    
                // decode simple sentence
                else if (iFragmentCount == 1)
                {
                    try
                    {
                        onMessage(strLine);
                        decodeMobileAisMsg(m_words[5], iFillBits);
                    }
                    catch (std::exception &ex)
                    {
                        m_uDecodingErrors++;
                        onDecodeError(strLine, ex.what());
                    }
                }
                
                // build up multi sentence payloads
                else if (iFragmentCount > 1)
                {
                    int iMsgId = strtoi(m_words[3]);
                    int iFragmentNum = single_digit_strtoi(m_words[2]);

                    // check for valid message
                    if (iMsgId >= (int)m_multiSentences.size())
                    {
                        m_uDecodingErrors++;
                        onDecodeError(strLine, "Invalid message sequence id (" + std::to_string(iMsgId) + ").");
                    }
                    
                    // check for valid message
                    else if ( (iFragmentNum == 0) || (iFragmentNum > iFragmentCount) )
                    {
                        m_uDecodingErrors++;
                        onDecodeError(strLine, "Invalid message fragment number (" + std::to_string(iFragmentNum) + ").");
                    }
                    
                    // create multi-sentence object with first message
                    else if (iFragmentNum == 1)
                    {
                        m_multiSentences[iMsgId] = std::make_unique<MultiSentence>(iFragmentCount, m_words[5]);
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
                                        onMessage(pMultiSentence->toString());
                                        decodeMobileAisMsg(pMultiSentence->toString(), iFillBits);
                                    }
                                    catch (std::exception &ex)
                                    {
                                        m_uDecodingErrors++;
                                        onDecodeError(pMultiSentence->toString(), ex.what());
                                    }
                                    
                                    m_multiSentences[iMsgId] = nullptr;
                                }
                            }
                            else
                            {
                                // sentence error, so just reset
                                m_uDecodingErrors++;
                                m_multiSentences[iMsgId] = nullptr;
                                onDecodeError(strLine, "Multi-sentence decoding failed.");
                            }
                        }
                    }
                }
            }
            else
            {
                m_uDecodingErrors++;
                onDecodeError(strLine, "Sentence not a valid.");
            }
        }
        else
        {
            m_uCrcErrors++;
            onDecodeError(strLine, "Sentence decoding error. CRC check failed.");
        }
        
        m_uTotalBytes += n;
    }
    
    return n;
}

