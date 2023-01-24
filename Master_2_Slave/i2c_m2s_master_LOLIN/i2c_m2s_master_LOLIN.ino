#include <Arduino.h>
#include <Wire.h>

#include "master_helper.h"

//IMPORTANT:
//Please check the master_helper.cpp file to change the user defined variables for the system

double* read_sensors(void);

void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200);           // start serial for output
    
    i2c_setup(); // join i2c bus for communication with other slaves

    auth_setup();
    
    Serial.println("Begining the process!..........");
    Serial.flush();
}

void loop() {
  // put your main code here, to run repeatedly:

   double* data_to_slave = read_sensors();
   int slave_addr = 9; //for sending data the slave adress to be used 

   if(Serial.available())
   {
         int temp = Serial.read();
         
         Serial.println("Sending Data to Slave.....");
         Serial.print("Sending following ");
         Serial.print((int)data_to_slave[0]);
         Serial.print(" Sensor values to the slave at address ");
         Serial.println(slave_addr);
         
         for(int i=1; i<(int)(data_to_slave[0]+1); i++)
      {
        Serial.print("Val");
        Serial.print(i);
        Serial.print(" = ");
        Serial.println(data_to_slave[i]);
        }
         send_data_to_slave(slave_addr,data_to_slave);
    }

 }

 double* read_sensors(void)
{
  int no_of_values = 4;// define the number of sensor values you are sending
  //NOTE: Only upto 7 sensor values at the most can be sent at a time due to buffer size limitations
  
  static double data_to_slave[8];

  //add the sensor reading code here

  //replace these values actual values
  data_to_slave[0] = (double)no_of_values; // the number of sensor readings to be sent to the i2c slave
  data_to_slave[1] = 0.35;// start from here
  data_to_slave[2] = 0.46;
  data_to_slave[3] = -64.4;
  data_to_slave[4] = -83.09;

  return data_to_slave;
  }
  
