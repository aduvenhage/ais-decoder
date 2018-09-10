## Simple AIS NMEA Message Decoder

This project was created to learn more about AIS and see how easy it would be to create a decoder for the NMEA strings. The NMEA string decoding is implemented according to: 'http://catb.org/gpsd/AIVDM.html'.

The decoder consists of a base class that does the decoding, with pure virtual user callbacks for each AIS messages type. Some time was also spent on improving the speed of the NMEA string processing to see how quickly NMEA logs could be processed.

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
