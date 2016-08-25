import os
import logging
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', filename='/home/pi/powerlogger.log', level=logging.DEBUG)
import datetime
import pymysql.cursors
import time
from time import mktime
import threading
import sys
import math
import signal
import serial
import json
import glob

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


def logPowerLineDB(type, location, powereading, averagecount):    
    try:
        connection = pymysql.connect(host='localhost', user='monitor', passwd='password', db='temps', charset='utf8mb4', cursorclass=pymysql.cursors.DictCursor)
        with connection.cursor() as cursor:
            sql = "INSERT INTO powerdat values(NOW(), '{}', '{}', '{}', '{}')".format(location, type, powereading, averagecount)
            print(sql)
            cursor.execute (sql)
        connection.commit()
        logging.debug("logTempLineDB() Rows logged: %s" % cursor.rowcount)
        connection.close()
    except:
        logging.error("logTempLineDB Temperature Logging exception Error ", exc_info=True)


def serial_ports():
    """ Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result




class PowerPoller(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.current_value = None
        self.running = True #setting the thread running to true
        

 
    def run(self):
        global gpsd
        global gpsp
                              
        ser = serial.Serial(serialPort, 115200, bytesize=8, parity='N', stopbits=1, timeout=1, rtscts=False, dsrdtr=False)
        #ser.setRTS(0) 
        time.sleep(1) # creating connection will reset arduino, need to wait for reset complete. 
        while gpsp.running:      
            try:
                time.sleep(0.1)
                ser.flushInput()
                
                ser.write(b'get_power;')       
                time.sleep(0.5)
                                                
                bytesresult = ser.readline()
                #print(bytesresult)
                power = json.loads(bytesresult.decode("utf-8"))
                #print(power)
            
                logPowerLineDB("phase0", currentLocation, power["power0"]["power"], power["power0"]["averagecount"])
                logPowerLineDB("phase1", currentLocation, power["power1"]["power"], power["power1"]["averagecount"])
    
                
                
            except:
                
                print(bytesresult)
                logging.error("Exception in main logging loop ", exc_info=True)
            
            time.sleep(loginterval)
            
        ser.close()
        print("ended")
 
 
 
 
 
 
if __name__ == "__main__":

    # Create logger and set options
    logging.getLogger().setLevel(logging.INFO)
    logging.info("Logging started")
    signal.signal(signal.SIGINT, signal_quitting)



    availableports = serial_ports()
    
    print(availableports)
    
    for port in availableports:
        try:
            ser = serial.Serial(port, 115200, bytesize=8, parity='N', stopbits=1, timeout=1, rtscts=False, dsrdtr=False)
            time.sleep(0.5)
            ser.flushInput()               
            ser.write(b'whatis;')       
            time.sleep(0.5)
            result = ser.readline()
            print("Testing port {}".format(port))
            if(result.find(b'power') >= 0):
                serialPort = port
                print("Found power logger on serial port {}".format(serialPort))
                ser.close()
                break
            
            ser.close()
        except:
            raise
        


#    try:        
        #Start Gas polling thread
    print("Starting logging")
    gpsp = PowerPoller()
    gpsp.start()
  
    while True: time.sleep(100)
#    except:
#        raise    
