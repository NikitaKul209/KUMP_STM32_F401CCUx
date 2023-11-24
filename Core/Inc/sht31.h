
#ifndef _SHT31_H_
#define _SHT31_H_

#define MAX_TEMP 125
#define MIN_TEMP -40
#define MAX_HUM 100
#define MIN_HUM 0
extern const uint16_t I2C_DEV_ADDR ;
extern const unsigned short START_SINGLE_SHOT_MODE;


struct sht31_struct{

unsigned char i2c_inbuff[6];
float temperature[10];
float humidity[10];
float average_temperature;
float average_humidity;
bool rx_done_flag;
int byte_counter;

};

extern struct sht31_struct sht31;

bool sht3x_read_temperature_and_humidity(I2C_HandleTypeDef *hi2c, struct sht31_struct* sht,union unn_t *unn, signed short *RegBuff );
unsigned char crc8(unsigned char *buff, unsigned int len);






#endif
