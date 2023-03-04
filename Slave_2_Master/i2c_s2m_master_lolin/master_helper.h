#ifndef _MASTER_HELPER_

#define _MASTER_HELPER_

#include <Arduino.h>
#include <Wire.h>

double* req_data_from_slave(int slave_no);
void auth_setup(void);
bool handle_auth(void);
bool i2c_send(int slave_no, int* _data, int data_len);
int* float2bytes(double sensor_val);
double byte2float(int b1,int b2,int b3,int b4);
bool auth_done_(int slave_index);
bool authentication(int slave_index);
void i2c_setup(void);
void default_slave_values(void);

#endif // _MASTER_HELPER_
