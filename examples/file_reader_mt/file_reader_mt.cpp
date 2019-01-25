
#include "../../ais_decoder/ais_decoder.h"
#include "../../ais_decoder/ais_file.h"
#include "../../ais_decoder/default_sentence_parser.h"
#include "../example_utils.h"

#include <string>
#include <vector>
#include <future>




/*
 
 
 
 
 
 
 This example demonstrates reading AIS messages from a file using multiple threads (one thread per file).
 The approach uses std::async to fire off asyncronous decoding of files (very simple, but effective).
 
 The AIS Decoder is created by inheriting from the AIS::AisDecoder base class with most of the pure virtual
 methods implemented as empty stubs.  This allows us to focus specifically on the raw decoding performance.
 
 The 'AIS::processAisFile(...)' function is a utility function (see 'ais_decoder/ais_file.h') that reads
 and decodes a file in blocks of the specified size.

 
 
 
 
 
 */




/// Decoder implementation that does nothing -- i.e. just testing NMEA decoding performance
class AisDummyDecoder : public AIS::AisDecoder
{
 public:
    AisDummyDecoder(int _iIndex)
        :AisDecoder(_iIndex)
    {}
    
 protected:
	 virtual void onType123(unsigned int, unsigned int, unsigned int, int, unsigned int, bool, int, int, int, int) override {}

	 virtual void onType411(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, bool, int, int) override {}

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

     virtual void onDecodeError(const AIS::StringRef &, const std::string &_strError) override {printf("%s\n", _strError.c_str());}
};


/// decoder callback that just prints progress/stats to console
void progressCb(size_t , const AIS::AisDecoder &)
{
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
    AIS::DefaultSentenceParser parser;
    AIS::processAisFile(std::string(EXAMPLE_DATA_PATH) + "/" + _strLogPath, decoder, parser, BLOCK_SIZE, progressCb);
    
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
    
    //return 0;
}
