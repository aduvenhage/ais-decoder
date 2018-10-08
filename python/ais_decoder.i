/* ais_decoder.i */
%module ais_decoder
%{
    /* Put header files here or function declarations like below */
    #include <ais_decoder/ais_quick.h>
%}

%include std_string.i
%include std_map.i
%include exception.i

%exception {
    try {
        $action
    } catch (const std::exception &e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}

%template(pairss) std::pair<std::string, std::string>;
%template(mapss) std::map<std::string, std::string>;

extern int pushAisSentence(const char *_pNmeaBuffer, size_t _uBufferSize, size_t _uOffset);
extern void pushAisChunk(const char *_pNmeaBuffer, size_t _uBufferSize);
extern std::map<std::string, std::string> popAisMessage();
extern int numAisMessages();

