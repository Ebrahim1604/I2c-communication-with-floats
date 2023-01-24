#include <arduino.h>
#include <Wire.h>
#include "master_helper.h"

//______________________ Change these variables as per requirement_____

#define SDA_PIN 12 //i2c pins for communication with other i2c slaves
#define SCL_PIN 13

/*
 * i2c Addressing will be as follows:
 * Master:- 8
 * Slaves: 9 to 119, depending on the number of slaves connected
 */
#define I2C_MASTER_ADDR 8 //master address

const int no_of_slaves = 1;
//include the address of the slaves connected in the array below
int slaves_list[] = {9};

char key[] = "KEY123";
int key_len = 7; // lenght of key is always number of charcters + 1

int rx_timeout = 500; //in milli sec, amount of time to wait for response

//_______________________________________________PVT FUNCTIONS__________

bool auth_done[no_of_slaves];

void i2c_setup(void)
{
   Wire.begin(SDA_PIN, SCL_PIN);
  }

void send_data_to_slave(int slave_no, double* data_to_slave)
{
  int slave_index;
  int no_of_values = (int)data_to_slave[0];
  
  for (int i=0; i<no_of_slaves; i++)
  {
    if(slaves_list[i] == slave_no)
    {
      slave_index = i;
      Serial.println("Slave defined in i2c bus, CHECking for authentication....");
      goto auth;
      }
    }
     Serial.print("The requested slave address ");
     Serial.print(slave_no);
     Serial.print(" is not defined in the i2c bus");
     Serial.println("Please update the slaves_list array and no_of_slaves variable for the same");
     Serial.println("");
     return;
  
  auth:
  bool authenticated = auth_done_(slave_index);

  if (authenticated == 1)
  {
    Serial.print("Slave at address ");
    Serial.print(slave_no);
    Serial.print(" authenticated with key: ");
    Serial.print(key);
    Serial.println(" ,Sending the sensor data now....");
    //delay(2000);
    }
  else if(authenticated == 0)
  {
    Serial.print("Slave at address ");
    Serial.print(slave_no);
    Serial.print(" could not be authenticated with key: ");
    Serial.print(key);
    Serial.println(" ,Please use the correct key and try again...");
    return;
    }

   //static int data2slave[(4*no_of_values)+3];
   static int data2slave[32];
   data2slave[0] = 0x02;
   data2slave[1] = 0x04;
   data2slave[2] = 4*no_of_values;

   for (int i=1; i<no_of_values+1; i++)
   {
    int* _float2byte = float2bytes(data_to_slave[i]);
    
    for(int j=0; j<4; j++)
    {
      data2slave[3+(4*(i-1))+j] = _float2byte[j];
    }
   }

 bool data_sent_ = i2c_send(slave_no,data2slave,(4*no_of_values)+3);

  if(data_sent_ == 0)
  {
    Serial.println("Data sending Failed from master to slave!");
    }
  else
   {
      Serial.println("Data sent successfully to slave");
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

void auth_setup(void)
{
  for (int i=0; i<no_of_slaves; i++)
  {
    auth_done[i] = 0;
    }
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
