#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Sep 12 10:41:46 2018

@author: rory
"""
# Standard Libs
import time
import logging
import os

# Special Libs
import ais_decoder

# My Libs
import ais_helper

log = logging.getLogger(__name__)

def read_in_chunks(file_object, chunk_size=1024*1024*4):
    """Lazy function (generator) to read a file piece by piece.
    Default chunk size: 4M. STACKOVERFLOW"""
    while True:
        data = file_object.read(chunk_size)
        if not data:
            break
        yield data


def the_bigun(CFG):
    start = time.time() 
    folder = CFG.get('Other','Folder')
    log.info('Reading AIS files from {0}'.format(folder))
    count_msg = 0
    count_decoded = 0
    count_error = 0
    count_crc = 0
    count_none = 0
    count_no_valid = 0 
    for filename in os.listdir(folder):
        try:
            log.info('Reading file: {0}'.format(filename))
            f = open(os.path.join(folder,filename))
     
            for piece in read_in_chunks(f):
                ais_decoder.pushAisChunk(piece, len(piece))       
                n = ais_decoder.numAisMessages()
                log.info("Num messages =  {0}".format(n))   
            
                while True:
                    if ais_decoder.numAisMessages() == 0:
                        break
                    # log.info('--------------------------')
                    msg = ais_decoder.popAisMessage().asdict()
                    count_msg += 1
                    try:
                        if msg['msg'] == '0':
                            # log.warning('Decoder error: {0}'.format(msg.get('error')))
                            # log.warning(msg)
                            count_error += 1
                            if msg.get('error') == 'Sentence decoding error. CRC check failed.':
                                count_crc += 1
                            elif msg.get('error') == 'Sentence decoding error. No valid NMEA data found.':
                                count_no_valid += 1
                            elif msg.get('error') is None:
                                count_none += 1
                        clean_msg = ais_helper.ais_handler(msg, source_type = 'SAT')     
                        # log.info('Decoded: {0}'.format(clean_msg))
                        count_decoded += 1
                    except Exception as Error:
                        # log.warning("Problem with cleaning AIS message: {0}".format(Error))
                        # log.warning(msg)
                        pass
        
        except RuntimeError as err:
            log.warning("Runtime error: {0} ".format(err))
        except Exception as err:
            log.warning("Other error: {0} ".format(err))
    
        duration = time.time() - start
        log.info("Num reported messages:  {0}".format(n)) 
        log.info('MSGs processed: {0} --- {1}%'.format(count_msg, count_msg/count_msg*100 ))
        log.info('Decoded: {0} --- {1}%'.format(count_decoded, count_decoded/count_msg*100))
        log.info('Failed : {0} --- {1}%'.format(count_error, count_error/count_msg*100))
        log.info('Failed None: {0} --- {1}%'.format(count_none, count_none/count_msg*100))
        log.info('Failed CRC: {0} --- {1}%'.format(count_crc, count_crc/count_msg*100))
        log.info('Failed Valid: {0} --- {1}%'.format(count_no_valid, count_no_valid/count_msg*100))
        log.info("Done in {0} secs".format(duration))



        
        

# 2019-04-19 00:00:00,855: \s:66,c:1555624743*38\!AIVDM,1,1,,,144hw5030=0>lhl1WhLJUUr40@1?,0*24
# ais_tester_1  | 2020-03-17 07:36:59,396 - WARNING - jobs - {'cog': '2710', 'footer': '', 'header': '2019-04-19 00:00:00,855: \\s:66,c:1555624743*38', 'heading': '189', 
# 'mmsi': '273432340', 'msg': '1', 'nav_status': '0', 'pos_accuracy': '0', 'pos_lat': '1699953', 'pos_lon': '1943066', 'rot': '12', 'sog': '13', 'timestamp': '1555624743'}
#  18 Apr 2019 21:59:03 GMT
# 2019 23:59:03 GMT+0200
