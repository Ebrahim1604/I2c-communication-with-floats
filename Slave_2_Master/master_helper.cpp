#include <Arduino.h>
#include <Wire.h>
#include "master_helper.h"

//______________________ Change these variables


#define SDA_PIN 12 //i2c pins for communication with other i2c slaves
#define SCL_PIN 13

#define I2C_MASTER_ADDR 8 //master address

const int no_of_slaves = 1;
//include the address of the slaves connected in the array below
int slaves_list[] = {9};

//char key[] = {'K','E','Y','1','2','3','\0'};
char key[] = "KEY123";
int key_len = 7; // lenght of key is always number of charcters + 1

int rx_timeout = 5000; //in milli sec, amount of time to wait for response

//signifies how many times to request data in a second
//int data_req_frequency = 2; 
//_____________________

//_______________________________________________PVT VARS & ARAYS__________
//bool data_req_sent_ = 0; //do not change
static double slave_values[8];
static int updated_values[32];
bool auth_done[no_of_slaves];

//_______________________________________________PVT FUNCTIONS__________


void i2c_setup(void)
{
   Wire.begin(SDA_PIN, SCL_PIN);
  }

void auth_setup(void)
{
  for (int i=0; i<no_of_slaves; i++)
  {
    auth_done[i] = 0;
    }
  }

double* req_data_from_slave(int slave_no)
{
  int slave_index;
  static double null_[1];
  null_[0] = -1.0;
  
  for (int i=0; i<no_of_slaves; i++)
  {
    if(slaves_list[i] == slave_no)
    {
      slave_index = i;
      Serial.println("Slave defined in i2c bus, checking for authentication....");
      //Serial.println("");
      goto auth;
      }
    }
     Serial.print("The requested slave address ");
     Serial.print(slave_no);
     Serial.print(" is not defined in the i2c bus");
     Serial.println("Please update the slaves_list array and no_of_slaves variable for the same");
     Serial.println("");
     return null_;
  
  auth:
  bool authenticated = auth_done_(slave_index);

  if (authenticated == 1)
  {
    Serial.print("Slave at address ");
    Serial.print(slave_no);
    Serial.print(" authenticated with key: ");
    Serial.print(key);
    Serial.println(" ,Sending the sensor data Request to slave now....");
    //delay(1000);
    }
  else if(authenticated == 0)
  {
    Serial.print("Slave at address ");
    Serial.print(slave_no);
    Serial.print(" could not be authenticated with key: ");
    Serial.print(key);
    Serial.println(" ,Please use the correct key and try again...");
    return null_;
    }

  
   static int data2slave[3];

   data2slave[0] = 0x02;
   data2slave[1] = 0x05;
   data2slave[2] = 0x06;

 bool data_sent_ = i2c_send(slave_no,data2slave,3);

  if(data_sent_ == 0)
  {
    Serial.println("Unable to send Data request to slave, please check connection");
    }
  else
   {
      Serial.println("Data Requested successfully from slave, awaiting data");
    }

  delay(500);
  int numofbytes = Wire.requestFrom(slave_no, 32, true);
  
  long int initial_time = millis();
   
   while(Wire.available() == 0)
          {
            if ((millis() - initial_time) > rx_timeout)
            {
              Serial.println("Slave took too long to respond. Please check the connection and the receiver handler function in slave device");
              return 0;
              }
             else
             {
               Serial.println("Waiting for Slave to respond...");
              }
            }
    default_slave_values();
  
  if (Wire.available())
  {
    int first_byte = Wire.read();
    //Serial.print("1st byte = ");
    //Serial.println(first_byte);
    
    if(first_byte == 0x01)
    {
      //Serial.println("Received something2...");
      if(Wire.read() == 0x05)
      {
        //Serial.print("Following ");
        int noofvals = Wire.read();
        //Serial.print(noofvals);
        //Serial.println(" values received from Slave:");
        updated_values[0] = noofvals;

        for (int i=1; i<noofvals+1; i++)
        {
          updated_values[i] = Wire.read();
          //Serial.print(updated_values[i]);
          //Serial.print(" ");
        } 
      }
     }
    }
    
    //Serial.println("");

  slave_values[0] = (double)updated_values[0]/4.0;

  //Serial.println("FOllowing sensors value received from slave:");
  //Serial.print(slave_values[0]);
  //Serial.print(" ");
  
  for (int i=1; i<(int)slave_values[0]+1; i++)
  {
       slave_values[i] = byte2float(updated_values[4*(i-1)+ 1],updated_values[4*(i-1)+ 2],updated_values[4*(i-1)+ 3],updated_values[4*(i-1)+ 4]);
       //Serial.print(slave_values[i]);
       //Serial.print(" ");
    }

    //Serial.println("");
    return slave_values; 
 }

bool auth_done_(int slave_index)
{
  if (auth_done[slave_index] == 1)
  {
     //Serial.println("Slave already authenticated...");
     return 1;
    }

   else
   {
      return authentication(slave_index);
    }
  }

bool authentication(int slave_index)
{
  static int auth_frame[32];

  auth_frame[0] = 0x02;
  auth_frame[1] = 0x03;
  auth_frame[2] = key_len;
  
  for(int i=0; i<key_len; i++)
  {
    auth_frame[i+3] = key[i];
    }

   Serial.println("Sending authentication request...");
   if((i2c_send(slaves_list[slave_index],auth_frame,3+key_len)) == 1)
   {
      auth_done[slave_index] = 1;
      delay(2000);
      return 1;
    }
    else
    {
      return 0;
      }
}

bool i2c_send(int slave_no, int* _data, int data_len)
{
   long int initial_time;
   bool result = 1;
   
   Wire.beginTransmission(slave_no);

   //Serial.println("Sending following Data packets: ");
   for(int i=0; i<data_len; i++)
   {
      Wire.write(_data[i]);
      //Serial.print(_data[i]);
      //Serial.print(" ");
    }

   //Serial.println("");
   int error = Wire.endTransmission();
   
   
   if(error == 0)
   {
      Serial.println("Message sent successfully, awaiting reply from Slave....");
    }

   else if(error == 1)
   {
      Serial.println("Bufferoverloaded, please restrict number of sensor values to 7 or less");
      return 0;
    }

   else if(error == 5)
   {
      Serial.println("Timeout, No response from Slave");
      return 0;
   }

   else
   {
      Serial.println("Error in communication. Check connection or Slave device");
      return 0;
    }


    if(_data[1]== 0x03)
    { 
      //Wire.end();
      Wire.requestFrom(slave_no,3,1);
      
      initial_time = millis();

      //Wire.begin(I2C_MASTER_ADDR);

          while(Wire.available() == 0)
          {
            if ((millis() - initial_time) > rx_timeout)
            {
              Serial.println("Slave took too long to respond. Please check the connection and the receiver handler function in slave device");
              return 0;
              }
             else
             {
               Serial.println("Waiting for Slave to respond...");
              }
            }
      
           while(Wire.available())
           {
              if (Wire.read() == 0x01)
              {
                int second_byte = Wire.read();
      
                if(second_byte == 0x03)
                  {
                    result = handle_auth();
                    }
                }
            }
          return result;
        }
     else
     {
        return 1;
      }
 }

void default_slave_values(void)
{ 
  for (int i=0; i<32; i++)
    {
      updated_values[i] = 0;
      }
      
  for (int i=0; i<8; i++)
    {
      slave_values[i] = 0.0;
      }
  }
  
double byte2float(int b1,int b2,int b3,int b4)
{
  union b2f {
  
    char _buffer[4];
    float voltageReading;
  
  } converter;

  converter._buffer[0] = b1;
  converter._buffer[1] = b2;
  converter._buffer[2] = b3;
  converter._buffer[3] = b4;

  return (double)converter.voltageReading;
  }

bool handle_auth()
{
  bool result = Wire.read();
  
  if(result == 0)
  {
    Serial.println("Key did not match! Please check the key[] and key_len variable values in master and slave");
    }
  else
  {
    Serial.println("Key matched successfully!!");
    }
    
  return result;
 }
