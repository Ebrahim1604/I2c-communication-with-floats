#include <Wire.h>
#include <string.h>
#include "slave_helper.h"
/*

/* Note:
 * sen_values is the array which stores the number of variables
 * sen_values[0] is always the number of sensor values sent by 
 * the master. sensor values are stored from index 1 to 7, depending
 * on the number of values sent.
 */

double* sen_values;
static double datatomaster[8];
double* update_datatomaster(void);

int* send_to_master;

int incoming_byte = 0;
int request_from_master = 0;

bool sensor_val_updated_ = 0;
bool data_req_rcvd = 0;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);        // start serial for output
  
  default_sensor_values();
  
  begin_i2c();
  
  Wire.onReceive(receiveEvent); // register receive event
  Wire.onRequest(requestEvent); // register request event

  Serial.println("Begining the slave process...!");
}

void loop() {
  // put your main code here, to run repeatedly:
  
  if(sensor_val_updated_ == 1)
    {
      //noInterrupts();
      sen_values = update_datatomaster();
      Serial.print("Following ");
      Serial.print((int)sen_values[0]);
      Serial.println(" Sensor values are being sent to master:");
      
      for(int i=1; i<(int)(sen_values[0]+1); i++)
      {
        Serial.print("Val");
        Serial.print(i);
        Serial.print(" = ");
        Serial.println(sen_values[i]);
        }
      sensor_val_updated_ = 0;
      }
  //sen_values[1],sen_values[2]... can be used in your code further

  if (incoming_byte == 1)
  {
    Serial.println("Incoming Values from i2c Bus, responding to the same...");

    if (Wire.read() == 0x02)
        {
          int second_byte = Wire.read();

          if(second_byte == 0x03)
            {
              Serial.println("Request for Authentication received....");
              handle_auth();
              }
           else if(second_byte == 0x04)
            {
              Serial.println("Receiving data from master....");
              handle_data_sending();
              sensor_val_updated_ = 1;
              }
           else if(second_byte == 0x05)
           {
              Serial.println("Request for Data received....");
              data_req_rcvd = 1;
              sen_values = update_datatomaster();
              Serial.print("Following ");
              Serial.print((int)sen_values[0]);
              Serial.println(" Sensor values will be sent to master at address 8: ");
              
              for(int i=1; i<(int)(sen_values[0]+1); i++)
              {
                Serial.print("Val");
                Serial.print(i);
                Serial.print(" = ");
                Serial.println(sen_values[i]);
                }
              send_to_master = handle_data_request(datatomaster);
            }
          }
    incoming_byte = 0;
    }

   if(request_from_master == 1)
   {
      Serial.println("MAster requesting authentication response....");
      respond_to_master(datatomaster);
      request_from_master = 0;
      incoming_byte = 0;
      return;
    }
}

double* update_datatomaster(void)
{
  int no_of_values = 4;// define the number of sensor values you are sending

    //replace these values actual values
    //Note: The size of this array is 8 which is already defined globally
  datatomaster[0] = (double)no_of_values; // the number of sensor readings to be sent to the i2c slave
  datatomaster[1] = -30.64;
  datatomaster[2] = 200.04;
  datatomaster[3] = 150.19;
  datatomaster[4] = -1.02;

  return datatomaster;
  }

void receiveEvent(int numBytesReceived)
{
   if(Wire.available())
     {  
        incoming_byte = 1;
        return;
      }
  }

void requestEvent()
{
  if (data_req_rcvd == 1)
  {
    //Wire.beginTransmission(8);

   //Serial.println("sending Following data: ");
   for(int i=0; i<32; i++)
   {
      Wire.write(send_to_master[i]);
      //Serial.print(send_to_master[i]);
      //Serial.print(" ");
    }

    //Serial.println("");
   
    //int error = Wire.endTransmission(0);
    data_req_rcvd = 0;
    return;
    }
    
  else
  {
  request_from_master = 1;
  return;
  }
}
