
#include "ais_quick.h"
#include "ais_decoder.h"

#include <queue>



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
    
    AisMessage popMessage() {
        AisMessage msg;
        
        if (m_messages.empty() == false)
        {
            msg = std::move(m_messages.front());
            m_messages.pop();
        }
        
        return msg;
    }
    
 protected:
    virtual void onType123(unsigned int _uMsgType, unsigned int _uMmsi, unsigned int _uNavstatus, int _iRot, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, int _iHeading) override {
        AisMessage msg;
        
        msg.m_fields["type"] = std::to_string(_uMsgType);
        msg.m_fields["mmsi"] = std::to_string(_uMmsi);
        msg.m_fields["nav_status"] = std::to_string(_uNavstatus);

        m_messages.push(std::move(msg));
    }
    
    virtual void onType411(unsigned int _uMsgType, unsigned int _uMmsi, unsigned int _uYear, unsigned int _uMonth, unsigned int _uDay, unsigned int _uHour, unsigned int _uMinute, unsigned int _uSecond,
                           bool _bPosAccuracy, int _iPosLon, int _iPosLat) override {}
    
    virtual void onType5(unsigned int _uMmsi, unsigned int _uImo, const std::string &_strCallsign, const std::string &_strName,
                         unsigned int _uType, unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard, unsigned int _uFixType,
                         unsigned int _uEtaMonth, unsigned int _uEtaDay, unsigned int _uEtaHour, unsigned int _uEtaMinute, unsigned int _uDraught,
                         const std::string &_strDestination) override {}
    
    virtual void onType9(unsigned int _uMmsi, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, unsigned int _iAltitude) override {}
    
    virtual void onType18(unsigned int _uMmsi, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, int _iHeading) override {}
    
    virtual void onType19(unsigned int _uMmsi, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, int _iHeading,
                          const std::string &_strName, unsigned int _uType,
                          unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard) override {}
    
    virtual void onType21(unsigned int _uMmsi, unsigned int _uAidType, const std::string &_strName, bool _bPosAccuracy, int _iPosLon, int _iPosLat,
                          unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard) override {}
    
    virtual void onType24A(unsigned int _uMmsi, const std::string &_strName) override {}
    
    virtual void onType24B(unsigned int _uMmsi, const std::string &_strCallsign, unsigned int _uType, unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard) override {}
    
    virtual void onType27(unsigned int _uMmsi, unsigned int _uNavstatus, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog) override {}
    
    virtual void onSentence(const AIS::StringRef &_strSentence) override {}
    
    virtual void onMessage(const AIS::StringRef &_strMessage) override {}
    
    virtual void onNotDecoded(const AIS::StringRef &_strMessage, int _iMsgType) override {}
    
    virtual void onDecodeError(const AIS::StringRef &_strMessage, const std::string &_strError) override {}
    
 private:
    std::queue<AisMessage>      m_messages;
};




AisQuickDecoder  g_decoder;



int pushSentence(const char *_pszSentence)
{
    return (int)g_decoder.decodeMsg(_pszSentence, std::strlen(_pszSentence), 0);
}


std::map<std::string, std::string> popMessage()
{
    auto msg = g_decoder.popMessage();
    return std::move(msg.m_fields);
}
