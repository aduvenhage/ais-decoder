/* ais_decoder.i */
%module ais_decoder
%{
    /* Put header files here or function declarations like below */
    #include "ais_decoder/ais_quick.h"
%}

extern int pushSentence(const char *_pszSentence);
extern std::map<std::string, std::string> popMessage();

