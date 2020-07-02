import os
import logging
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', filename='./gassensor.log', level=logging.DEBUG)
import datetime
import pymysql.cursors
import time
from time import mktime
import threading
import sys
import math
import signal
import serial
import glob
import json

currentLocation = 'basement'
loginterval = 300 # in seconds
errordelay = 3000 # in seconds

serialPort = None
baud = 115200

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
        connection = pymysql.connect(host='192.168.1.104', user='monitor', passwd='password', db='temps', charset='utf8mb4', cursorclass=pymysql.cursors.DictCursor)
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
                              
        ser = serial.Serial(serialPort, baud, bytesize=8, parity='N', stopbits=1, timeout=1, rtscts=False, dsrdtr=False)
        #ser.setRTS(0) 
        time.sleep(3) # creating connection will reset arduino, need to wait for reset complete. 
        while gpsp.running:      
            try:
                time.sleep(0.1)
                ser.flushInput()
                
                ser.write(b'get_all_avg_json;')       
                time.sleep(0.8)
                data = ser.readline()            
                logging.info(data)
                gases = json.loads(data.decode("utf-8"))

                #print("a0: {}  a1: {}  a2:  {} dust {}".format(int(a0), int(a1), int(a2), int(dust)))
                    
                logGaslineDB("A0_Avg", currentLocation, gases["a0_avg"])
                logGaslineDB("A1_Avg", currentLocation, gases["a1_avg"])
                logGaslineDB("A2_Avg", currentLocation, gases["a2_avg"])
                logGaslineDB("TVOC", currentLocation, gases["tvoc_avg"])
                logGaslineDB("CO2", currentLocation, gases["tvoc_avg"])
            
                
                time.sleep(loginterval)
            except (OSError, serial.SerialException):
                logging.error("Serial Exception in main logging loop ", exc_info=True)
                time.sleep(errordelay)
            except:
                logging.error("Some other Exception in main logging loop ", exc_info=True)
                time.sleep(errordelay)
        
        ser.close()
        print("ended")


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
 
if __name__ == "__main__":
    # Create logger and set options
    logging.getLogger().setLevel(logging.INFO)
    logging.info("Logging started")
    signal.signal(signal.SIGINT, signal_quitting)


    availableports = serial_ports()
    
    print(availableports)
    
    for port in availableports:
        try:
            ser = serial.Serial(port, baud, bytesize=8, parity='N', stopbits=1, timeout=1, rtscts=False, dsrdtr=False)
            time.sleep(4)
            ser.flushInput()               
            ser.write(b'get_a0;')       
            time.sleep(0.5)
            result = ser.readline()
            
            print("Testing port {}".format(port))
            if(int(result) >= 0):
                serialPort = port
                print("Found gas logger on serial port {}".format(serialPort))
                ser.close()
                break
            
            ser.close()
        except ValueError:
            ser.close()
            pass
        except:
            raise
    if(serialPort == None):
        logging.error("Gas logger device not found")
        exit()

#    try:        
        #Start Gas polling thread
    gpsp = GasPoller()
    gpsp.start()
  
    while True: time.sleep(100)
#    except:
#        raise    
