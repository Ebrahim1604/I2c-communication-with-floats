#include <Arduino.h>
#include <Wire.h>

//void data_req_setup(int freq);

#include "master_helper.h"

//bool data_req_sent_;

void setup() {
  // put your setup code here, to run once:

    Serial.begin(115200);           // start serial for output
    
    i2c_setup(); //join i2c bus for communication with other slaves

    auth_setup();

    //data_req_setup(data_req_frequency);

    bool data_req_sent_ = 0;

    Serial.println("Begining the process!..........");
    Serial.flush();
}

void loop() {
  // put your main code here, to run repeatedly:

  if(Serial.available())
   {
    int ser = Serial.read();

        Serial.println("Requesting data....");
        double* val_from_slave = req_data_from_slave(9);
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
}
