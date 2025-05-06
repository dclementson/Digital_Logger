Digital Data Logger for long-term monitoring of a DUT. 

An external 74HC374 octal flip flop samples eight opto-isolated inputs and applies the data to the Arduino Nano ESP32's GPIO D2..D9,  The main loop clocks the flip flops whereupon the ESP32 performs an 8-channel GPIO read using register access.  The input data are masked and shifted to form an input byte.  The byte is compared to the byte capured on the previous loop iteration.  If there has been a change in the data pattern, the data is stored in a buffer along with a timestamp consisting of the current contents of the ESP32's microsecond timer.  

On the first discovery of a data change, a timer is started and checked each loop iteration.  Data changes and their timestamps are collected until the timer expires.  When that happens, reports are generated and sent to the serial port and appended to a "log.csv" file on the attached SD card.  The reports consist of the capture number, the number of microsecond since the first capture, and a binary representation of the data byte.

Two data outputs are provided to trigger an oscilloscope.  One pulses the first time a data change is recognized.  The other pulses each time a subsequent changes is logged.

The sampling interval is about 2 microseconds.  The report timeout is currently 250 ms.

The eight opto-isolator LED currents are limited by resistors in a 16-pin network.  The value of these resostors should be chosen based on the voltages to be monitored.  LED current should be about 10mA.
