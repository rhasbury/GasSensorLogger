import os
import logging
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', filename='/home/pi/GasSensorLogger/Rpi/gassensor.log', level=logging.DEBUG)
import datetime
import pymysql.cursors
import time
from time import mktime
import threading
import sys
import math
import signal
import serial

currentLocation = 'basement'
loginterval = 300 # in seconds

serialPort = '/dev/ttyUSB0'

gpsd = None #seting the global variable
gpsp = None #


def signal_quitting(signal, frame):
    global gpsp
    logging.info("Received Sigint, killing threads and waiting for join. ")    
    gpsp.running = False        
    gpsp.join(2) # wait for the thread to finish what it's doing
    sys.exit(0)    


def logGaslineDB(type, location, gasreading):    
    try:
        connection = pymysql.connect(host='localhost', user='monitor', passwd='password', db='temps', charset='utf8mb4', cursorclass=pymysql.cursors.DictCursor)
        with connection.cursor() as cursor:
            cursor.execute ("INSERT INTO gasdat values(NOW(), %s, %s, %s)", (location, type, gasreading))
        connection.commit()
        logging.debug("logTempLineDB() Rows logged: %s" % cursor.rowcount)
        connection.close()
    except:
        logging.error("logTempLineDB Temperature Logging exception Error ", exc_info=True)




class GasPoller(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.current_value = None
        self.running = True #setting the thread running to true
        

 
    def run(self):
        global gpsd
        global gpsp
                              
        ser = serial.Serial(serialPort, 9600, bytesize=8, parity='N', stopbits=1, timeout=1, rtscts=False, dsrdtr=False)
        #ser.setRTS(0) 
        time.sleep(3) # creating connection will reset arduino, need to wait for reset complete. 
        while gpsp.running:      
            try:
                time.sleep(0.1)
                ser.flushInput()
                
                ser.write(b'get_a0;')       
                time.sleep(0.1)
                a0 = ser.readline()            
                ser.write(b'get_a1;')
                time.sleep(0.1)
                a1 = ser.readline()
                ser.write(b'get_a2;')
                time.sleep(0.1)
                a2 = ser.readline()
                
                print("a0: {}  a1: {}  a2:  {}".format(int(a0), int(a1), int(a2)))
            
                logGaslineDB("MQ-135", currentLocation, int(a0))            
                logGaslineDB("MQ-5", currentLocation, int(a1))
                logGaslineDB("MQ-9", currentLocation, int(a2))
            
                
                time.sleep(loginterval)
            except:
                 logging.error("Exception in main logging loop ", exc_info=True)
            
        
        ser.close()
        print("ended")
 
 
if __name__ == "__main__":

    # Create logger and set options
    logging.getLogger().setLevel(logging.INFO)
    logging.info("Logging started")
    signal.signal(signal.SIGINT, signal_quitting)


#    try:        
        #Start Gas polling thread
    gpsp = GasPoller()
    gpsp.start()
  
    while True: time.sleep(100)
#    except:
#        raise    
