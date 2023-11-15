


#include "main.h"
#include "sht31.h"
#include "i2c.h"
#include <stdbool.h>

 void sht3x_send_command(unsigned short command, unsigned char dev_addr)
{
	uint8_t command_buffer[2] = {(command & 0xff00u) >> 8u, command & 0xffu};

	 HAL_I2C_Master_Transmit_IT(&hi2c1, dev_addr<<1, command_buffer, sizeof(command_buffer));




}


 bool sht3x_read_temperature_and_humidity(I2C_HandleTypeDef hi2c, struct sht31_struct* sht31, float *temperature, float *humidity)
 {
 	sht3x_send_command(MEASUREMENT_PERIODIC_COMMANDS, I2C_DEV_ADDR);


 	HAL_I2C_Mem_Read_IT(&hi2c, I2C_DEV_ADDR, START_PERIODIC_READ_COMMANDS, 0x2, sht31->in_buff, 0x6);

 	uint8_t temperature_crc = crc8(sht31->in_buff, 2);
 	uint8_t humidity_crc = crc8(sht31->in_buff + 3, 2);

 	if (temperature_crc != sht31->in_buff[2] || humidity_crc != sht31->in_buff[5]) {
 		return false;
 	}

 	sht31->temperature = (sht31->in_buff[0]>>)

 	uint16_t temperature_raw = uint8_to_uint16(buffer[0], buffer[1]);
 	uint16_t humidity_raw = uint8_to_uint16(buffer[3], buffer[4]);

 	*temperature = -45.0f + 175.0f * (float)temperature_raw / 65535.0f;
 	*humidity = 100.0f * (float)humidity_raw / 65535.0f;

 	return true;
 }


 unsigned char crc8(unsigned char *buff, unsigned int len)
 {
     unsigned char crc = 0xFF;
     unsigned int i;

     while (len--)
     {
         crc ^= *buff++;

         for (i = 0; i < 8; i++)
             crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
     }

     return crc;
 }



