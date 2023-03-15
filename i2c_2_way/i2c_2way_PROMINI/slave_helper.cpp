#include <arduino.h>
#include <Wire.h>
#include "slave_helper.h"

//______________________________
//char key[] = {'K','E','Y','1','2','3','\0'};
char key[] = "KEY123";
#define key_len 7

#define rx_timeout 500 //in milli sec

/*
 * i2c Addressing will be as follows:
 * Master: 8
 * Slaves: 9 to 119, depending on the number of slaves connected
 */
 
#define I2C_MASTER_ADDR 8
#define I2C_MY_ADDR 9
//_________________________________

int master_response,auth_result;

//static double data2master[8];

void default_sensor_values(void)
{
    for (int i=0; i<8; i++)
    {
      sensor_values[i] = 0.0;
      }
  }

void begin_i2c(void)
{
  Wire.begin(I2C_MY_ADDR); // join i2c bus
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
  
int* float2bytes(double sensor_val)
{
    union floatToBytes {
  
    char _buffer[4];
    float voltageReading;
  
  } converter;

  converter.voltageReading = (float)sensor_val;

  static int f2b[4];

  f2b[0] = converter._buffer[0];
  f2b[1] = converter._buffer[1];
  f2b[2] = converter._buffer[2];
  f2b[3] = converter._buffer[3];

  /*
  Serial.println(f2b[0]);
  Serial.println(f2b[1]);
  Serial.println(f2b[2]);
  Serial.println(f2b[3]);
  */
  
  return f2b;  
  }
    
double* get_latest_values(void)
{
  static double sen_vals[8];

  sen_vals[0] = (double)updated_values[0]/4.0;

  for (int i=1; i<(int)sen_vals[0]+1; i++)
  {
       sen_vals[i] = byte2float(updated_values[4*(i-1)+ 1],updated_values[4*(i-1)+ 2],updated_values[4*(i-1)+ 3],updated_values[4*(i-1)+ 4]);
    }

    return sen_vals;
  }

void i2c_send(int master_addr, int* _data, int data_len)
{
  
   Wire.beginTransmission(master_addr);
  /*
   Serial.print("sending Following data: ");
   for(int i=0; i<data_len; i++)
   {
      Wire.write(_data[i]);
      Serial.print(_data[i]);
      Serial.print(" ");
    }

    Serial.println("");
   */
   int error = Wire.endTransmission();

   if(error == 0)
   {
      Serial.println("Response sent successfully to master ....");
    }

   else if(error == 1)
   {
      Serial.println("Bufferoverloaded, please restrict the data length to less than 32 bytes");
    }

   else if(error == 5)
   {
      Serial.println("Timeout, No response from SLave");
   }

   else
   {
      Serial.println("Error in communication. Check connection or Master device");
    }
  }

int* handle_data_request(double* data2master)
{
  if(Wire.read() == 0x06)
  {
    Serial.println("Master Data request completed!");
    }

    int no_of_values = (int)data2master[0];
   
   static int s2m[32];

   s2m[0] = 0x01;
   s2m[1] = 0x05;
   s2m[2] = 4*no_of_values;

   //Serial.println("Following data is sent to master: ");
   for (int i=1; i<(no_of_values+1); i++)
   {
    int* _float2byte = float2bytes(data2master[i]);
    
    for(int j=0; j<4; j++)
    {
      s2m[3+(4*(i-1))+j] = _float2byte[j];
      //Serial.print(_float2byte[j]);
      //Serial.print(" ");
    }
    //Serial.println("");
   }
   
   return s2m;
   }
  
void handle_data_sending(void)
{
  int noofvals = Wire.read();
  updated_values[0] = noofvals;
  //Serial.println("Following data has been received");
  for (int i=1; i<(int)(noofvals+1); i++)
  {
    int temp_val = Wire.read();
    /*
    if(temp_val >127)
    {
      temp_val = 256 - temp_val;
      temp_val *= -1;
      }
      */
    updated_values[i] = temp_val;
    //Serial.print(updated_values[i]);
    //Serial.print(" ");
    }
    //Serial.println("");
    //if req respond to master
  }
  
void handle_auth(void)
{
  
  int temp_key_len = Wire.read();
  Serial.print("Length of key received = ");
  Serial.println(temp_key_len);
  char temp_key[temp_key_len];

  Serial.print("Following Characters received: ");
  for(int i=0; i<temp_key_len; i++)
  {
    temp_key[i] = Wire.read();
    Serial.print(temp_key[i]);
    }
  Serial.println("");
  int result = strcmp(temp_key,key);

  if(result == 0)
  {
    Serial.println("Key matched! Responding to master..... ");
    }
  else
  {
    Serial.println("Key did not match! Please check the key[] and key_len variable values in master and slave");
    }
    
  master_response = 1;
  auth_result = result;
  }

void respond_to_master(double* data2master)
{
  if (master_response == 1) //response for authentication
  { 
    static int auth_response[3];

    auth_response[0] = 0x01;
    auth_response[1] = 0x03;
    auth_response[2] = !auth_result;

    i2c_send(I2C_MASTER_ADDR,auth_response,3);
    master_response = 0;
    return;
   }
  }
