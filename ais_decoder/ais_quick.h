
#ifndef AIS_QUICK_H
#define AIS_QUICK_H


#include <string>
#include <map>



/**
 Push new data onto the decoder.
 Scans for a complete line and only consumes one line at a time.
 Returns the number of bytes processed.
 Slower than processing a whole chunk, see 'pushAisChunk(...)'.
 */
int pushAisSentence(const char *_pNmeaBuffer, size_t _uBufferSize);

/// Push new data onto the decoder. Scans for all sentences in data and buffers remaining data until the next call. Returns the number of bytes processed.
void pushAisChunk(const char *_pNmeaBuffer, size_t _uBufferSize);

/// Pop next message as key value pairs. Returns and empty map if no new messages are available.
std::map<std::string, std::string> popAisMessage();

/// Check how many messages are available.
int numAisMessages();

/// translates MMSI to country/flag
std::string mmsi2country(const std::string &_strMmsi);

/// translates MMSI to transmitter class
std::string mmsi2class(const std::string &_strMmsi);



#endif  // #ifndef AIS_QUICK_H

