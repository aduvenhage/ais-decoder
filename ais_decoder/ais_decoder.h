
#ifndef TOOLKIT_AIS_DECODER_H
#define TOOLKIT_AIS_DECODER_H


#include "strutils.h"

#include <string>
#include <vector>
#include <array>


namespace AIS
{
    // constants
    const char ASCII_CHARS[] = "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ !\"#$%&'()*+,-./0123456789:;<=>?";
    
    
    /**
     Buffer used to push back 6 bit nibbles and extract as signed, unsigned, boolean and string values
     */
    class PayloadBuffer
    {
     private:
        /// NOTE: the max payload size could be up to '(5 fragments * 82 chars * 6bits / 8) = 308' bytes
        const static int MAX_PAYLOAD_SIZE = 512;
        
     public:
        PayloadBuffer();
        
        /// set bit index back to zero
        void resetBitIndex();
        
        /// unpack next _iBits (most significant bit is packed first)
        unsigned int getUnsignedValue(int _iBits);
        
        /// unpack next _iBits (most significant bit is packed first; with sign check/conversion)
        int getSignedValue(int _iBits);
        
        /// unback next boolean (1 bit)
        bool getBoolValue()
        {
            return getUnsignedValue(1) != 0;
        }

        /// unback string (6 bit characters)
        std::string getString(int _iNumBits);
        
        unsigned char* getData(void) {
            return &m_data[0];
        }
        
     private:
        std::array<unsigned char, MAX_PAYLOAD_SIZE>       m_data;
        int32_t                                           m_iBitIndex;
    };
    
    
    /// Convert payload to decimal (de-armour) and concatenate 6bit decimal values into payload buffer. Returns the number of bits used.
    int decodeAscii(PayloadBuffer &_buffer, const StringRef &_strPayload, int _iFillBits);
    
    /// calc CRC
    uint8_t crc(const StringRef &_strPayload);

    
    /**
     Multi-sentence message container.
     
     */
    class MultiSentence
    {
     public:
        MultiSentence(int _iFragmentCount, const StringRef &_strFirstFragment);
        
        bool addFragment(int _iFragmentNum, const StringRef &_strFragment);
        bool isComplete() const;
        
        /// NOTE: the return stays valid only while this object exists and addFragment is not called
        const StringRef &toString() const {return m_strPayload;}
        
     protected:
        int                     m_iFragmentCount;
        int                     m_iFragmentNum;
        StringRef               m_strPayload;
        std::vector<char>       m_vecStrData;       ///< data backing for m_strPayload string ref
    };
    
    
    /**
     AIS decoder base class.
     
     \note implemented according to 'http://catb.org/gpsd/AIVDM.html'.
     
     */
    class AisDecoder
    {
     private:
        const static int MAX_MSG_SEQUENCE_IDS      = 10;       ///< max multi-sentience message sequences
        const static int MAX_MSG_TYPES             = 64;       ///< max message type count (unique messsage IDs)
        const static int MAX_MSG_PAYLOAD_LENGTH    = 82;       ///< max payload length (NMEA limit)
        const static int MAX_MSG_FRAGMENTS         = 5;        ///< maximum number of fragments/sentences a message can have
        const static int MAX_MSG_WORDS             = 10;       ///< maximum number of words per sentence

     public:
        AisDecoder(int _iIndex = 0);
        
        /// returns the user defined index
        int index() const {return m_iIndex;}
        
        /// decode next sentence (starts reading from input buffer with the specified offset; returns the number of bytes processed)
        size_t decodeMsg(const char *_pNmeaBuffer, size_t _uBufferSize, size_t _uOffset);
        
        /// returns the total number of messages processed
        uint64_t getTotalMessageCount() const {return m_uTotalMessages;}
        
        /// returns the number of messages processed per type
        uint64_t getMessageCount(int _iMsgType) {return m_msgCounts[_iMsgType];}
        
        /// returns the total number of bytes processed
        uint64_t getTotalBytes() const {return m_uTotalBytes;}
        
        /// returns the number of CRC check errors
        uint64_t getCrcErrorCount() const {return m_uCrcErrors;}
        
        /// returns the total number of decoding errors
        uint64_t getDecodingErrorCount() const {return m_uDecodingErrors;}
        
     protected:
        virtual void onType123(unsigned int _uMsgType, unsigned int _uMmsi, unsigned int _uNavstatus, int _iRot, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, int _iHeading) = 0;
        virtual void onType4(unsigned int _uMmsi, unsigned int _uYear, unsigned int _uMonth, unsigned int _uDay, unsigned int _uHour, unsigned int _uMinute, unsigned int _uSecond,
                             bool _bPosAccuracy, int _iPosLon, int _iPosLat) = 0;
        virtual void onType5(unsigned int _uMmsi, unsigned int _uImo, const std::string &_strCallsign, const std::string &_strName,
                             unsigned int _uType, unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard, unsigned int _uFixType,
                             unsigned int _uEtaMonth, unsigned int _uEtaDay, unsigned int _uEtaHour, unsigned int _uEtaMinute, unsigned int _uDraught,
                             const std::string &_strDestination) = 0;
        
        virtual void onType18(unsigned int _uMmsi, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, int _iHeading) = 0;
        
        virtual void onType19(unsigned int _uMmsi, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, int _iHeading,
                              const std::string &_strName, unsigned int _uType,
                              unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard) = 0;
        
        virtual void onType24A(unsigned int _uMmsi, const std::string &_strName) = 0;
        
        virtual void onType24B(unsigned int _uMmsi, const std::string &_strCallsign, unsigned int _uType, unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard) = 0;
        
        /// called on every full message, before onTypeXX specific message
        virtual void onMessage(const StringRef &_strPayload) = 0;
        
        /// called when message type is not supported (i.e. onTypeXX not implemented), and onMessage(...) is still called
        virtual void onNotDecoded(const StringRef &_strPayload, int _iMsgType) = 0;
        
        /// called when any decoding error ocurred
        virtual void onDecodeError(const StringRef &_strPayload, const std::string &_strError) = 0;
        
     private:
        /// check sentence CRC
        bool checkCrc(const StringRef &_strPayload);
        
        /// decode Mobile AIS station message
        void decodeMobileAisMsg(const StringRef &_strPayload, int _iFillBits);
        
        /// decode Position Report (class A; type nibble already pulled from buffer)
        void decodeType123(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits);
        
        /// decode Base Station Report (type nibble already pulled from buffer)
        void decodeType4(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits);
        
        /// decode Voyage Report and Static Data (type nibble already pulled from buffer)
        void decodeType5(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits);
        
        /// decode Position Report (class B; type nibble already pulled from buffer)
        void decodeType18(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits);
        
        /// decode Position Report (class B; type nibble already pulled from buffer)
        void decodeType19(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits);
        
        /// decode Voyage Report and Static Data (type nibble already pulled from buffer)
        void decodeType24(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits);
        
    private:
        int                                                                     m_iIndex;               ///< arbitrary id/index set by user for this decoder
        PayloadBuffer                                                           m_binaryBuffer;
        std::array<std::unique_ptr<MultiSentence>, MAX_MSG_SEQUENCE_IDS>        m_multiSentences;
        std::vector<StringRef>                                                  m_words;
        
        std::array<uint64_t, MAX_MSG_TYPES>                                     m_msgCounts;            ///< message counts per message type
        uint64_t                                                                m_uTotalMessages;
        uint64_t                                                                m_uTotalBytes;
        uint64_t                                                                m_uCrcErrors;           ///< CRC check error count
        uint64_t                                                                m_uDecodingErrors;      ///< decoding error count (includes CRC errors)
    };
    
    
};  // namespace AIS


#endif        // #ifndef TOOLKIT_AIS_DECODER_H

