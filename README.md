# I2c-communication-with-floats

Master:
1) The master_helper.cpp file contains all the system vaiables which can be changed easily like Number of slaves, Slave list, Authentication Key etc.
2) The function read_sensors() in the main .ino file is responsible for acquiring sensor data which has to be sent to slave. 
3) The Number of sensor values being sent to the slave needs to be defined correctly with the no_of_values variables and then each sensor value can be defined from index 1 to upto index 7. 
Note: Only upto 7 sensor values can be sent at a time due to buffersize limitations
4) The function send_data_to_slave(int slave_no, double* data_to_slave) is responsible for sending the acquired sensor data to the slave.

Slave:
1) The slave_helper.cpp file contains all the system vaiables which can be changed easily changed.
2) The sen_values[0], sen_values[1]...will be the sensor values sent from master that can be used in the system.


