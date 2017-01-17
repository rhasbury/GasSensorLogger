import os
import logging
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', filename='./powerlogger.log', level=logging.DEBUG)
import datetime

import socketserver
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

HOST, PORT = "", 50008

currentLocation = 'basement'
loginterval = 300 # how often we log to the database in seconds
liveinterval = 3  # how often we pull live data from the monitor

# actual voltages measured with a dmm 
onetwentyvoltage = 113
twofortyvoltage = 236



serialPort = None

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
        connection = pymysql.connect(host='192.168.0.105', user='monitor', passwd='password', db='temps', charset='utf8mb4', cursorclass=pymysql.cursors.DictCursor)
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
        self.twofortywatts = 0
        self.onetwentywatts = 0
        self.clamp1watts = 0 
        self.clamp2watts = 0
 
    def run(self):
        global gpsd
        global gpsp
                              
        ser = serial.Serial(serialPort, 115200, bytesize=8, parity='N', stopbits=1, timeout=1, rtscts=False, dsrdtr=False)
        #ser.setRTS(0) 
        hoursoflogging = 0     
        twofortyload = 0
        onetwentyload = 0 
        clamp1load = 0   
        clamp2load = 0
        lastlog = datetime.datetime.utcnow()
		
        time.sleep(1) # creating connection will reset arduino, need to wait for reset complete. 
        while gpsp.running:      
            try:
                time.sleep(0.1)
                ser.flushInput()                
                ser.write(b'get_power;')       
                time.sleep(2)                                                
                bytesresult = ser.readline()                
                power = json.loads(bytesresult.decode("utf-8"))


                
                # These two lines separate the 120 volt currents from the 230 volt currents
                onetwentyamps = abs(float(power["power3"]["current"] - float(power["power4"]["current"])))                
                twofortyamps = max(float(power["power3"]["current"]),float(power["power4"]["current"])) - onetwentyamps
                
                self.twofortywatts = twofortyamps * twofortyvoltage
                self.onetwentywatts = onetwentyamps * onetwentyvoltage
                self.clamp1watts = float(power["power1"]["current"]) * onetwentyvoltage
                self.clamp2watts = float(power["power2"]["current"]) * onetwentyvoltage
                
                # accumulates time measured between database logs. 
                hoursoflogging = hoursoflogging + (power["power1"]["averagecount"] * 1.5)/(60*60)  # Each arduino average is 1.5 seconds. This holds "hours"
                
                # these accumulate the measured amount of KwH between each logging cycle. 
                twofortyload  = twofortyload  + ((twofortyamps * twofortyvoltage) * hoursoflogging )
                onetwentyload = onetwentyload + ((onetwentyamps * onetwentyvoltage) * hoursoflogging ) 
                clamp1load = clamp1load + ((float(power["power1"]["current"])* onetwentyvoltage) * hoursoflogging)   
                clamp2load = clamp2load + ((float(power["power2"]["current"])* onetwentyvoltage) * hoursoflogging)
            
            
                if(datetime.datetime.utcnow() - lastlog > datetime.timedelta(seconds=loginterval)):
                        
                    logPowerLineDB("Clamp1", currentLocation, (power["power1"]["power"]*hoursoflogging), power["power1"]["averagecount"])
                    logPowerLineDB("Clamp2", currentLocation, (power["power2"]["power"]*hoursoflogging), power["power2"]["averagecount"])                   
                    logPowerLineDB("240v Total", currentLocation, twofortyload, power["power4"]["averagecount"])
                    logPowerLineDB("120v Total", currentLocation, onetwentyload, power["power4"]["averagecount"])
                    hoursoflogging = 0     
                    twofortyload = 0
                    onetwentyload = 0 
                    clamp1load = 0   
                    clamp2load = 0 
                    
            except:                
                print(bytesresult)
                logging.error("Exception in main logging loop ", exc_info=True)
            
            
            
            
            time.sleep(liveinterval)
            
        ser.close()
        print("ended")
    
    def to_JSON(self):
        return json.dumps({"twofortywatts" : self.twofortywatts, "onetwentywatts" : self.onetwentywatts, "clamp1watts" : self.clamp1watts, "clamp2watts" : self.clamp2watts  })
 
 

class MyTCPHandler(socketserver.BaseRequestHandler):
    
    #def obj_dict(obj):
    #    return obj.__dict__

   #The RequestHandler class for data requests from the web interface or any remote applications.   

    def handle(self):        
        date_handler = lambda obj: (
            obj.isoformat()
            if isinstance(obj, datetime.datetime)
            or isinstance(obj, datetime.date)
            else obj.__dict__            
        )
        # self.request is the TCP socket connected to the client
        self.data = self.request.recv(1024).strip()        
        
        if("get_powers" in self.data.decode("utf-8")):
            self.request.sendall(bytes(gpsp.to_JSON(), 'UTF-8'))
        elif("get_something" in self.data.decode("utf-8")):
            self.request.sendall(bytes("whattt?", 'UTF-8'))



 
 
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
            time.sleep(2) # for Arduino Nano this needs to be 2 seconds, for Arduino micro it can be almost zero. (nano resets on serial connection) 
            ser.flushInput()               
            ser.write(b'whatis;')       
            time.sleep(2)
            result = ser.readline()
            print("Testing port {}".format(port))            
            if(result.find(b'power') >= 0):
                serialPort = port
                print("Found power logger on serial port {}".format(serialPort))
                ser.close()
                break
            
            print("closind read result {}".format(result))
            ser.close()
        except:
            raise
        

    if(serialPort == None):
        print("No monitor found. Exiting")
        exit()
        
#    try:        
        #Start Gas polling thread
    print("Starting logging")
    gpsp = PowerPoller()
    gpsp.start()
    
    
    # Create the data server and assigning the request handler        
    server = socketserver.TCPServer((HOST, PORT), MyTCPHandler)
    serverthread = threading.Thread(target=server.serve_forever)
    serverthread.daemon = True
    serverthread.start()
    #my_logger.info("Data sockect listner started")
    print("Data sockect listner started")
  
    while True: time.sleep(100)
#    except:
#        raise    
