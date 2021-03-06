#!/usr/bin/env python

"""
MR17
"""

__author__ = 'Trevor Stanhope'

# Modules
from cherrypy.process.plugins import Monitor
import json
import cherrypy
from cherrypy import tools
from pymongo import MongoClient
from bson import json_util
from pymongo import MongoClient
import numpy as np
import sys, os
import serial
import binascii
import random
import cvme
from datetime import datetime

class Logger:
    def __init__(self, fname, ftype=".csv"):
        self.file = open(os.path.join("logs", fname + ftype), 'w')
    def write_headers(self, d):
        self.file.write(','.join([str(k) for k,v in d.iteritems()] + ['\n']))
    def insert_data(self, d):
        self.file.write(','.join([str(v) for k,v in d.iteritems()] + ['\n']))

class OBD:
    def __init__(self, debug=False):
        """
        """
        self.port = None
        self.debug = debug

        # Initialize computer vision ground speed sensor
        try:
            self.CLORB = CLORB()
        except:
            self.CLORB = None

        # JSON Data to be transmitted to the app over the web API
        self.json_data = {
            "slip" : 0,
            "cvt_ratio" : 0,
            "belt_slip" : 0,
            "cvt_pct" : 0,
            "rpm" : 0,
            "gear" : 0,
            "trans_temp" : 0,
            "throttle" : 0,
            "lbrake" : 0,
            "rbrake" : 0,
            "bat" : 0,
            "user" : 0,
            "lock" : 0,
            "eng_temp" : 0,
            "oil" : 0,
            "load" : 0,
            "susp" : 0,
            "ballast" : 0,
            "vel" : 0,
            "hours" : 0
        }
        
        # ECU Data to be refreshed by querying the OBD
        self.esc_a_data = {
            "run_mode" : 0,
            "trigger" : 0,
            "pull_mode" : 0,
            "cvt_sp" : 0,
            "throttle" : 0,
            "cart_mode" : 0,
            "cart_dir" : 0
        }
        self.esc_b_data = {
            "left_brake" : 0,
            "right_brake" : 0,
            "temp" : 0,
            "lph" : 0,
            "psi" : 0,
            "voltage" : 0,
            "rfid_auth" : 0
        }
        self.vdc_data = {
            "steering_sp" : 0,
            "steering_pv" : 0,
            "mot1" : 0,
            "susp_pv" : 0,
            "mot2" : 0,
            "slot6" : 0,
            "slot7" : 0,
        }
        self.tsc_data = {
            "engine_rpm" : 0,
            "shaft_rpm" : 0,
            "guard" : 0,
            "cvt_pv" : 0,
            "cart_sp" : 0,
            "gear" : 0,
            "lock" : 0
        }

    # Connect to OBD
    def attach(self, device_classes=['/dev/ttyUSB','/dev/ttyACM'], attempts=5, baud=38400):
        """
        """
        if self.port is None:
            for i in range(attempts):
                for dev in device_classes:
                    self.dev_id = None
                    try:
                        self.dev_id = dev + str(i)
                        self.port = serial.Serial(self.dev_id, baud) # set self.port to the located device
                        return True
                    except Exception as e:
                        self.port = None
            if self.port is None:
                raise Exception("Could not locate OBD device!")
        else:
            raise Exception("Port already attached!")
    
    # Query CAN
    def query(self, ESC_A_ID = 9, ESC_B_ID = 10, TSC_ID = 11, VDC_ID = 12):
        """
        Grab the latest data from the CANBus via the OBD gateway
        Display requires the following keys:
        See src/app/display.tsc for required key-values
        """
        if self.port is not None:
            try:
                string = self.port.readline()
            except:
                raise Exception("Failed to read from OBD")
            try:
                msg = json.loads(string) # parse JSON
            except:
                raise Exception("Failed to parse message as JSON!")
            try:
                data = msg['data'] # grab data component of string
                checksum_b = self.checksum(data) # calculate checksum
            except:
                raise Exception("No sensor values found in message!")
            try:
                checksum_a = int(msg['chksum']) # parse checksum
            except:
                raise Exception("No checksum value found in message!")
            if checksum_a == checksum_b:
                try:
                    id = int(msg["id"])
                except:
                    raise Exception("No ID for CANBUS message found!")
#                self.print_msg(data)
                if id == TSC_ID:
                    self.tsc_data.update(data)
                elif id == VDC_ID:
                    self.vdc_data.update(data)
                elif id == ESC_A_ID:
                    self.esc_a_data.update(data)
                elif id == ESC_B_ID:
                    self.esc_b_data.update(data)
                else:
                    raise Exception("CANBUS ID not recognized!: %d" % id)
                self.print_msg("Updated ID%d values" % id)

    def print_msg(self, msg):
        print datetime.strftime(datetime.now(), "[%d/%b/%Y:%H:%H:%S]") + ' CANBUS ' + str(msg)


    def get_latest(self):
        self.query() # grab latest info from CAN
        if self.debug is True:
            dummy_data = {
                "belt_slip" : random.randint(0, 100),
                "cvt_pct" : random.randint(0, 100),
                "cvt_ratio" : round(random.uniform(0,4), 2),
                "rpm" : random.randint(0, 3600),
                "gear" : random.randint(-1, 3),
                "trans_temp" : round(random.uniform(25, 27), 1),
                "throttle" : random.randint(0, 100),
                "lbrake" : random.randint(0, 100),
                "rbrake" : random.randint(0, 100),
                "bat" : random.randint(0, 24),
                "user" : random.randint(0, 255),
                "lock" : random.randint(0, 1),
                "eng_temp" : random.randint(0, 999),
                "oil" : random.randint(0, 100),
                "load" : random.randint(0, 100),
                "susp" : random.randint(0, 100),
                "ballast" : ["Forward", "Backward", "Off"][random.randint(0, 2)],
                "vel" : random.randint(0, 50),
                "slip" : random.randint(0, 100),
                "hours" : random.randint(0, 100),
            }
            self.json_data.update(dummy_data)
        else:
            if self.CLORB:
                groundspeed = self.CLORB.get_groundspeed()
                slip = self.calculate_slip(groundspeed)
                self.json_data.update({"vel" : groundspeed})
                self.json_data.update({"slip" : self.calculate_slip(groundspeed)}) 

            # VDC
            self.json_data.update({                
                "susp" : self.vdc_data["susp_pv"],
            })

            # ESC-A
            self.json_data.update({
                "throttle" : self.esc_a_data["throttle"],
            })

            # ESC-B
            self.json_data.update({
                "rbrake" : self.esc_b_data["right_brake"],
                "lbrake" : self.esc_b_data["left_brake"],
                "eng_temp" : self.esc_b_data["temp"],
                "lph" : self.esc_b_data["lph"],
                "oil" : self.esc_b_data["psi"]
            })

            # TSC
            self.json_data.update({
                "gear" : self.tsc_data["gear"],
                "cvt_pct" : self.tsc_data["cvt_pv"],
                "gear" : self.tsc_data["gear"],
                "rpm" : self.tsc_data["engine_rpm"],
                "cvt_ratio" : self.calculate_cvt_ratio()
            })
        return self.json_data

    # Calculate Checksum
    def checksum(self, d, mod=256, decimals=2):
        """
        Returns: mod N checksum
        """
        chksum = 0
        d = {k.encode('ascii'): v for (k, v) in d.iteritems()}
        for k,v in d.iteritems():
            if v is float:
                d[k] = round(v,decimals)
        s = str(d)
        r = s.replace(' ', '').replace('\'', '\"')
        for i in r:
            chksum += ord(i)
        return chksum % mod

    # Get Device
    def get_device(self):
        """
        Returns the device ID
        """
        if self.dev_id is not None:
            return self.dev_id
        else:
            return False

    # Calculate Slip
    def calculate_slip(self, ground_kmh, effective_radius=28.0):
        gear = self.tsc_data["gear"]
        shaft_rpm = self.tsc_data["shaft_rpm"]
        if gear == 1:
            ratio = 43.28
        elif gear == 2:
            ratio = 35.88
        elif gear == 3:
            ratio = 15.95
        elif gear == 4:
            ratio = 47.84
        else:
            return 0.0
        axle_rpm = shaft_rpm / ratio
        wheel_kmh = 2 * np.pi * (effective_radius / 1e5) * (60 * axle_rpm)
        slip = wheel_kmh / ground_kmh 
        return slip

    def calculate_cvt_ratio(self):
        shaft_rpm = self.tsc_data["shaft_rpm"]
        engine_rpm = self.tsc_data["engine_rpm"]
        if shaft_rpm == 0:
           cvt_ratio = -1
        else:
           cvt_ratio = engine_rpm / shaft_rpm
        return cvt_ratio    

# Web Interface
class App:

    def __init__(self, config_file):
        try:
            with open(config_file) as cfg:
                self.config = json.loads(cfg.read())
            self.session_key = binascii.b2a_hex(os.urandom(self.config['SESSION_KEY_LENGTH']))
            self.latest_data = {}

            # Mongo
            try:
                self.mongo_client = MongoClient()
                self.db = self.mongo_client[self.config['MONGO_NAME']]
                self.session = self.db[self.session_key]
            except:
                self.print_error('MONGO', e)

            # OBD
            try:
                self.obd = OBD()
                self.obd.attach()
                self.obd.get_device()
            except Exception as e:
                self.print_error('OBD', e)

            # Logger
            try:
                self.log = Logger(self.session_key)
                self.log.write_headers(self.obd.json_data)
            except Exception as e:
                self.print_error('LOGGER', e)

            # Scheduled Tasks
            try:
                Monitor(cherrypy.engine, self.listen, frequency=self.config["POLLING_FREQ"]).subscribe()
            except Exception as e:
                raise e
        except Exception as e:
            self.print_error('SYSTEM', e)

    ## Listen
    def listen(self):
        try:
            self.latest_data = self.obd.get_latest()
            self.log.insert_data(self.latest_data)
        except Exception as e:
            print str(e)

    ## Print Error
    def print_error(self, subsystem, e):
        print datetime.strftime(datetime.now(), "[%d/%b/%Y:%H:%H:%S]") + ' ' + subsystem + ' ' + str(e)

    ## Render Webapp
    @cherrypy.expose 
    def index(self, index_file="index.html"):
	path = os.path.join(self.config['CHERRYPY_PATH'], index_file)
        with open(path) as html: 
            return html.read()
	
    ## Handle API Requests
    @cherrypy.expose
    @tools.json_out()
    @cherrypy.tools.accept(media='application/json')
    def default(self, *args, **kwargs):
        """ This function the API """
        try:
            if args[0] == 'api':
                self.print_error("JSON", "Caught request from app API")
                return self.latest_data
        except Exception as e:
            raise e
        return {}

if __name__ == '__main__':
    config_file = "settings.json"
    app = App(config_file)
    cherrypy.server.socket_host = "0.0.0.0"
    cherrypy.server.socket_port = 8080
    currdir = os.path.dirname(os.path.abspath(__file__))
    conf = {
        '/': {'tools.staticdir.on':True, 'tools.staticdir.dir':os.path.join(currdir,app.config['CHERRYPY_PATH'])},
    }
    cherrypy.quickstart(app, '/', config=conf)
