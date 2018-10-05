
#ifndef AIS_QUICK_H
#define AIS_QUICK_H


#include <string>
#include <map>


/// \todo the quick interface's internal decoder object is not complete yet


int pushSentence(const char *_pszSentence);
std::map<std::string, std::string> popMessage();



#endif  // #ifndef AIS_QUICK_H

