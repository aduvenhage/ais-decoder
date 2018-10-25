
#include "../../ais_decoder/ais_decoder.h"
#include "../../ais_decoder/ais_file.h"
#include "../../ais_decoder/ais_utils.h"
#include "../utils.h"

#include <string>
#include <vector>
#include <fstream>



/*
 
 
 
 
 
 
 This example demonstrates reading type 1, 2 or 3 AIS messages from a NMEA file and writing decoded values to a CSV file.
 This example also demonstrates how to extract META data from the sentences.
 
 The 'AIS::processAisFile(...)' function is a utility function (see 'ais_decoder/ais_file.h') that reads
 and decodes a file in blocks of the specified size.
 
 
 
 
 
 
 */





/**
    Decodes type 1, 2, 3 and writes info to a file
 
    'onType123(...)' writes the message info to the CSV file.
    'onScanForNmea(...)' extract NMEA sentences from the file lines.
    'onMessage(...)' interprets the META data.
 
 */
class AisCsvDecoder : public AIS::AisDecoder
{
 public:
    AisCsvDecoder(const std::string &_strCsvPath)
        :m_fout(_strCsvPath)
    {}
    
 protected:
    
    /**
     This method extracts the NMEA sentence from the data/line.
     The data before and after the NMEA sentence is taken as the META data header and footer.
     */
    virtual AIS::StringRef onScanForNmea(const AIS::StringRef &_strSentence) override
    {
        // see '../../ais_decoder/ais_decoder.cpp' for example implementation 'defaultScanForNmea(...)'
        return AIS::defaultScanForNmea(_strSentence);
    }
    
    /**
     Decodes AIS message values and writes to CSV file.
     Uses some utility/helper functions from '../../ais_decoder/ais_utils.h'
     
     Callback are called in the following order:
     - onSentence(...): used to reset message number tracking logic
     - onType123(...): outputs message to CSV
     - onMessage(...): outputs META data to CSV if previous message was type 1, 2 or 3
     
     */
    virtual void onType123(unsigned int _uMsgType, unsigned int _uMmsi, unsigned int _uNavstatus, int _iRot, unsigned int _uSog, bool _bPosAccuracy, int _iPosLon, int _iPosLat, int _iCog, int _iHeading) override
    {
        m_iLastMsgType = _uMsgType;
        
        m_fout << _uMsgType << ", "
               << AIS::mmsi_to_string(_uMmsi) << ", "
               << _iPosLat / 600000.0 << ", "
               << _iPosLon / 600000.0 << ", "
               << _uSog / 10.0 << ", "
               << _iCog / 10.0 << ", "
               << AIS::getAisNavigationStatus(_uNavstatus) << ", ";
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
    
    virtual void onSentence(const AIS::StringRef &_strSentence) override {
        // reset message type index
        m_iLastMsgType = 0;
    }
    
    virtual void onMessage(const AIS::StringRef &_strMessage, const AIS::StringRef &_strHeader, const AIS::StringRef &_strFooter) override {
        if (m_iLastMsgType != 0)
        {
            m_fout << "[" << (std::string)_strHeader << "], [" << (std::string)_strFooter << "]\n";
        }
    }
    
    virtual void onNotDecoded(const AIS::StringRef &_strMessage, int _iMsgType) override {}
    
    virtual void onDecodeError(const AIS::StringRef &_strMessage, const std::string &_strError) override {
        std::string msg = _strMessage;
        printf("%s [%s]\n", _strError.c_str(), msg.c_str());
    }
    
 private:
    std::ofstream       m_fout;
    int                 m_iLastMsgType;            /// stores last message type decoded
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
void createCsv(const std::string &_strLogPath)
{
    auto strInputFilePath = std::string(EXAMPLE_DATA_PATH) + "/" + _strLogPath;
    const size_t BLOCK_SIZE = 1024 * 1024 * 8;
    auto tsInit = UTILS::CLOCK::getClockNow();

    // create decoder instance
    AisCsvDecoder decoder(strInputFilePath + ".csv");
    
    // NOTE: EXAMPLE_DATA_PATH is defined by cmake script to be absolute path to source/data folder
    AIS::processAisFile(strInputFilePath, decoder, BLOCK_SIZE, progressCb);
    
    auto td = UTILS::CLOCK::getClockNow() - tsInit;
    double dTd = UTILS::CLOCK::getClockDurationS(td);
    
    printf("rate = %.2f, count = %lu, time = %.2f\n\n\n\n", (float)(decoder.getTotalMessageCount() / dTd), (unsigned long)decoder.getTotalMessageCount(), dTd);
}


/**
 
 */
int main()
{
    createCsv("20170210.log");
    createCsv("datacron_20160105.txt");

    return 0;
}
