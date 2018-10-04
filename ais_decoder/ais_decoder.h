
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

        /// unback string (6 bit characters) -- already cleans string (removes trailing '@' and trailing spaces)
        std::string getString(int _iNumBits);
        
        unsigned char* getData(void) {
            return &m_data[0];
        }
        
     private:
        alignas(16) std::array<unsigned char, MAX_PAYLOAD_SIZE>       m_data;
        int32_t                                                       m_iBitIndex;
    };
    
    
    /// Convert payload to decimal (de-armour) and concatenate 6bit decimal values into payload buffer. Returns the number of bits used.
    int decodeAscii(PayloadBuffer &_buffer, const StringRef &_strPayload, int _iFillBits);
    
    /// calc CRC
    uint8_t crc(const StringRef &_strLine);

    
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
     Implemented according to 'http://catb.org/gpsd/AIVDM.html'.
     
     A user of the decoder has to inherit from the decoder class and implement/override 'onTypeXX(...)' style methods as well as error handling methods.
     Some user onTypeXX(...) methods are attached to multiple message types, for example: 123 (types 1, 2 & 3) and 411 (types 4 & 11), in which case the message type is the first parameter.
     
     Basic error checking, including CRC checks, are done and also reported.
     No assumtions are made on default or blank values -- all values are returned as integers and the user has to scale and convert the values like position and speed to floats and the desired units.
     
     */
    class AisDecoder
    {
     private:
        const static int MAX_MSG_SEQUENCE_IDS      = 10;       ///< max multi-sentience message sequences
        const static int MAX_MSG_TYPES             = 64;       ///< max message type count (unique messsage IDs)
        const static int MAX_MSG_PAYLOAD_LENGTH    = 82;       ///< max payload length (NMEA limit)
        const static int MAX_MSG_FRAGMENTS         = 5;        ///< maximum number of fragments/sentences a message can have
        const static int MAX_MSG_WORDS             = 10;       ///< maximum number of words per sentence
        
        using pfnMsgCallback = void (AisDecoder::*)(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits);

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
        
        // user defined callbacks
     protected:
        virtual void onType123(unsigned int _uMsgType, unsigned int _uMmsi, unsigned int _uNavstatus, int _iRot, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, int _iHeading) = 0;
        virtual void onType411(unsigned int _uMsgType, unsigned int _uMmsi, unsigned int _uYear, unsigned int _uMonth, unsigned int _uDay, unsigned int _uHour, unsigned int _uMinute, unsigned int _uSecond,
                               bool _bPosAccuracy, int _iPosLon, int _iPosLat) = 0;
        virtual void onType5(unsigned int _uMmsi, unsigned int _uImo, const std::string &_strCallsign, const std::string &_strName,
                             unsigned int _uType, unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard, unsigned int _uFixType,
                             unsigned int _uEtaMonth, unsigned int _uEtaDay, unsigned int _uEtaHour, unsigned int _uEtaMinute, unsigned int _uDraught,
                             const std::string &_strDestination) = 0;
        
        virtual void onType9(unsigned int _uMmsi, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, unsigned int _iAltitude) = 0;
        
        virtual void onType18(unsigned int _uMmsi, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, int _iHeading) = 0;
        
        virtual void onType19(unsigned int _uMmsi, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, int _iHeading,
                              const std::string &_strName, unsigned int _uType,
                              unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard) = 0;
        
        virtual void onType21(unsigned int _uMmsi, unsigned int _uAidType, const std::string &_strName, bool _bPosAccuracy, int _iPosLon, int _iPosLat,
                              unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard) = 0;
        
        virtual void onType24A(unsigned int _uMmsi, const std::string &_strName) = 0;
        
        virtual void onType24B(unsigned int _uMmsi, const std::string &_strCallsign, unsigned int _uType, unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard) = 0;
        
        virtual void onType27(unsigned int _uMmsi, unsigned int _uNavstatus, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog) = 0;
        
        /// called on every sentence received (includes all characters, including NL, CR, etc.; called before any validation or CRCs checks are performed)
        virtual void onSentence(const StringRef &_strSentence) = 0;
        
        /// called on every full message (i.e. all fragments), before onTypeXX specific message
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
        
        /// decode Base Station Report (type nibble already pulled from buffer; or, response to inquiry)
        void decodeType411(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits);
        
        /// decode Voyage Report and Static Data (type nibble already pulled from buffer)
        void decodeType5(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits);
        
        /// decode Standard SAR Aircraft Position Report
        void decodeType9(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits);
        
        /// decode Standard SAR Aircraft Position Report
        void decodeType11(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits);
        
        /// decode Position Report (class B; type nibble already pulled from buffer)
        void decodeType18(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits);
        
        /// decode Position Report (class B; type nibble already pulled from buffer)
        void decodeType19(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits);
        
        /// decode Aid-to-Navigation Report
        void decodeType21(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits);
        
        /// decode Voyage Report and Static Data (type nibble already pulled from buffer)
        void decodeType24(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits);
        
        /// decode Long Range AIS Broadcast message (type nibble already pulled from buffer)
        void decodeType27(PayloadBuffer &_buffer, unsigned int _uMsgType, int _iPayloadSizeBits);

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
        
        std::array<pfnMsgCallback, 100>                                         m_vecMsgCallbacks;
    };
    
    
};  // namespace AIS


#endif        // #ifndef TOOLKIT_AIS_DECODER_H

