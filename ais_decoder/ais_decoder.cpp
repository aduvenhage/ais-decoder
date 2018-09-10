
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

/* pack 6 bits (most significant bit first) */
void PayloadBuffer::set6bits(char _iValue)
{
    setBit((_iValue & 0x20) >> 5);
    setBit((_iValue & 0x10) >> 4);
    setBit((_iValue & 0x08) >> 3);
    setBit((_iValue & 0x04) >> 2);
    setBit((_iValue & 0x02) >> 1);
    setBit(_iValue & 0x01);
}

/* unpack next _iBits (most significant bit is packed first) */
unsigned int PayloadBuffer::getUnsignedValue(int _iBits)
{
    unsigned int uValue = 0;
    
    for (int i = 0; i < _iBits; i++)
    {
        uValue |= getBit() << (_iBits - i - 1);
    }
    
    return uValue;
}

/* unpack next _iBits (most significant bit is packed first; with sign check/conversion) */
int PayloadBuffer::getSignedValue(int _iBits)
{
    unsigned int uValue = 0;
    unsigned int uSignBit = checkBit() << (_iBits - 1);
    unsigned int uSignMask = ~uSignBit;
    
    for (int i = 0; i < _iBits; i++)
    {
        uValue |= getBit() << (_iBits - i - 1);
    }
    
    return (int)(uValue & uSignMask) - uSignBit;
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

/* pack single bit and advance bit index */
void PayloadBuffer::setBit(char _iState)
{
    // set correct bit in correct byte (most significant bit first)
    const unsigned char uBitPosition = 0x07 - (m_iBitIndex & 0x07);
    unsigned char &uByte = m_data[m_iBitIndex >> 3];
    
    if (_iState != 0)
    {
        uByte |= 0x01 << uBitPosition;        // set bit
    }
    else
    {
        uByte &= ~(0x01 << uBitPosition);        // zero out bit
    }
    
    m_iBitIndex++;
}

/* test next bit */
unsigned char PayloadBuffer::checkBit() const
{
    // get correct bit in correct byte (most significant bit first)
    const unsigned char uByte = m_data[m_iBitIndex >> 3];
    const unsigned char uBitPosition = 0x07 - (m_iBitIndex & 0x07);
    return (uByte >> uBitPosition) & 0x01;
}




/* convert payload to decimal (de-armour) and concatenate 6bit decimal values into payload buffer */
void AIS::decodeAscii(PayloadBuffer &_buffer, const StringRef &_strPayload)
{
    for (const char ch : _strPayload)
    {
        // convert to decimal
        char iValue = ch - 48;
        if (iValue > 40)
        {
            iValue -= 8;
        }
        
        // push bits into binary payload buffer
        _buffer.set6bits(iValue);
    }
}



/* calc CRC */
uint8_t AIS::crc(const StringRef &_strPayload)
{
    uint8_t crc = 0;
    for (const char ch : _strPayload)
    {
        crc ^= (uint8_t)ch;
    }
    
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



AisDecoder::AisDecoder()
    :m_multiSentences{},
     m_msgCounts{},
     m_uTotalMessages(0),
     m_uTotalBytes(0),
     m_uCrcErrors(0),
     m_uDecodingErrors(0)
{}

/* decode Position Report (class A) */
void AisDecoder::decodeType1(PayloadBuffer &_buffer)
{
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
    auto heading = _buffer.getSignedValue(9);
    
    onType1(mmsi, navstatus, rot, sog, posAccuracy, posLon, posLat, cog, heading);
}

/* decode Base Station Report (type nibble already pulled from buffer) */
void AisDecoder::decodeType4(PayloadBuffer &_buffer)
{
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
void AisDecoder::decodeType5(PayloadBuffer &_buffer)
{
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
void AisDecoder::decodeType18(PayloadBuffer &_buffer)
{
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
void AisDecoder::decodeType19(PayloadBuffer &_buffer)
{
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
void AisDecoder::decodeType24(PayloadBuffer &_buffer)
{
    // decode message fields (binary buffer has to go through all fields, but some fields are not used)
    _buffer.getUnsignedValue(2);                 // repeatIndicator
    auto mmsi = _buffer.getUnsignedValue(30);
    auto partNo = _buffer.getUnsignedValue(2);
    
    // decode part A
    if (partNo == 0)
    {
        auto name = _buffer.getString(120);
        _buffer.getUnsignedValue(8);            // spare
        
        onType24A(mmsi, name);
    }
    
    // decode part B
    else if (partNo == 1)
    {
        auto type = _buffer.getUnsignedValue(8);
        _buffer.getString(18);                       // vendor ID
        _buffer.getUnsignedValue(4);                 // unit model code
        _buffer.getUnsignedValue(20);                // serial number
        auto callsign = _buffer.getString(42);
        auto toBow = _buffer.getUnsignedValue(9);
        auto toStern = _buffer.getUnsignedValue(9);
        auto toPort = _buffer.getUnsignedValue(6);
        auto toStarboard = _buffer.getUnsignedValue(6);
        _buffer.getUnsignedValue(30);                // Mothership MMSI
        
        onType24B(mmsi, callsign, type, toBow, toStern, toPort, toStarboard);
    }
}

/* decode Mobile AIS station message */
void AisDecoder::decodeMobileAisMsg(const StringRef &_strPayload)
{
    // de-armour string and back bits into buffer
    m_binaryBuffer.resetBitIndex();
    decodeAscii(m_binaryBuffer, _strPayload);
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
        decodeType1(m_binaryBuffer);
    }
    
    // base station report
    else if (msgType == 4)
    {
        decodeType4(m_binaryBuffer);
    }
    
    // voyage report
    else if (msgType == 5)
    {
        decodeType5(m_binaryBuffer);
    }
    
    // position report (class B)
    else if (msgType == 18)
    {
        decodeType18(m_binaryBuffer);
    }
    
    // position report (class B)
    else if (msgType == 19)
    {
        decodeType18(m_binaryBuffer);
    }
    
    // static data report (class B)
    else if (msgType == 24)
    {
        decodeType24(m_binaryBuffer);
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
        int iCrc = (int)std::strtol(_strPayload.data() + n + 1, nullptr, 16);
        int iCrcCalc = (int)AIS::crc(StringRef(_strPayload.data() + 1, n - 1));
        return iCrc == iCrcCalc;
    }
    else
    {
        return false;
    }
}

/* decode next message (starts reading from input buffer with the specified offset; returns the number of bytes processed) */
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
            m_words.clear();
            seperate(m_words, strLine);
            if (m_words.empty() == false)
            {
                // try to decode message
                // \todo what AIS talker IDs (words[0]) should we support?
                bool bValidMsg = m_words.size() >= 7;
                if (bValidMsg == true)
                {
                    int iFragmentCount = strtoi(m_words[1]);
                    
                    // decode simple sentence
                    if (iFragmentCount == 1)
                    {
                        try
                        {
                            onMessage(strLine);
                            decodeMobileAisMsg(m_words[5]);
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
                        int iFragmentNum = strtoi(m_words[2]);
                        
                        if (iMsgId > m_multiSentences.size())
                        {
                            m_uDecodingErrors++;
                            onDecodeError(strLine, std::string("Invalid message sequence id (") + std::to_string(iMsgId) + std::string(")."));
                        }
                        else if (iFragmentNum == 1)
                        {
                            // first sentence
                            m_multiSentences[iMsgId] = std::make_unique<MultiSentence>(iFragmentCount, m_words[5]);
                        }
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
                                            onMessage(strLine);
                                            decodeMobileAisMsg(pMultiSentence->toString());
                                        }
                                        catch (std::exception &ex)
                                        {
                                            m_uDecodingErrors++;
                                            onDecodeError(strLine, ex.what());
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
                    else
                    {
                        m_uDecodingErrors++;
                        onDecodeError(strLine, "Invalid fragment count value (0).");
                    }
                }
            }
        }
        else
        {
            m_uCrcErrors++;
            onDecodeError(strLine, "Message decoding error. CRC check failed.");
        }
        
        m_uTotalBytes += n;
    }
    
    return n;
}

