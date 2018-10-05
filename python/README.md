
Create a python module
==================


swig -Wall -c++ -python ais_decoder.i
c++ -c -fPIC ais_decoder_wrap.cxx -I /System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7/
c++ -shared ais_decoder_wrap.o -lpython -lais_decoder -o _ais_decoder.so



Import and use
===========

import ais_decoder

ais_decoder.pushSentence("!AIVDM,1,1,,A,13HOI:0P0000VOHLCnHQKwvL05Ip,0*23\n")

msg = ais_decoder.popMessage()
