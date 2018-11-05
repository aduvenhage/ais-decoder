# -*- coding: utf-8 -*-
'''
Basic Python starter. Manages logging, job scheduling and reading a config file
Useful in hundreds of different ways. 
'''

#from apscheduler.schedulers.blocking import BlockingScheduler
import argparse
import logging
import configparser
import traceback
#import sys
import os

import jobs


log = logging.getLogger(__name__)


def read_config(cfg_file):
    '''
    same as the hudred other instances of this function
    '''
    CFG = configparser.ConfigParser()
    CFG.read(cfg_file)
    return CFG


def main(args):
    '''
    Setup logging, read config, fire up the scheduling and wait.
    '''
    cfg_file = args.config
    CFG = read_config(cfg_file)
    log_file = CFG.get('Other','Log_file')
    
    logging.basicConfig(
        format='%(asctime)s - %(levelname)s - %(name)s - %(message)s',
        level=getattr(logging, args.loglevel),
        handlers=[
        logging.FileHandler(log_file),
        logging.StreamHandler()])

    log.info('Read config from file: %s',cfg_file)

    try:
        jobs.the_bigun(CFG)
    except (KeyboardInterrupt, SystemExit):
        log.info('Keyboard Interrupt, ending proc...')
    except Exception as Error:
        log.error('ERROR: {0}'.format(Error))
    log.info('Script Ended...')

if __name__ == "__main__":
    '''
    This takes the command line args and passes them to the 'main' function
    '''
    PARSER = argparse.ArgumentParser(
        description='Run the DAFF report.')
    PARSER.add_argument(
        '-c', '--config', help='Config filename',
        default='report_gen.cfg', required=False)  # tests.cfg
    PARSER.add_argument(
        '-ll', '--loglevel', default='INFO',
        choices=['DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL'],
        help="Set log level for service (%s)" % 'DEBUG')
    ARGS = PARSER.parse_args()
    try:
        main(ARGS)
    except KeyboardInterrupt:
        log.warning('Keyboard Interrupt. Exiting...')
        os._exit(0)
    except Exception as error:
        log.error('Other exception. Exiting with code 1...')
        log.error(traceback.print_tb(error.__traceback__))
        log.error(error)
        os._exit(1)
