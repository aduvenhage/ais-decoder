
#include "../../ais_decoder/ais_file.h"
#include "../utils.h"

#include <string>
#include <vector>
#include <future>





/// Decoder implementation that does nothing -- i.e. just testing NMEA decoding performance
class AisDummyDecoder : public AIS::AisDecoder
{
 public:
    AisDummyDecoder(int _iIndex)
        :AisDecoder(_iIndex)
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
    
    virtual void onMessage(const AIS::StringRef &_strMessage, const AIS::StringRef &_strHeader, const AIS::StringRef &_strFooter) override {}
    
    virtual void onNotDecoded(const AIS::StringRef &_strMessage, int _iMsgType) override {}

    virtual void onDecodeError(const AIS::StringRef &_strMessage, const std::string &_strError) override {printf("%s\n", _strError.c_str());}
};


/// decoder callback that just prints progress/stats to console
void progressCb(size_t _uTotalBytes, const AIS::AisDecoder &_decoder)
{
    //printf("%d: bytes = %lu, messages = %lu, errors = %lu\n", _decoder.index(), (unsigned long)_uTotalBytes, (unsigned long)_decoder.getTotalMessageCount(), (unsigned long)_decoder.getDecodingErrorCount());
}


/**
 Run decoder through one file.
 The decoder user defined methods (for example 'onTypeXX(..)') are called as the messages are decoded.
 The progress callback 'progressCb(...)' is called after a block of data has been processed.
 
 */
void testAis(const std::string &_strLogPath, int _iIndex)
{
    const size_t BLOCK_SIZE = 1024 * 512;
    auto tsInit = UTILS::CLOCK::getClockNow();
    
    AisDummyDecoder decoder(_iIndex);
    
    // NOTE: EXAMPLE_DATA_PATH is defined by cmake script to be absolute path to source/data folder
    AIS::processAisFile(std::string(EXAMPLE_DATA_PATH) + "/" + _strLogPath, decoder, BLOCK_SIZE, progressCb);
    
    auto td = UTILS::CLOCK::getClockNow() - tsInit;
    double dTd = UTILS::CLOCK::getClockDurationS(td);
    
    printf("%d: rate = %.2f, count = %lu, time = %.2f\n", decoder.index(), (float)(decoder.getTotalMessageCount() / dTd), (unsigned long)decoder.getTotalMessageCount(), dTd);
}


/**
 Start multiple threads and then wait for all of them to complete
 */
void runAndWait()
{
    std::vector<std::future<void>> vecThreads;
    
    // NOTE: std::async is used to run test in its own thread
    // NOTE: puts the std::future values (result of async) in a list to wait on later
    //vecThreads.push_back(std::async(std::launch::async, testAis, "nmea_data_sample.txt", 0));
    //vecThreads.push_back(std::async(std::launch::async, testAis, "nmea-sample.txt", 1));
    vecThreads.push_back(std::async(std::launch::async, testAis, "20170210.log", 0));
    vecThreads.push_back(std::async(std::launch::async, testAis, "20170211.log", 1));
    vecThreads.push_back(std::async(std::launch::async, testAis, "20170212.log", 2));
    vecThreads.push_back(std::async(std::launch::async, testAis, "20170213.log", 3));
    vecThreads.push_back(std::async(std::launch::async, testAis, "20170214.log", 4));
    vecThreads.push_back(std::async(std::launch::async, testAis, "20170215.log", 5));
    vecThreads.push_back(std::async(std::launch::async, testAis, "20170216.log", 6));
    vecThreads.push_back(std::async(std::launch::async, testAis, "20170217.log", 7));
    
    // wait on all threads to stop
    while (vecThreads.empty() == false)
    {
        auto &result = vecThreads.back();
        if (result.wait_for(std::chrono::milliseconds(10)) == std::future_status::ready)
        {
            vecThreads.pop_back();
        }
    }
}

/**
 Runs through several files, each with its own decoder, all in on thread.
 
 */
int main()
{
    
    // to test - just keep on loading files forever
    for (;;)
    {
        printf("New generation started.\n");
        
        runAndWait();
    }
    
    return 0;
}
