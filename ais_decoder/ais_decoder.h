
#ifndef TOOLKIT_AIS_DECODER_H
#define TOOLKIT_AIS_DECODER_H


#include "strutils.h"

#include <string>
#include <vector>
#include <array>


namespace AIS
{
    // constants
    const char ASCII_CHARS[]                = "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ !\"#$%&'()*+,-./0123456789:;<=>?";
    const size_t MAX_FRAGMENTS              = 5;
    const size_t MAX_CHARS_PER_FRAGMENT     = 82;
    
    
    /**
     Buffer used to push back 6 bit nibbles and extract as signed, unsigned, boolean and string values
     */
    class PayloadBuffer
    {
     private:
        const static int MAX_PAYLOAD_SIZE = MAX_FRAGMENTS * MAX_CHARS_PER_FRAGMENT * 6 / 8 + 1;
        
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
     Default implementation to scan through a sentence and extract NMEA string.
     Look at 'onScanForNmea' user defined method on AisDecoder class.
     
     This implementation will scan past META data that start and end with a '\'.  It will also stop at NMEA CRC.
     
     */
    StringRef defaultScanForNmea(const StringRef &_strSentence);
    
    
    /// calc header string from original line and extracted NMEA payload
    StringRef getHeader(const StringRef &_strLine, const StringRef &_strNmea);

    
    /// calc footer string from original line and extracted NMEA payload
    StringRef getFooter(const StringRef &_strLine, const StringRef &_strNmea);

    
    
    /**
     Multi-sentence message container.
     
     */
    class MultiSentence
    {
     public:
        /// add first fragment
        MultiSentence(int _iFragmentCount, const StringRef &_strFirstFragment, const StringRef &_strHeader, const StringRef &_strFooter);
        
        /// add more fragments (returns false if there is an fragment indexing error)
        bool addFragment(int _iFragmentNum, const StringRef &_strFragment);
        
        /// returns true if all fragments have been received
        bool isComplete() const;
        
        /// returns full payload (ref stays valid only while this object exists and addFragment is not called)
        const StringRef &payload() const {return m_strPayload;}
        
        /// returns full payload (ref stays valid only while this object exists and addFragment is not called)
        const StringRef &header() const {return m_strHeader;}
        
        /// returns full payload (ref stays valid only while this object exists and addFragment is not called)
        const StringRef &footer() const {return m_strFooter;}
        
     protected:
        int                     m_iFragmentCount;
        int                     m_iFragmentNum;
        StringRef               m_strHeader;        ///< footer
        StringRef               m_strFooter;
        StringRef               m_strPayload;
        std::vector<char>       m_vecStrData;       ///< data backing for string refs
    };
    
    
    /**
     AIS decoder base class.
     Implemented according to 'http://catb.org/gpsd/AIVDM.html'.
     
     A user of the decoder has to inherit from the decoder class and implement/override 'onTypeXX(...)' style methods as well as error handling methods.
     Some user onTypeXX(...) methods are attached to multiple message types, for example: 123 (types 1, 2 & 3) and 411 (types 4 & 11), in which case the message type is the first parameter.
     
     Callback sequence:
        - onSentence(..) provides raw message fragments as they are received
        - onMessage(...) provides message payload and meta info of the message being decoded
        - onTypeXX(...) provides message specific callbacks
     
     Basic error checking, including CRC checks, are done and also reported.
     No assumtions are made on default or blank values -- all values are returned as integers and the user has to scale and convert the values like position and speed to floats and the desired units.
     
     The 'onScanForNmea(...)' callback allows the decoder to support META data around the NMEA sentence.  The simplest implementation for this callback would just return the
     input parameter if no META data is expected.  The META data footer and header are calculated based on the start and the end of the NMEA string in each NMEA sentence.  For
     multiline messages only the header and footer of the first sentence is reported (reported via 'onMessage(...)').
     
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
        /// called to find NMEA start (scan past any headers, META data, etc.; returns NMEA payload; may be overloaded for app specific meta data; look at 'defaultScanForNmea' function)
        virtual StringRef onScanForNmea(const StringRef &_strSentence) = 0;
        
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
        
        /// called on every sentence (raw data) received (includes all characters, including NL, CR, etc.; called before any validation or CRCs checks are performed)
        virtual void onSentence(const StringRef &_strSentence) = 0;
        
        /// called on every full message (concatenated fragments -- payload), after 'onTypeXX(...)'
        virtual void onMessage(const StringRef &_strPayload, const StringRef &_strHeader, const StringRef &_strFooter) = 0;
        
        /// called when message type is not supported (i.e. 'onTypeXX(...)' not implemented), and 'onMessage(...)' is still called
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
        std::array<StringRef, MAX_MSG_WORDS>                                    m_words;
        
        std::array<uint64_t, MAX_MSG_TYPES>                                     m_msgCounts;            ///< message counts per message type
        uint64_t                                                                m_uTotalMessages;
        uint64_t                                                                m_uTotalBytes;
        uint64_t                                                                m_uCrcErrors;           ///< CRC check error count
        uint64_t                                                                m_uDecodingErrors;      ///< decoding error count (includes CRC errors)
        
        std::array<pfnMsgCallback, 100>                                         m_vecMsgCallbacks;
    };
    
    
};  // namespace AIS


#endif        // #ifndef TOOLKIT_AIS_DECODER_H

