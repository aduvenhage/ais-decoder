## Simple AIS NMEA Message Decoder

This project was created to learn more about AIS and see how easy it would be to create a decoder for the NMEA strings. The NMEA string decoding is implemented according to: 'http://catb.org/gpsd/AIVDM.html'.  The key component to implement was the 6bit nibble packing and unpacking of arbitrarily sized signed and unsigned integers as well as strings (see PayloadBuffer in ais_decoder.h).

The decoder consists of a base class that does the decoding, with pure virtual methods for each AIS messages type.  An user of the decoder has to inherit from the decoder class and implement/override 'onTypeXX(...)' style methods as well as error handling methods (see the examples, for how this is done).  

Some time was also spent on improving the speed of the NMEA string processing to see how quickly NMEA logs could be processed.  Currently the multi-threaded file reading examples (running a thread per file) achieve more than 500k NMEA messages per second per thread.  When running on multiple logs concurrently (8 threads is a good number on modern hardware) 4M+ NMEA messages per second is possible.  During testing it was also found that most of the time was spent on the 6bit nibble packing and unpacking, not the file IO.


## Build
This project uses CMAKE to build.  To build through command line on linux, do the following:

- git clone https://github.com/aduvenhage/ais-decoder.git
- mkdir ais-decoder-build
- cd ais-decoder-build
- cmake ../ais-decoder -DCMAKE_BUILD_TYPE=RELEASE
- make
- sudo make install

** Currently the 'develop' branch is the branch to use -- the 'master' branch is empty/outdated.

## Examples
The project includes some examples of how to use the AIS decoder lib.
