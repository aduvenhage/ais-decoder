
#ifndef AIS_QUICK_H
#define AIS_QUICK_H


#include <string>
#include <map>



/// Push new data onto the decoder. Scans for a complete line and only consumes one line at a time. Returns the number of bytes processed.
int pushAisSentence(const char *_pNmeaBuffer, size_t _uBufferSize, size_t _uOffset);

/// Pop next message as key value pairs. Returns and empty map if no new messages are available.
std::map<std::string, std::string> popAisMessage();

/// Check how many messages are available.
int numAisMessages();



#endif  // #ifndef AIS_QUICK_H

