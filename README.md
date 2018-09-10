Simple AIS NMEA Message Decoder
===============================

This project was created to learn more about AIS and see how easy it would be to create a decoder for the NMEA strings. The NMEA string decoding is implemented according to: 'http://catb.org/gpsd/AIVDM.html'.

The decoder consists of a base class that does the decoding, with pure virtual user callbacks for each AIS messages type.


