
#include "../../ais_decoder/ais_quick.h"
#include "../utils.h"


void pushSentence(const char *_pszSentence)
{
    pushAisSentence(_pszSentence, strlen(_pszSentence), 0);
}

int main()
{
    pushSentence("!AIVDM,1,1,,A,13HOI:0P0000VOHLCnHQKwvL05Ip,0*23\n");
    pushSentence("!AIVDM,1,1,,A,133sVfPP00PD>hRMDH@jNOvN20S8,0*7F\n");
    pushSentence("!AIVDM,1,1,,B,100h00PP0@PHFV`Mg5gTH?vNPUIp,0*3B\n");
    pushSentence("!AIVDM,1,1,,B,13eaJF0P00Qd388Eew6aagvH85Ip,0*45\n");
    pushSentence("!AIVDM,1,1,,A,14eGrSPP00ncMJTO5C6aBwvP2D0?,0*7A\n");
    pushSentence("!AIVDM,1,1,,A,15MrVH0000KH<:V:NtBLoqFP2H9:,0*2F\n");
    
    auto msg1 = popAisMessage();
    auto msg2 = popAisMessage();
    auto msg3 = popAisMessage();
    auto msg4 = popAisMessage();
    
    return 0;
}






