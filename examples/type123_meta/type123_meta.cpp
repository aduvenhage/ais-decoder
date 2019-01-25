
#include "../../ais_decoder/ais_decoder.h"
#include "../../ais_decoder/ais_file.h"
#include "../../ais_decoder/ais_utils.h"
#include "../../ais_decoder/strutils.h"
#include "../../ais_decoder/default_sentence_parser.h"
#include "../example_utils.h"

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
 
     Callbacks are called in the following order:
     - onScanForNmea(...): extract NMEA sentences from the file lines
     - onSentence(...): used to reset message number tracking logic
     - onMessage(...): temporarily stores META info for all messages
     - onType123(...): outputs message and META to CSV
 */
class AisCsvDecoder : public AIS::AisDecoder
{
 public:
    AisCsvDecoder(const std::string &_strCsvPath, const AIS::SentenceParser &_parser)
        :m_fout(_strCsvPath),
         m_parser(_parser)
    {
        m_fout << "type" << ", "
               << "mmsi" << ", "
               << "timestamp" << ", "
               << "Lat" << ", "
               << "Lon" << ", "
               << "Sog" << ", "
               << "Cog" << ", "
               << "NavStatus" << ", ["
               << "Header" << "], ["
               << "Footer" << "]\n";
    }
    
 protected:
    /**
     Decodes AIS message values and writes to CSV file.
     Uses some utility/helper functions from '../../ais_decoder/ais_utils.h'
     
     */
    virtual void onType123(unsigned int _uMsgType, unsigned int _uMmsi, unsigned int _uNavstatus, int /*_iRot*/, unsigned int _uSog, bool /*_bPosAccuracy*/, int _iPosLon, int _iPosLat, int _iCog, int /*_iHeading*/) override
    {
        // decode META info to get timestamp
        uint64_t uTimestamp = m_parser.getTimestamp(header(), footer());
        
        // output to CSV
        m_fout << _uMsgType << ", "
               << AIS::mmsi_to_string(_uMmsi) << ", "
               << uTimestamp << ", "
               << _iPosLat / 600000.0 << ", "
               << _iPosLon / 600000.0 << ", "
               << _uSog / 10.0 << ", "
               << _iCog / 10.0 << ", "
               << AIS::getAisNavigationStatus(_uNavstatus) << ", ["
               << (std::string)header() << "], ["
               << (std::string)footer() << "]\n";
    }
    
	virtual void onType411(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int,
                           bool, int, int) override {}

	virtual void onType5(unsigned int, unsigned int, const std::string &, const std::string &,
                         unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int,
                         unsigned int, unsigned int, unsigned int, unsigned int, unsigned int,
                         const std::string &) override {}

	virtual void onType9(unsigned int, unsigned int, bool, int, int, int, unsigned int) override {}

	virtual void onType18(unsigned int, unsigned int, bool, int, int, int, int) override {}

	virtual void onType19(unsigned int, unsigned int, bool, int, int, int, int, const std::string &, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int) override {}

	virtual void onType21(unsigned int, unsigned int, const std::string &, bool, int, int, unsigned int, unsigned int, unsigned int, unsigned int) override {}

	virtual void onType24A(unsigned int, const std::string &) override {}

	virtual void onType24B(unsigned int, const std::string &, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int) override {}

	virtual void onType27(unsigned int, unsigned int, unsigned int, bool, int, int, int) override {}

	virtual void onSentence(const AIS::StringRef &) override {}

	virtual void onMessage(const AIS::StringRef &, const AIS::StringRef &, const AIS::StringRef &) override {}

	virtual void onNotDecoded(const AIS::StringRef &, int) override {}

    virtual void onDecodeError(const AIS::StringRef &_strMessage, const std::string &_strError) override {
        std::string msg(_strMessage.data(), _strMessage.size());
        printf("%s [%s]\n", _strError.c_str(), msg.c_str());
    }
    
 private:
    std::ofstream               m_fout;             ///< CVS output file
    const AIS::SentenceParser   &m_parser;          ///< sentence parser being used
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
    // NOTE: EXAMPLE_DATA_PATH is defined by cmake script to be absolute path to source/data folder
    auto strInputFilePath = std::string(EXAMPLE_DATA_PATH) + "/" + _strLogPath;
    const size_t BLOCK_SIZE = 1024 * 1024 * 8;
    auto tsInit = UTILS::CLOCK::getClockNow();

    // create decoder instance
    AIS::DefaultSentenceParser parser;
    AisCsvDecoder decoder(strInputFilePath + ".csv", parser);
    
    AIS::processAisFile(strInputFilePath, decoder, parser, BLOCK_SIZE, progressCb);
    
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
