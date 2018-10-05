/* ais_decoder.i */
%module ais_decoder
%{
    /* Put header files here or function declarations like below */
    #include "ais_decoder/ais_quick.h"
%}

%include std_string.i
%include std_map.i

%template(pairss) std::pair<std::string, std::string>;
%template(mapss) std::map<std::string, std::string>;

extern int pushSentence(const char *_pszSentence);
extern std::map<std::string, std::string> popMessage();

