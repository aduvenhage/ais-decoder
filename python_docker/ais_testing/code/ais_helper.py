#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Oct 26 09:00:54 2018

@author: rory
"""
# Standard Libs
import datetime
import logging

# Special Libs
#import ais_decoder

# My Libs


log = logging.getLogger(__name__)


def ais_handler(ais_bit_dict, source_type = 'TNPA'):
    ais_message_type = ais_bit_dict['msg']
    if source_type == 'TNPA': 
        msg_datetime, ais_bit_dict['event_time']  = tnpa_meta_handler(ais_bit_dict)
    else:
        ais_bit_dict['event_time'] = datetime.datetime.now()
        
    if ais_message_type in ['1','2','3']: return msg_1_2_3_handler(ais_bit_dict)
    elif ais_message_type in ['4']: return msg_4_handler(ais_bit_dict)
    elif ais_message_type in ['5']: return msg_5_handler(ais_bit_dict, msg_datetime.year)
    elif ais_message_type in ['6']: return msg_6_handler(ais_bit_dict)
    elif ais_message_type in ['7']: return msg_7_handler(ais_bit_dict)
    elif ais_message_type in ['8']: return msg_8_handler(ais_bit_dict)
    elif ais_message_type in ['9']: return msg_9_handler(ais_bit_dict)
    elif ais_message_type in ['10']: return msg_10_handler(ais_bit_dict)
    elif ais_message_type in ['11']: return msg_11_handler(ais_bit_dict)
    elif ais_message_type in ['12']: return msg_12_handler(ais_bit_dict)
    elif ais_message_type in ['13']: return msg_13_handler(ais_bit_dict)
    elif ais_message_type in ['14']: return msg_14_handler(ais_bit_dict)
    elif ais_message_type in ['15']: return msg_15_handler(ais_bit_dict)
    elif ais_message_type in ['16']: return msg_16_handler(ais_bit_dict)
    elif ais_message_type in ['17']: return msg_17_handler(ais_bit_dict)
    elif ais_message_type in ['18']: return msg_18_handler(ais_bit_dict)
    elif ais_message_type in ['19']: return msg_19_handler(ais_bit_dict)
    elif ais_message_type in ['21']: return msg_21_handler(ais_bit_dict)
    elif ais_message_type in ['22']: return msg_22_handler(ais_bit_dict)
    elif ais_message_type in ['23']: return msg_23_handler(ais_bit_dict)
    elif ais_message_type in ['24']: return msg_24_handler(ais_bit_dict, msg_datetime.year)
    elif ais_message_type in ['25']: return msg_25_handler(ais_bit_dict)
    elif ais_message_type in ['26']: return msg_26_handler(ais_bit_dict)
    elif ais_message_type in ['27']: return msg_27_handler(ais_bit_dict)
    else: return ais_bit_dict
    

def tnpa_meta_handler(ais_bit_dict):
    iso_time = datetime.datetime.fromtimestamp(int(ais_bit_dict['footer'].strip(',')))
    
    return iso_time, iso_time.strftime('%Y-%m-%d %H:%M:%S')
    
def msg_1_2_3_handler(ais_bit_dict):
    ais_data_dict = {}
    for key,value in ais_bit_dict.items():
            if key == 'cog':
                if value == '3600': ais_data_dict['cog'] = None
                else: ais_data_dict['cog'] = int(value)/10.0
            elif key == 'pos_lat':
                if value == '54600000': ais_data_dict['latitude'] = None
                else: ais_data_dict['latitude'] = int(value)/600000.0
            elif key == 'pos_lon': 
                if value == '108600000': ais_data_dict['longitude'] = None
                else: ais_data_dict['longitude'] = int(value)/600000.0
            elif key == 'rot':
                if value == '128': ais_data_dict['rot'] = None
                else: ais_data_dict['rot'] = (int(value)/4.733)**2
            elif key == 'sog':
                if value == '1023': ais_data_dict['sog'] = None
                else: ais_data_dict['sog'] = int(value)/10.0
            elif key == 'heading':
                if value == '511': ais_data_dict['hdg'] = None
                else: ais_data_dict['hdg'] = int(value)/1
            else: ais_data_dict[key] = value
            
    return ais_data_dict

def msg_4_handler(ais_bit_dict):
    ais_data_dict = ais_bit_dict.copy()
    YY = str(ais_bit_dict['year']).zfill(2)
    MM = str(ais_bit_dict['month']).zfill(2)
    dd = str(ais_bit_dict['day']).zfill(2)
    hh = str(ais_bit_dict['hour']).zfill(2)
    mm = str(ais_bit_dict['minute']).zfill(2)
    ss = str(ais_bit_dict['second']).zfill(2)
    ais_data_dict['base_station_time'] = '{0}-{1}-{2} {3}:{4}:{5}+00'.format(YY,MM,dd,hh,mm,ss)  
    
    for key, value in ais_bit_dict.items():
        if key == 'pos_lat':
            if value == '54600000': ais_data_dict['latitude'] = None
            else: ais_data_dict['latitude'] = int(value)/600000.0
        elif key == 'pos_lon': 
            if value == '108600000': ais_data_dict['longitude'] = None
            else: ais_data_dict['longitude'] = int(value)/600000.0
    return ais_data_dict


def msg_5_handler(ais_bit_dict, year):
    ais_data_dict = ais_bit_dict.copy()
    try:
        MM = str(ais_bit_dict['eta_month']).zfill(2)
        dd = str(ais_bit_dict['eta_day']).zfill(2)
        hh = str(ais_bit_dict['eta_hour']).zfill(2)
        mm = str(ais_bit_dict['eta_minute']).zfill(2)
        
        ais_data_dict['eta'] = '{0}-{1}-{2} {3}:{4}:00+00'.format(year, MM,dd,hh,mm)  
    except:
        pass
    
    try:
        ais_data_dict['length'] = int(ais_bit_dict['to_bow']) + int(ais_bit_dict['to_stern'])
        ais_data_dict['width'] = int(ais_bit_dict['to_port']) + int(ais_bit_dict['to_starboard'])
    except:
        pass
    return ais_data_dict

def msg_6_handler(ais_bit_dict):
    return ais_bit_dict

def msg_7_handler(ais_bit_dict):
    return ais_bit_dict

def msg_8_handler(ais_bit_dict):   
    return ais_bit_dict

def msg_9_handler(ais_bit_dict):
    ais_data_dict = {}
    for key,value in ais_bit_dict.items():
        if key == 'cog':
            if value == '3600': ais_data_dict['cog'] = None
            else: ais_data_dict['cog'] = int(value)/10.0
        elif key == 'pos_lat':
            if value == '54600000': ais_data_dict['latitude'] = None
            else: ais_data_dict['latitude'] = int(value)/600000.0
        elif key == 'pos_lon': 
            if value == '108600000': ais_data_dict['longitude'] = None
            else: ais_data_dict['longitude'] = int(value)/600000.0
        elif key == 'rot':
            if value == '128': ais_data_dict['rot'] = None
            else: ais_data_dict['rot'] = (int(value)/4.733)**2
        elif key == 'sog':
            if value == '1023': ais_data_dict['sog'] = None
            else: ais_data_dict['sog'] = int(value)/10.0
        elif key == 'heading':
            if value == '511': ais_data_dict['hdg'] = None
            else: ais_data_dict['hdg'] = int(value)/1
        else: ais_data_dict[key] = value
    return ais_data_dict

def msg_10_handler(ais_bit_dict):   
    return ais_bit_dict

def msg_11_handler(ais_bit_dict):
    ais_data_dict = ais_bit_dict
    YY = str(ais_bit_dict['eta_month']).zfill(2)
    MM = str(ais_bit_dict['eta_month']).zfill(2)
    dd = str(ais_bit_dict['eta_day']).zfill(2)
    hh = str(ais_bit_dict['eta_hour']).zfill(2)
    mm = str(ais_bit_dict['eta_minute']).zfill(2)
    ss = str(ais_bit_dict['eta_minute']).zfill(2)
    ais_data_dict['base_station_time'] = '{0}-{1}-{2} {3}:{4}:{5}+00'.format(YY,MM,dd,hh,mm,ss)  
    
    for key, value in ais_bit_dict.items():
        if key == 'pos_lat':
            if value == '54600000': ais_data_dict['latitude'] = None
            else: ais_data_dict['latitude'] = int(value)/600000.0
        elif key == 'pos_lon': 
            if value == '108600000': ais_data_dict['longitude'] = None
            else: ais_data_dict['longitude'] = int(value)/600000.0
        else: ais_data_dict[key] = value
    return ais_data_dict

def msg_12_handler(ais_bit_dict):   
    return ais_bit_dict

def msg_13_handler(ais_bit_dict):  
    return ais_bit_dict

def msg_14_handler(ais_bit_dict):
    return ais_bit_dict

def msg_15_handler(ais_bit_dict):  
    return ais_bit_dict

def msg_16_handler(ais_bit_dict):    
    return ais_bit_dict

def msg_17_handler(ais_bit_dict):
    ais_data_dict = {}
    for key, value in ais_bit_dict.items():
        if key == 'pos_lat':
            if value == '54600': ais_data_dict['latitude'] = None
            else: ais_data_dict['latitude'] = int(value)/600.0
        elif key == 'pos_lon': 
            if value == '108600': ais_data_dict['longitude'] = None
            else: ais_data_dict['longitude'] = int(value)/600.0 
        else: ais_data_dict[key] = value
    return ais_data_dict

def msg_18_handler(ais_bit_dict):
    ais_data_dict = {}
    for key,value in ais_bit_dict.items():
        if key == 'cog':
            if value == '3600': ais_data_dict['cog'] = None
            else: ais_data_dict['cog'] = int(value)/10.0
        elif key == 'pos_lat':
            if value == '54600000': ais_data_dict['latitude'] = None
            else: ais_data_dict['latitude'] = int(value)/600000.0
        elif key == 'pos_lon': 
            if value == '108600000': ais_data_dict['longitude'] = None
            else: ais_data_dict['longitude'] = int(value)/600000.0
        elif key == 'rot':
            if value == '128': ais_data_dict['rot'] = None
            else: ais_data_dict['rot'] = (int(value)/4.733)**2
        elif key == 'sog':
            if value == '1023': ais_data_dict['sog'] = None
            else: ais_data_dict['sog'] = int(value)/10.0
        elif key == 'heading':
            if value == '511': ais_data_dict['hdg'] = None
            else: ais_data_dict['hdg'] = int(value)/1
        else: ais_data_dict[key] = value

    return ais_data_dict

def msg_19_handler(ais_bit_dict):
    ais_data_dict = {}
    for key,value in ais_bit_dict.items():
        if key == 'cog':
            if value == '3600': ais_data_dict['cog'] = None
            else: ais_data_dict['cog'] = int(value)/10.0
        elif key == 'pos_lat':
            if value == '54600000': ais_data_dict['latitude'] = None
            else: ais_data_dict['latitude'] = int(value)/600000.0
        elif key == 'pos_lon': 
            if value == '108600000': ais_data_dict['longitude'] = None
            else: ais_data_dict['longitude'] = int(value)/600000.0
        elif key == 'rot':
            if value == '128': ais_data_dict['rot'] = None
            else: ais_data_dict['rot'] = (int(value)/4.733)**2
        elif key == 'sog':
            if value == '1023': ais_data_dict['sog'] = None
            else: ais_data_dict['sog'] = int(value)/10.0
        elif key == 'heading':
            if value == '511': ais_data_dict['hdg'] = None
            else: ais_data_dict['hdg'] = int(value)/1
        else: ais_data_dict[key] = value 
    return ais_data_dict

def msg_20_handler(ais_bit_dict):
    return ais_bit_dict

def msg_21_handler(ais_bit_dict):
    ais_data_dict = {}
    for key,value in ais_bit_dict.items():
        if key == 'pos_lat':
            if value == '54600000': ais_data_dict['latitude'] = None
            else: ais_data_dict['latitude'] = int(value)/600000.0
        elif key == 'pos_lon': 
            if value == '108600000': ais_data_dict['longitude'] = None
            else: ais_data_dict['longitude'] = int(value)/600000.0
        else: ais_data_dict[key] = value      
        
    try:
        ais_data_dict['length'] = int(ais_bit_dict['to_bow']) + int(ais_bit_dict['to_stern'])
        ais_data_dict['width'] = int(ais_bit_dict['to_port']) + int(ais_bit_dict['to_starboard'])
    except:
        pass
    
    return ais_data_dict

def msg_22_handler(ais_bit_dict):  
    return ais_bit_dict

def msg_23_handler(ais_bit_dict):    
    return ais_bit_dict

def msg_24_handler(ais_bit_dict, year):
    ais_data_dict = ais_bit_dict.copy()
    try:
        MM = str(ais_bit_dict['eta_month']).zfill(2)
        dd = str(ais_bit_dict['eta_day']).zfill(2)
        hh = str(ais_bit_dict['eta_hour']).zfill(2)
        mm = str(ais_bit_dict['eta_minute']).zfill(2)
        
        ais_data_dict['eta'] = '{0}-{1}-{2} {3}:{4}:00+00'.format(year, MM,dd,hh,mm)  
    except:
        pass
    
    try:
        ais_data_dict['length'] = int(ais_bit_dict['to_bow']) + int(ais_bit_dict['to_stern'])
        ais_data_dict['width'] = int(ais_bit_dict['to_port']) + int(ais_bit_dict['to_starboard'])
    except:
        pass
    return ais_data_dict

def msg_25_handler(ais_bit_dict):
    ais_data_dict = {}
    
    return ais_data_dict

def msg_26_handler(ais_bit_dict):
    ais_data_dict = {}
    
    return ais_data_dict

def msg_27_handler(ais_bit_dict):
    ais_data_dict = {}
    for key,value in ais_bit_dict.items():
            if key == 'cog':
                if value == '511': ais_data_dict['cog'] = None
                else: ais_data_dict['cog'] = int(value)
            elif key == 'pos_lat':
                if value == '91000': ais_data_dict['latitude'] = None
                else: ais_data_dict['latitude'] = int(value)/1000.0
            elif key == 'pos_lon': 
                if value == '181000': ais_data_dict['longitude'] = None
                else: ais_data_dict['longitude'] = int(value)/1000.0
            elif key == 'rot':
                if value == '128': ais_data_dict['rot'] = None
                else: ais_data_dict['rot'] = (int(value)/4.733)**2
            elif key == 'sog':
                if value == '63': ais_data_dict['sog'] = None
                else: ais_data_dict['sog'] = int(value)
            elif key == 'heading':
                if value == '511': ais_data_dict['hdg'] = None
                else: ais_data_dict['hdg'] = int(value)/1
            else: ais_data_dict[key] = value
    return ais_data_dict