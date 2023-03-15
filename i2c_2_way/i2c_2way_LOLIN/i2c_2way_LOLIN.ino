#include <Arduino.h>
#include <Wire.h>

#include "master_helper.h"

//IMPORTANT:
//Please check the master_helper.cpp file to change the user defined variables for the system

double* read_sensors(void);

double* data_to_slave;

int slave_addr;

bool data_req_sent_;
float data_req_frequency = 0.167; //Number of times in a second to request sensor data from slave
/*
 * 4.0--> 4 times in a second
 * 2.0--> 2 times a second
 * 1.0--> 1 time every second
 * 0.5-->1 times in every 2 seconds
 * 0.25-->1 time in every 4 seconds
 */
hw_timer_t *timer = NULL;

double* val_from_slave;

void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200);           // start serial for output
    
    i2c_setup(); // join i2c bus for communication with other slaves
    auth_setup();
    data_req_setup(data_req_frequency);

    bool data_req_sent_ = 0;
    
    Serial.println("Begining the process!..........");
    Serial.flush();
}

void loop() {
  // put your main code here, to run repeatedly:

  data_to_slave = read_sensors(); 
  slave_addr = 9; //for sending data the slave adress to be used 

  if(Serial.available())
   {
     int temp = Serial.read();
     Serial.println("Master----->SLAVE");
     master_2_slave();
   }

   if(data_req_sent_ == 1)
  {
    Serial.println("SLAVE----->Master");
    slave_2_master();
    data_req_sent_ = 0;
    }

}

double* read_sensors(void) //edit this function for getting sensor values in master
{
  int no_of_values = 4;// define the number of sensor values you are sending
  //NOTE: Only upto 7 sensor values at the most can be sent at a time due to buffer size limitations
  
  static double data_2_slave[8];

  //add the sensor reading code here

  //replace these values actual values
  data_2_slave[0] = (double)no_of_values; // the number of sensor readings to be sent to the i2c slave
  data_2_slave[1] = 0.35;// start from here
  data_2_slave[2] = 0.46;
  data_2_slave[3] = -64.4;
  data_2_slave[4] = -83.09;

  return data_2_slave;
}

void master_2_slave(void)
{
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

void slave_2_master(void)
{
  Serial.println("Requesting data....");
  val_from_slave = req_data_from_slave(9);
  if(val_from_slave[0] == -1.0){return;}
  Serial.print("Following ");
  Serial.print((int)val_from_slave[0]);
  Serial.println(" Sensor values have been received from slave: ");
  for(int i=1; i<(int)(val_from_slave[0]+1); i++)
  {
    Serial.print("Val");
    Serial.print(i);
    Serial.print(" = ");
    Serial.println(val_from_slave[i]);
   }
}

void IRAM_ATTR onTimer(){
  data_req_sent_ = 1;
}

void data_req_setup(float freq)
{
  uint64_t alarm_value = (uint64_t)(1000000.0/freq); //in micro secs
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, alarm_value, true);
  timerAlarmEnable(timer); //Just Enable
}
