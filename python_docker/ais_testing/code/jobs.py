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
                    msg = ais_decoder.popAisMessage().asdict()
                    try:
                        clean_msg = ais_helper.ais_handler(msg)     
                        log.info('Decoded: {0}'.format(clean_msg))
                    except Exception as Error:
                        log.warning("Problem with cleaning AIS message: {0}".format(Error))
                        log.warning(msg)
        
        except RuntimeError as err:
            log.warning("Runtime error: {0} ".format(err))
        except Exception as err:
            log.warning("Other error: {0} ".format(err))
    
        duration = time.time() - start
        log.info("Done in {0} secs".format(duration))