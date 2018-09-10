
#include "../../ais_decoder/ais_file.h"
#include "../utils.h"

#include <string>
#include <vector>
#include <chrono>





/// Decoder implementation that does nothing -- i.e. just testing NMEA decoding performance
class AisTestDecoder : public AIS::AisDecoder
{
 public:
    AisTestDecoder()
    {}
    
 protected:
    virtual void onType1(unsigned int _uMmsi, unsigned int _uNavstatus, int _iRot, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, int _iHeading) override {}
    
    virtual void onType4(unsigned int _uMmsi, unsigned int _uYear, unsigned int _uMonth, unsigned int _uDay, unsigned int _uHour, unsigned int _uMinute, unsigned int _uSecond,
                         bool _bPosAccuracy, int _iPosLon, int _iPosLat) override {}
    
    virtual void onType5(unsigned int _uMmsi, unsigned int _uImo, const std::string &_strCallsign, const std::string &_strName,
                         unsigned int _uType, unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard, unsigned int _uFixType,
                         unsigned int _uEtaMonth, unsigned int _uEtaDay, unsigned int _uEtaHour, unsigned int _uEtaMinute, unsigned int _uDraught,
                         const std::string &_strDestination) override {}
    
    virtual void onType18(unsigned int _uMmsi, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, int _iHeading) override {}
    
    virtual void onType19(unsigned int _uMmsi, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, int _iHeading,
                          const std::string &_strName, unsigned int _uType,
                          unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard) override {}
    
    virtual void onType24A(unsigned int _uMmsi, const std::string &_strName) override {}
    
    virtual void onType24B(unsigned int _uMmsi, const std::string &_strCallsign, unsigned int _uType, unsigned int _uToBow, unsigned int _uToStern, unsigned int _uToPort, unsigned int _uToStarboard) override {}
    
    virtual void onMessage(const AIS::StringRef &_strMessage) override {}
    
    virtual void onNotDecoded(const AIS::StringRef &_strMessage, int _iMsgType) override {}
    
    virtual void onDecodeError(const AIS::StringRef &_strMessage, const std::string &_strError) override {printf("%s\n", _strError.c_str());}
};


/// decoder callback that just prints progress/stats to console
void progressCb(size_t _uTotalBytes, const AIS::AisDecoder &_decoder)
{
    printf("bytes = %lu, messages=%lu, errors=%lu\n", (unsigned long)_uTotalBytes, (unsigned long)_decoder.getTotalMessageCount(), (unsigned long)_decoder.getDecodingErrorCount());
}


/**
 Run decoder through one file.
 The decoder user defined methods (for example 'onTypeXX(..)') are called as the messages are decoded.
 The progress callback 'progressCb(...)' is called after a block of data has been processed.
 
 */
void test_ais(const std::string &_strLogPath)
{
    const size_t BLOCK_SIZE = 1024 * 1024 * 8;
    auto tsInit = UTILS::CLOCK::getClockNow();
    
    AisTestDecoder decoder;
    AIS::processAisFile(_strLogPath, decoder, BLOCK_SIZE, progressCb);
    
    auto td = UTILS::CLOCK::getClockNow() - tsInit;
    double dTd = UTILS::CLOCK::getClockDurationS(td);
    
    printf("rate = %.2f, count = %lu, time = %.2f\n\n\n\n", (float)(decoder.getTotalMessageCount() / dTd), (unsigned long)decoder.getTotalMessageCount(), dTd);
}


/**
 Runs through several files, each with its own decoder, all in on thread.
 
 */
int main()
{
    // NOTE: EXAMPLE_DATA_PATH is defined by cxmake script to be absolute path to source/data folder
    for (;;)
    {
        test_ais(std::string(EXAMPLE_DATA_PATH) + "/ais_20170201.log");
        test_ais(std::string(EXAMPLE_DATA_PATH) + "/ais_20170203.log");
        test_ais(std::string(EXAMPLE_DATA_PATH) + "/ais_20170204.log");
        test_ais(std::string(EXAMPLE_DATA_PATH) + "/ais_20170205.log");
        test_ais(std::string(EXAMPLE_DATA_PATH) + "/ais_20170206.log");
        test_ais(std::string(EXAMPLE_DATA_PATH) + "/ais_20170207.log");
        test_ais(std::string(EXAMPLE_DATA_PATH) + "/ais_20170208.log");
        test_ais(std::string(EXAMPLE_DATA_PATH) + "/ais_20170209.log");
        test_ais(std::string(EXAMPLE_DATA_PATH) + "/ais_20170210.log");
    }

    return 0;
}
