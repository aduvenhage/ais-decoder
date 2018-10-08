## Simple AIS NMEA Message Decoder (v2.0)
This project was created to learn more about AIS and see how easy it would be to create a decoder for the NMEA strings. The NMEA string decoding is implemented according to: 'http://catb.org/gpsd/AIVDM.html'.  The key component to implement was the 6bit nibble packing and unpacking of arbitrarily sized signed and unsigned integers as well as strings (see PayloadBuffer in ais_decoder.h).

The decoder consists of a base class that does the decoding, with pure virtual methods for each AIS messages type.  A user of the decoder has to inherit from the decoder class and implement/override 'onTypeXX(...)' style methods as well as error handling methods (see the examples, for how this is done).  Basic error checking, including CRC checks, are done and also reported.

The current 'onTypeXX(...)' message callback are unique for each message type (types 1,2,3,4,5,18,19 & 24 currently supported).  No assumtions are made on default or blank values -- all values are returned as integers and the user has to scale and convert the values like position and speed to floats and the desired units.

Some time was also spent on improving the speed of the NMEA string processing to see how quickly NMEA logs could be processed.  Currently the multi-threaded file reading examples (running a thread per file) achieve more than 3M NMEA messages per second, per thread.  When running on multiple logs concurrently (8 threads is a good number on modern hardware) 12M+ NMEA messages per second is possible.  During testing it was also found that most of the time was spent on the 6bit nibble packing and unpacking, not the file IO.


## Checklist
- [x] Basic payload 6bit nibble stuffing and unpacking
- [x] ASCII de-armouring
- [x] CRC checking
- [x] Multi-Sentence message handling
- [x] Decoder base class
- [x] Support types 1, 2, 3, 4, 5, 18, 19, 24 -- position reports and static info
- [x] Test with large data-sets (files)
- [x] Validate payload sizes (reject messages, where type and size does not match)
- [x] Build-up message stats (bytes processed, messages processed, etc.)
- [x] Profile and improve speed 
- [x] Validate fragment count and fragment number values
- [x] Investigate faster ascii de-armouring and bit packing techniques (thanks to Frans van den Bergh)
- [x] Add python interface

- [ ] Validate talker IDs
- [ ] Look at multiple threads/decoders working on the same file, for very large files
- [ ] Support NMEA files/data with non-standard meta data, timestamp data, etc.
- [ ] Add minimal networking to work with RTL-AIS (https://github.com/dgiardini/rtl-ais.git) and also to forward raw data

## Build
This project uses CMAKE to build.  To build through command line on linux, do the following:

- git clone https://github.com/aduvenhage/ais-decoder.git
- mkdir ais-decoder-build
- cd ais-decoder-build
- cmake ../ais-decoder -DCMAKE_BUILD_TYPE=RELEASE
- make
- sudo make install


## Examples
The project includes some examples of how to use the AIS decoder lib.


## Create a python module (WIP)
Use SWIG to compile a python module.  The module is built around the 'ais_quick' interface. See 'examples/quick'. The SWIG interface file is located at 'python/ais_decoder.i'. Follow the decoder lib build instructions -- if the library is installed on the system the SWIG steps are easier.

To build and install

```
cd python
python setup.py build
sudo python setyp.py install
```

To build manually (tested with MacOS):

```
cd python
swig -Wall -c++ -python ais_decoder.i
c++ -c -fPIC ais_decoder_wrap.cxx -I /System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7/
c++ -shared ais_decoder_wrap.o -lpython -lais_decoder -o _ais_decoder.so
```

Make sure you use the correct python lib for the version you will be working with.


## Import and use python module (WIP)
In python do the following to test:

```
import ais_decoder

str = "!AIVDM,1,1,,A,13HOI:0P0000VOHLCnHQKwvL05Ip,0*23\n"
ais_decoder.pushAisSentence(str, len(str), 0)

n = ais_decoder.numAisMessages()
msg = ais_decoder.popAisMessage().asdict()
```

Message fragments, for multi-fragment messages, are managed and stored internally.  'msg' will be empty if no output is ready yet. 'pushAisSentence(...)' scans for one sentence only and sentences should always end with a newline.  'pushAisSentence(...)' also throws Python 'RuntimeError' exceptions on message errors.

The interface also has 'pushChunk(data, len)' that accepts any number of messages. Incomplete sentences at the end of the supplied chunk will be buffered until the next call.
