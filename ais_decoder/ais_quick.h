
#ifndef AIS_QUICK_H
#define AIS_QUICK_H


#include <string>
#include <map>



/// \todo the quick interface's internal decoder object is not complete yet
namespace AIS
{

    /// Push new data onto the decoder. Scans for a complete line and only consumes one line at a time. Returns the number of bytes processed.
    int pushSentence(const char *_pNmeaBuffer, size_t _uBufferSize, size_t _uOffset);


    /// Pop next message as key value pairs. Returns and empty map if no new messages are available.
    std::map<std::string, std::string> popMessage();

};


/// Push new data onto the decoder. Scans for a complete line and only consumes one line at a time. Returns the number of bytes processed.
inline int pushAisSentence(const char *_pNmeaBuffer, size_t _uBufferSize, size_t _uOffset) {
    return AIS::pushSentence(_pNmeaBuffer, _uBufferSize, _uOffset);
}


/// Pop next message as key value pairs. Returns and empty map if no new messages are available.
inline std::map<std::string, std::string> popAisMessage() {
    return AIS::popMessage();
}


#endif  // #ifndef AIS_QUICK_H

