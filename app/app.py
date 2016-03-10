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

class App:
    def __init__(self, config_file):
        try:
            with open(config_file) as cfg:
                self.config = json.loads(cfg.read())
            self.init_tasks()
            self.init_db()
            self.init_obd()
        except Exception as e:
            raise(e)

    ## Listen
    def listen(self):
        try:
            s = self.obd.readline()
            j = json.loads(s)
            d = j['data'] # grab data component of string
            n = j['chksum'] # parse checksum
            c = self.checksum(d) # calculate checksum
            print s
            print c, n
        except Exception as e:
            print str(e)
        
    def checksum(self, d, mod=256, decimals=2):
        chksum = 0
        d = {k.encode('ascii'): v for (k, v) in d.iteritems()}
        for k,v in d.iteritems():
            if v is float:
                d[k] = round(v,decimals)
        s = str(d)
        r = s.replace(' ', '').replace('\'', '\"')
        print r
        for i in r:
            chksum += ord(i)
        return chksum % mod
    
    ## Initialize OBD
    def init_obd(self, device_classes=['/dev/ttyUSB','/dev/ttyACM'], attempts=5, baud=9600):
        self.obd = None
        for i in range(attempts):
            for dev in device_classes:
                try:
                    dev_id = dev + str(i)
                    self.obd = serial.Serial(dev_id, baud)
                    break
                except Exception as e:
                    print str(e)
            if self.obd is not None: break
        print dev_id
            
    ## Initialize Tasks
    def init_tasks(self, listen_interval=0.001):
        try:
            Monitor(cherrypy.engine, self.listen, frequency=listen_interval).subscribe()
        except Exception as e:
            raise e
            
    ## Initialize MongoDB
    def init_db(self, addr='127.0.0.1', port=27019):
        try:
            self.mongo_client = MongoClient()
            print self.mongo_client
        except Exception as e:
            raise e

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
        except Exception as e:
            raise e
        return None

if __name__ == '__main__':
    config_file = "settings.json"
    template = "packet.tmpl"
    app = App(config_file)
    cherrypy.server.socket_host = "0.0.0.0"
    cherrypy.server.socket_port = 8080
    currdir = os.path.dirname(os.path.abspath(__file__))
    conf = {
        '/': {'tools.staticdir.on':True, 'tools.staticdir.dir':os.path.join(currdir,'www')},
    }
    cherrypy.quickstart(app, '/', config=conf)
