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
from cvme import CLORB
 
class OBD:

    def __init__(self, device_classes=['/dev/ttyUSB','/dev/ttyACM'], attempts=5, baud=9600, debug=True):
        """
        """

        # Connect to OBD
        self.port = None
        self.debug = debug
        for i in range(attempts):
            for dev in device_classes:
                try:
                    dev_id = dev + str(i)
                    self.port = serial.Serial(dev_id, baud) # set self.port to the located device
                    break
                except Exception as e:
                    self.port = None
                    print str(e)
            if self.port is not None: break

        # Initialize empty data object
        self.data = {
            "slip" : 0,
            "cvt" : 0,
            "rpm" : 0,
            "gear" : 0,
            "cvt_temp" : 0,
            "throttle" : 0,
            "lbrake" : 0,
            "rbrake" : 0,
            "bat" : 0,
            "user" : 0,
            "lock" : 0,
            "engine_temp" : 0,
            "oil" : 0,
            "load" : 0,
            "susp" : 0,
            "ballast" : 0,
            "vel" : 0,
            "hours" : 0,
        }
    
    def query(self):
        """
        """
        if self.port is not None:
            try:
                s = self.port.readline()
            except:
                raise Exception("Failed to read from OBD")
            try:
                j = json.loads(s) # parse JSON
            except:
                raise Exception("Failed to parse message as JSON!")
            try:
                d = j['data'] # grab data component of string
                chk_b = self.checksum(d) # calculate checksum
            except:
                raise Exception("No sensor values found in message!")
            try:
                chk_a = int(j['chksum']) # parse checksum
            except:
                raise Exception("No checksum value found in message!")
            if chk_a == chk_b:
                self.data.update(d)
        elif self.debug is True:
            d_tsc = {
                "slip" : random.randint(0, 100),
                "cvt" : random.randint(0, 100),
                "rpm" : random.randint(0, 3600),
                "gear" : random.randint(-1, 3),
                "cvt_temp" : random.randint(0, 999)
            }
            d_esc = {
                "throttle" : random.randint(0, 100),
                "lbrake" : random.randint(0, 100),
                "rbrake" : random.randint(0, 100),
                "bat" : random.randint(0, 24),
                "user" : random.randint(0, 255),
                "lock" : random.randint(0, 1),
                "engine_temp" : random.randint(0, 999),
                "oil" : random.randint(0, 100),
                "load" : random.randint(0, 100),
            }
            d_vdc = {
                "susp" : random.randint(0, 100),
                "ballast" : random.randint(0, 1)
            }
            d_tps = {
                "vel" : random.randint(0, 50),
                "hours" : random.randint(0, 100),
            }
            dummy_data = [d_tsc, d_esc, d_vdc, d_tps]
            self.data.update(dummy_data[random.randint(0,len(dummy_data)-1)])
        else:
            pass
        return self.data

    def checksum(self, d, mod=256, decimals=2):
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


# Web Interface
class App:

    def __init__(self, config_file):
        try:
            with open(config_file) as cfg:
                self.config = json.loads(cfg.read())
            self.mongo_client = MongoClient()
            self.db = self.mongo_client[self.config['MONGO_NAME']]
            self.session_key = binascii.b2a_hex(os.urandom(self.config['MONGO_KEY_LENGTH']))
            self.session = self.db[self.session_key]
            self.obd = OBD()
            self.log = open(os.path.join("logs", self.session_key + ".csv"), 'w')
            d = self.obd.query()
            self.log.write(','.join([str(k) for k in d.iterkeys()] + ['\n']))
            try:
                self.cvme = CLORB()
            except Exception as e:
                print str(e)

            # Scheduled Tasks
            try:
                Monitor(cherrypy.engine, self.listen, frequency=self.config["POLLING_FREQ"]).subscribe()
            except Exception as e:
                raise e
        except Exception as e:
            print str(e)

    ## Listen
    def listen(self):
        try:
            d = self.obd.query()
            self.log.write(','.join([str(v) for v in d.itervalues()] + ['\n']))
        except Exception as e:
            print str(e)

    ## Render Webapp
    @cherrypy.expose 
    def index(self, www_dir="www", index_file="index.html"):
	path = os.path.join(www_dir, index_file)
        with open(path) as html: 
            return html.read()
	
    ## Handle API Requests
    @cherrypy.expose
    def default(self, *args, **kwargs):
        """ This function the API """
        try:
            print kwargs
            return json.dumps(self.session.find().limit(1).sort({"$natural":-1}))
        except Exception as e:
            raise e
        return None

if __name__ == '__main__':
    config_file = "settings.json"
    app = App(config_file)
    cherrypy.server.socket_host = "0.0.0.0"
    cherrypy.server.socket_port = 8080
    currdir = os.path.dirname(os.path.abspath(__file__))
    conf = {
        '/': {'tools.staticdir.on':True, 'tools.staticdir.dir':os.path.join(currdir,'www')},
    }
    cherrypy.quickstart(app, '/', config=conf)
