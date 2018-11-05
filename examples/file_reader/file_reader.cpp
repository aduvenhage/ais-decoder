
#include "../../ais_decoder/ais_decoder.h"
#include "../../ais_decoder/ais_file.h"
#include "../utils.h"

#include <string>
#include <vector>



/*
 
 
 
 
 
 
 This example demonstrates reading AIS messages from a file.
 
 The AIS Decoder is created by inheriting from the AIS::AisDecoder base class with most of the pure virtual
 methods implemented as empty stubs.  This allows us to focus specifically on the raw decoding performance.
 
 The 'AIS::processAisFile(...)' function is a utility function (see 'ais_decoder/ais_file.h') that reads
 and decodes a file in blocks of the specified size.
 
 
 
 
 
 
 */





/// Dummy Sentence Parser (ignores any META data)
class AisDummySentenceParser     : public AIS::SentenceParser
{
 public:
    /// called to find NMEA start (scan past any headers, META data, etc.; returns NMEA payload)
    virtual AIS::StringRef onScanForNmea(const AIS::StringRef &_strSentence) const override {
        return _strSentence;
    }
    
    /// calc header string from original line and extracted NMEA payload
    virtual AIS::StringRef getHeader(const AIS::StringRef &_strLine, const AIS::StringRef &_strNmea) const override {
        return AIS::StringRef();
    }
    
    /// calc footer string from original line and extracted NMEA payload
    virtual AIS::StringRef getFooter(const AIS::StringRef &_strLine, const AIS::StringRef &_strNmea) const override {
        return AIS::StringRef();
    }
    
    /// extracts the timestamp from the meta info
    virtual uint64_t getTimestamp(const AIS::StringRef &_strHeader, const AIS::StringRef &_strFooter) const override {
        return 0;
    }
};




/// Decoder implementation that does nothing -- i.e. just testing NMEA decoding performance
class AisDummyDecoder : public AIS::AisDecoder
{
 public:
    AisDummyDecoder()
    {}
    
 protected:
    virtual void onType123(unsigned int _uMsgType, unsigned int _uMmsi, unsigned int _uNavstatus, int _iRot, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, int _iHeading) override {}
    
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
    
    virtual void onMessage(const AIS::StringRef &_strMessage,
                           const AIS::StringRef &_strHeader, const AIS::StringRef &_strFooter) override {}
    
    virtual void onNotDecoded(const AIS::StringRef &_strMessage, int _iMsgType) override {}
    
    virtual void onDecodeError(const AIS::StringRef &_strMessage, const std::string &_strError) override {
        std::string msg = _strMessage;
        printf("%s [%s]\n", _strError.c_str(), msg.c_str());
    }
};


/// decoder callback that just prints progress/stats to console
void progressCb(size_t _uTotalBytes, const AIS::AisDecoder &_decoder)
{
    printf("bytes = %lu, messages = %lu, errors = %lu\n", (unsigned long)_uTotalBytes, (unsigned long)_decoder.getTotalMessageCount(), (unsigned long)_decoder.getDecodingErrorCount());
}


/**
 Run decoder through one file.
 The decoder user defined methods (for example 'onTypeXX(..)') are called as the messages are decoded.
 The progress callback 'progressCb(...)' is called after a block of data has been processed.
 
 */
void testAis(const std::string &_strLogPath)
{
    const size_t BLOCK_SIZE = 1024 * 1024 * 8;
    auto tsInit = UTILS::CLOCK::getClockNow();
    
    AisDummyDecoder decoder;
    
    // NOTE: EXAMPLE_DATA_PATH is defined by cmake script to be absolute path to source/data folder
    AisDummySentenceParser parser;
    AIS::processAisFile(std::string(EXAMPLE_DATA_PATH) + "/" + _strLogPath, decoder, parser, BLOCK_SIZE, progressCb);
    
    auto td = UTILS::CLOCK::getClockNow() - tsInit;
    double dTd = UTILS::CLOCK::getClockDurationS(td);
    
    printf("rate = %.2f, count = %lu, time = %.2f\n\n\n\n", (float)(decoder.getTotalMessageCount() / dTd), (unsigned long)decoder.getTotalMessageCount(), dTd);
}


/**
 Runs through several files, each with its own decoder, all in on thread.
 
 */
int main()
{
    // to test - just keep on loading files forever
    for (;;)
    {
        /*
        testAis("LSS_20180514.ITU123_data.bu1");
        testAis("nmea-sample_rep.txt");
        */
        testAis("20170210.log");
        testAis("20170211.log");
        testAis("20170212.log");
        testAis("20170213.log");
        testAis("20170214.log");
        testAis("20170215.log");
        testAis("20170216.log");
        testAis("20170217.log");
        /*
        testAis("datacron_20160103.txt");
        testAis("datacron_20160104.txt");
        testAis("datacron_20160105.txt");
         */
    }

    return 0;
}
