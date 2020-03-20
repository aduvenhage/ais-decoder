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
    for filename in os.listdir(folder):
        try:
            log.info('Reading file: {0}'.format(filename))
            f = open(os.path.join(folder,filename))
     
            for piece in read_in_chunks(f):
                ais_decoder.pushAisChunk(piece, len(piece))          
            
                while True:
                    if ais_decoder.numAisMessages() == 0:
                        break
                    log.warning('--------------------------')
                    msg = ais_decoder.popAisMessage().asdict()
                    try:
                        if msg['msg'] == '0':
                            log.warning('Decoder error: {0}'.format(msg.get('error')))
                            log.warning(msg)
                        clean_msg = ais_helper.ais_handler(msg, source_type = 'SAT')     
                        # log.info('Decoded: {0}'.format(clean_msg))
                    except Exception as Error:
                        log.warning("Problem with cleaning AIS message: {0}".format(Error))
                        log.warning(msg)
        
        except RuntimeError as err:
            log.warning("Runtime error: {0} ".format(err))
        except Exception as err:
            log.warning("Other error: {0} ".format(err))
    
        duration = time.time() - start
        log.info("Done in {0} secs".format(duration))



        
        

# 2019-04-19 00:00:00,855: \s:66,c:1555624743*38\!AIVDM,1,1,,,144hw5030=0>lhl1WhLJUUr40@1?,0*24
# ais_tester_1  | 2020-03-17 07:36:59,396 - WARNING - jobs - {'cog': '2710', 'footer': '', 'header': '2019-04-19 00:00:00,855: \\s:66,c:1555624743*38', 'heading': '189', 
# 'mmsi': '273432340', 'msg': '1', 'nav_status': '0', 'pos_accuracy': '0', 'pos_lat': '1699953', 'pos_lon': '1943066', 'rot': '12', 'sog': '13', 'timestamp': '1555624743'}
#  18 Apr 2019 21:59:03 GMT
# 2019 23:59:03 GMT+0200
