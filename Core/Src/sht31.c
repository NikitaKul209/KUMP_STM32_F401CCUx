#include "main.h"
#include "sht31.h"
#include "i2c.h"
#include <stdbool.h>

const uint16_t I2C_DEV_ADDR = 0x44;
const unsigned short START_SINGLE_SHOT_MODE = 0x2c06;
const unsigned short MEASUREMENT_PERIODIC_COMMANDS = 0x2737;
const unsigned short START_PERIODIC_READ_COMMANDS = 0xE000;
const unsigned short I2C_STOP_PERIODIC_READ_COMMAND = 0x3093;
const unsigned short I2C_RESET_COMMAND = 0x0006;

 void sht3x_send_command(unsigned short command, unsigned char dev_addr)
{
	uint8_t command_buffer[2] = {(command & 0xff00u) >> 8u, command & 0xffu};

	 HAL_I2C_Master_Transmit_IT(&hi2c1, dev_addr<<1, command_buffer, sizeof(command_buffer));


}


bool sht3x_read_temperature_and_humidity(I2C_HandleTypeDef *hi2c, struct sht31_struct* sht,union unn_t *unn )
 {

 	if(sht->rx_done_flag){

 		sht->rx_done_flag = false;

 	 	uint8_t temperature_crc = crc8(sht->in_buff, 2);
 	 	uint8_t humidity_crc = crc8(sht->in_buff + 3, 2);

 	 	if (temperature_crc != sht->in_buff[2] || humidity_crc != sht->in_buff[5]) {
 	 		return false;
 	 	}

 	 	unn->ch_val[0] = sht->in_buff[0];
 	 	unn->ch_val[1] = sht->in_buff[1];
 	 	float temperature = unn->w_val;

 	 	unn->ch_val[0] = sht->in_buff[3];
 	 	unn->ch_val[1] = sht->in_buff[4];
 	 	float humidity = unn->w_val;

 	 	temperature =  -45.0f + 175.0f * temperature / 65535.0f;
 	 	humidity = 100.0f * humidity / 65535.0f;

 	 	sht->temperature[sht->byte_counter++] = temperature;
 	 	sht->temperature[sht->byte_counter++] = humidity;

 	 	if (sht->byte_counter == 4){

 	 		sht->byte_counter = 0;
 	 	}


 	 	return true;

 	}
return false;
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



