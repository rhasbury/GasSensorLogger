# GasSensorLogger

An Arduino/Raspberry Pi application pair that can be used to log the voltage levels from gas sensors (or any voltage that you can meansure with an arduino Ain) to a mysql database. 

The Arduino sketch simply waits on the serial port for a text string containing get_aX, to which it replies with the integer value read from analog input number X. 

The Raspberry pi applicaion, written in python, polls the arduino through the serial interface and takes each of the Ain numbers and writes them to a table in a mysql database. 

# PowerLogger

An Arduino/Raspberry Pi application pair that makes use of current sensors and the library at https://github.com/openenergymonitor/EmonLib to measure power usage. Power usage in kwh is logged every 10 mins to a mysql. A value that's updated every ~5 seconds is also available via a socket connection for (realatively)live power monitoring. 

Harware setup on the arduino is as described here https://openenergymonitor.org/emon/buildingblocks/ct-sensors-interface
