#ifndef _SLAVE_HELPER_

#define _SLAVE_HELPER_

#include <arduino.h>
void begin_i2c(void);
static double sensor_values[8];
static int updated_values[32];
//bool sensor_val_updated_ = 0;
void default_sensor_values(void);
double byte2float(int b1,int b2,int b3,int b4);
int* float2bytes(double sensor_val);
double* get_latest_values(void);
void handle_auth(void);
void handle_data_sending(void);
//void handle_data_request(void);
int* handle_data_request(double* data2master);
void i2c_send(int master_addr, int* _data, int data_len);
void respond_to_master(double* data2master);

#endif // _SLAVE_HELPER_
