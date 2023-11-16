
#ifndef _SHT31_H_
#define _SHT31_H_

extern const uint16_t I2C_DEV_ADDR ;
extern const unsigned short START_SINGLE_SHOT_MODE;


struct sht31_struct{

unsigned char in_buff[6];
float temperature[4];
float humidity[4];
bool rx_done_flag;
int byte_counter;

};

extern struct sht31_struct sht31;

bool sht3x_read_temperature_and_humidity(I2C_HandleTypeDef *hi2c, struct sht31_struct* sht31,union unn_t *unn );
unsigned char crc8(unsigned char *buff, unsigned int len);






#endif
