
#include "../../ais_decoder/ais_decoder.h"
#include "../../ais_decoder/ais_file.h"
#include "../../ais_decoder/default_sentence_parser.h"
#include "../example_utils.h"

#include <string>
#include <vector>



/*
 
 
 
 
 
 
 This example demonstrates reading AIS messages from a file.
 
 The AIS Decoder is created by inheriting from the AIS::AisDecoder base class with most of the pure virtual
 methods implemented as empty stubs.  This allows us to focus specifically on the raw decoding performance.
 
 The 'AIS::processAisFile(...)' function is a utility function (see 'ais_decoder/ais_file.h') that reads
 and decodes a file in blocks of the specified size.
 
 
 
 
 
 
 */




/// Decoder implementation that does nothing -- i.e. just testing NMEA decoding performance
class AisDummyDecoder : public AIS::AisDecoder
{
 public:
    AisDummyDecoder()
    {}
    
 protected:
    virtual void onType123(unsigned int , unsigned int , unsigned int , int , unsigned int , bool , int , int , int , int ) override {}
    
    virtual void onType411(unsigned int , unsigned int , unsigned int , unsigned int , unsigned int , unsigned int , unsigned int , unsigned int ,
                           bool , int , int ) override {}
    
    virtual void onType5(unsigned int , unsigned int , const std::string &, const std::string &,
                         unsigned int , unsigned int , unsigned int , unsigned int , unsigned int , unsigned int ,
                         unsigned int , unsigned int , unsigned int , unsigned int , unsigned int ,
                         const std::string &) override {}
    
    virtual void onType9(unsigned int , unsigned int , bool , int , int , int , unsigned int ) override {}
    
    virtual void onType18(unsigned int , unsigned int , bool , int , int , int , int ) override {}
    
    virtual void onType19(unsigned int , unsigned int , bool , int , int , int , int ,
                          const std::string &, unsigned int ,
                          unsigned int , unsigned int , unsigned int , unsigned int ) override {}
    
    virtual void onType21(unsigned int , unsigned int , const std::string &, bool , int , int ,
                          unsigned int , unsigned int , unsigned int , unsigned int ) override {}
    
    virtual void onType24A(unsigned int , const std::string &) override {}
    
    virtual void onType24B(unsigned int , const std::string &, unsigned int , unsigned int , unsigned int , unsigned int , unsigned int ) override {}
    
    virtual void onType27(unsigned int , unsigned int , unsigned int , bool , int , int , int ) override {}
    
    virtual void onSentence(const AIS::StringRef &) override {}
    
    virtual void onMessage(const AIS::StringRef &,
                           const AIS::StringRef &, const AIS::StringRef &) override {}
    
    virtual void onNotDecoded(const AIS::StringRef &, int ) override {}
    
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
    AIS::DefaultSentenceParser parser;
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
        //testAis("nmea-sample.txt");
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

    //return 0;
}
