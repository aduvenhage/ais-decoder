
#include "ais_quick.h"
#include "ais_decoder.h"
#include "ais_file.h"
#include "ais_utils.h"

#include <queue>


using namespace std::literals::string_literals;



/// AIS message container
struct AisMessage
{
    std::map<std::string, std::string>  m_fields;
};


/// Decoder implementation that builds message structures with key/value pairs for the fields
class AisQuickDecoder : public AIS::AisDecoder
{
 public:
    AisQuickDecoder()
    {}
    
    static AisQuickDecoder &instance()
    {
        static thread_local AisQuickDecoder decoder;
        return decoder;
    }
    
    /// Pop next message as key value pairs. Returns and empty map if no new messages are available.
    AisMessage popMessage() {
        AisMessage msg;
        
        if (m_messages.empty() == false)
        {
            msg = std::move(m_messages.front());
            m_messages.pop();
        }
        
        return msg;
    }
    
    /// check how many messages are available
    int numMessages() {
        return m_messages.size();
    }

    /// decode all sentences and buffer trailing data internally until next call
    void decodeChunk(const char *_pNmeaBuffer, size_t _uBufferSize) {
        m_buffer.append(_pNmeaBuffer, _uBufferSize);
        AIS::processAisBuffer(*this, m_buffer);
    }
    
 protected:
    virtual void onType123(unsigned int _uMsgType, unsigned int _uMmsi, unsigned int _uNavstatus, int _iRot, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, int _iHeading) override {
        AisMessage msg;
        
        msg.m_fields["msg"] = std::to_string(_uMsgType);
        msg.m_fields["mmsi"] = AIS::mmsi_to_string((long)_uMmsi);
        msg.m_fields["nav_status"] = std::to_string(_uNavstatus);
        msg.m_fields["rot"] = std::to_string(_iRot);
        msg.m_fields["sog"] = std::to_string(_uSog);
        msg.m_fields["pos_accuracy"] = std::to_string(_bPosAccuracy);
        msg.m_fields["pos_lon"] = std::to_string(_iPosLon);
        msg.m_fields["pos_lat"] = std::to_string(_iPosLat);
        msg.m_fields["cog"] = std::to_string(_iCog);
        msg.m_fields["heading"] = std::to_string(_iHeading);

        m_messages.push(std::move(msg));
    }
    
    virtual void onType411(unsigned int _uMsgType, unsigned int _uMmsi, unsigned int _uYear, unsigned int _uMonth, unsigned int _uDay, unsigned int _uHour, unsigned int _uMinute, unsigned int _uSecond,
                           bool _bPosAccuracy, int _iPosLon, int _iPosLat) override {
        AisMessage msg;
        
        msg.m_fields["msg"] = std::to_string(_uMsgType);
        msg.m_fields["mmsi"] = AIS::mmsi_to_string((long)_uMmsi);
        msg.m_fields["year"] = std::to_string(_uYear);
        msg.m_fields["month"] = std::to_string(_uMonth);
        msg.m_fields["day"] = std::to_string(_uDay);
        msg.m_fields["hour"] = std::to_string(_uHour);
        msg.m_fields["minute"] = std::to_string(_uMinute);
        msg.m_fields["second"] = std::to_string(_uSecond);
        msg.m_fields["pos_accuracy"] = std::to_string(_bPosAccuracy);
        msg.m_fields["pos_lon"] = std::to_string(_iPosLon);
        msg.m_fields["pos_lat"] = std::to_string(_iPosLat);

        m_messages.push(std::move(msg));
    }

    virtual void onType5(unsigned int _uMmsi, unsigned int _uImo, const std::string &_strCallsign, const std::string &_strName,
                         unsigned int _uType, unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard, unsigned int _uFixType,
                         unsigned int _uEtaMonth, unsigned int _uEtaDay, unsigned int _uEtaHour, unsigned int _uEtaMinute, unsigned int _uDraught,
                         const std::string &_strDestination) override {
        AisMessage msg;
        
        msg.m_fields["msg"] = std::to_string(5);
        msg.m_fields["mmsi"] = AIS::mmsi_to_string((long)_uMmsi);
        msg.m_fields["imo"] = std::to_string(_uImo);
        msg.m_fields["callsign"] = _strCallsign;
        msg.m_fields["name"] = _strName;
        msg.m_fields["type"] = std::to_string(_uType);
        msg.m_fields["to_bow"] = std::to_string(_uToBow);
        msg.m_fields["to_stern"] = std::to_string(_uToStern);
        msg.m_fields["to_port"] = std::to_string(_uToPort);
        msg.m_fields["to_starboard"] = std::to_string(_uToStarboard);
        msg.m_fields["fix_type"] = std::to_string(_uFixType);
        msg.m_fields["eta_month"] = std::to_string(_uEtaMonth);
        msg.m_fields["eta_day"] = std::to_string(_uEtaDay);
        msg.m_fields["eta_hour"] = std::to_string(_uEtaHour);
        msg.m_fields["eta_minute"] = std::to_string(_uEtaMinute);
        msg.m_fields["draught"] = std::to_string(_uDraught);
        msg.m_fields["destination"] = _strDestination;

        m_messages.push(std::move(msg));
    }
    
    virtual void onType9(unsigned int _uMmsi, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, unsigned int _iAltitude) override {
        AisMessage msg;
        
        msg.m_fields["msg"] = std::to_string(9);
        msg.m_fields["mmsi"] = AIS::mmsi_to_string((long)_uMmsi);
        msg.m_fields["sog"] = std::to_string(_uSog);
        msg.m_fields["pos_accuracy"] = std::to_string(_bPosAccuracy);
        msg.m_fields["pos_lon"] = std::to_string(_iPosLon);
        msg.m_fields["pos_lat"] = std::to_string(_iPosLat);
        msg.m_fields["cog"] = std::to_string(_iCog);
        msg.m_fields["altitude"] = std::to_string(_iAltitude);
        
        m_messages.push(std::move(msg));
    }
    
    virtual void onType18(unsigned int _uMmsi, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, int _iHeading) override {
        AisMessage msg;
        
        msg.m_fields["msg"] = std::to_string(18);
        msg.m_fields["mmsi"] = AIS::mmsi_to_string((long)_uMmsi);
        msg.m_fields["sog"] = std::to_string(_uSog);
        msg.m_fields["pos_accuracy"] = std::to_string(_bPosAccuracy);
        msg.m_fields["pos_lon"] = std::to_string(_iPosLon);
        msg.m_fields["pos_lat"] = std::to_string(_iPosLat);
        msg.m_fields["cog"] = std::to_string(_iCog);
        msg.m_fields["heading"] = std::to_string(_iHeading);
        
        m_messages.push(std::move(msg));
    }
    
    virtual void onType19(unsigned int _uMmsi, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, int _iHeading,
                          const std::string &_strName, unsigned int _uType,
                          unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard) override {
        AisMessage msg;
        
        msg.m_fields["msg"] = std::to_string(19);
        msg.m_fields["mmsi"] = AIS::mmsi_to_string((long)_uMmsi);
        msg.m_fields["sog"] = std::to_string(_uSog);
        msg.m_fields["pos_accuracy"] = std::to_string(_bPosAccuracy);
        msg.m_fields["pos_lon"] = std::to_string(_iPosLon);
        msg.m_fields["pos_lat"] = std::to_string(_iPosLat);
        msg.m_fields["cog"] = std::to_string(_iCog);
        msg.m_fields["heading"] = std::to_string(_iHeading);
        msg.m_fields["name"] = _strName;
        msg.m_fields["type"] = std::to_string(_uType);
        msg.m_fields["to_bow"] = std::to_string(_uToBow);
        msg.m_fields["to_stern"] = std::to_string(_uToStern);
        msg.m_fields["to_port"] = std::to_string(_uToPort);
        msg.m_fields["to_starboard"] = std::to_string(_uToStarboard);
        
        m_messages.push(std::move(msg));
    }
    
    virtual void onType21(unsigned int _uMmsi, unsigned int _uAidType, const std::string &_strName, bool _bPosAccuracy, int _iPosLon, int _iPosLat,
                          unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard) override {
        AisMessage msg;
        
        msg.m_fields["msg"] = std::to_string(21);
        msg.m_fields["mmsi"] = AIS::mmsi_to_string((long)_uMmsi);
        msg.m_fields["aid_type"] = std::to_string(_uAidType);
        msg.m_fields["pos_accuracy"] = std::to_string(_bPosAccuracy);
        msg.m_fields["pos_lon"] = std::to_string(_iPosLon);
        msg.m_fields["pos_lat"] = std::to_string(_iPosLat);
        msg.m_fields["name"] = _strName;
        msg.m_fields["to_bow"] = std::to_string(_uToBow);
        msg.m_fields["to_stern"] = std::to_string(_uToStern);
        msg.m_fields["to_port"] = std::to_string(_uToPort);
        msg.m_fields["to_starboard"] = std::to_string(_uToStarboard);
        
        m_messages.push(std::move(msg));
    }
    
    virtual void onType24A(unsigned int _uMmsi, const std::string &_strName) override {
        AisMessage msg;
        
        msg.m_fields["msg"] = std::to_string(24);
        msg.m_fields["part"] = "A";
        msg.m_fields["mmsi"] = AIS::mmsi_to_string((long)_uMmsi);
        msg.m_fields["name"] = _strName;
        
        m_messages.push(std::move(msg));
    }
    
    virtual void onType24B(unsigned int _uMmsi, const std::string &_strCallsign, unsigned int _uType, unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard) override {
        AisMessage msg;
        
        msg.m_fields["msg"] = std::to_string(24);
        msg.m_fields["part"] = "B";
        msg.m_fields["mmsi"] = AIS::mmsi_to_string((long)_uMmsi);
        msg.m_fields["callsign"] = _strCallsign;
        msg.m_fields["type"] = std::to_string(_uType);
        msg.m_fields["to_bow"] = std::to_string(_uToBow);
        msg.m_fields["to_stern"] = std::to_string(_uToStern);
        msg.m_fields["to_port"] = std::to_string(_uToPort);
        msg.m_fields["to_starboard"] = std::to_string(_uToStarboard);
        
        m_messages.push(std::move(msg));
    }
    
    virtual void onType27(unsigned int _uMmsi, unsigned int _uNavstatus, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog) override {
        AisMessage msg;
        
        msg.m_fields["msg"] = std::to_string(27);
        msg.m_fields["mmsi"] = AIS::mmsi_to_string((long)_uMmsi);
        msg.m_fields["nav_status"] = std::to_string(_uNavstatus);
        msg.m_fields["sog"] = std::to_string(_uSog);
        msg.m_fields["pos_accuracy"] = std::to_string(_bPosAccuracy);
        msg.m_fields["pos_lon"] = std::to_string(_iPosLon);
        msg.m_fields["pos_lat"] = std::to_string(_iPosLat);
        msg.m_fields["cog"] = std::to_string(_iCog);
        
        m_messages.push(std::move(msg));
    }
    
    virtual void onSentence(const AIS::StringRef &_strSentence) override {}
    
    virtual void onMessage(const AIS::StringRef &_strMessage, const AIS::StringRef &_strHeader, const AIS::StringRef &_strFooter) override {
        AisMessage msg;
        
        msg.m_fields["msg"] = std::to_string(0);
        msg.m_fields["payload"] = _strMessage;
        msg.m_fields["header"] = _strHeader;
        msg.m_fields["footer"] = _strFooter;

        m_messages.push(std::move(msg));
    }
    
    virtual void onNotDecoded(const AIS::StringRef &_strMessage, int _iMsgType) override {}
    
    virtual void onDecodeError(const AIS::StringRef &_strMessage, const std::string &_strError) override {
        AisMessage msg;
        
        msg.m_fields["msg"] = std::to_string(0);
        msg.m_fields["payload"] = _strMessage;
        msg.m_fields["error"] = _strError;
        
        m_messages.push(std::move(msg));
    }
    
 private:
    std::queue<AisMessage>      m_messages;     ///< decoded messages -- quick decoder output
    AIS::FileBuffer             m_buffer;       ///< buffer used internally to decode chunks of data
};




/* Push new data onto the decoder. Scans for a complete line and only consumes one line at a time. Returns the number of bytes processed. */
int pushAisSentence(const char *_pNmeaBuffer, size_t _uBufferSize, size_t _uOffset)
{
    return (int)AisQuickDecoder::instance().decodeMsg(_pNmeaBuffer, _uBufferSize, _uOffset);
}


/* Push new data onto the decoder. Scans for all sentences in data and buffers remaining data internally until the next call. Returns the number of bytes processed. */
void pushAisChunk(const char *_pNmeaBuffer, size_t _uBufferSize)
{
    AisQuickDecoder::instance().decodeChunk(_pNmeaBuffer, _uBufferSize);
}


/* Pop next message as key value pairs. Returns and empty map if no new messages are available. */
std::map<std::string, std::string> popAisMessage()
{
    auto msg = AisQuickDecoder::instance().popMessage();
    return std::move(msg.m_fields);
}


/* check how many messages are available */
int numAisMessages()
{
    return AisQuickDecoder::instance().numMessages();
}


/* translates MMSI to country/flag */
std::string mmsi2country(const std::string &_strMmsi)
{
    return AIS::getAisCountryCodes(_strMmsi).first;
}


/* translates MMSI to transmitter class */
std::string mmsi2class(const std::string &_strMmsi)
{
    return AIS::getAisTransmitterClass(_strMmsi);
}
