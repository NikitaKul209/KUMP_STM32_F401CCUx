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


bool sht3x_read_temperature_and_humidity(I2C_HandleTypeDef *hi2c, struct sht31_struct* sht,union unn_t *unn, signed short *RegBuff )
 {
	if(sht->i2c_ecode){
		sht->i2c_ecode = false;
		I2C_Deinit();

	}

	if (sht->i2c_start_flag){
//		sht->i2c_ecode =  HAL_I2C_GetState(&hi2c1);
		sht->i2c_start_flag = false;
		if (HAL_I2C_Mem_Read_IT(hi2c, I2C_DEV_ADDR<<1, START_SINGLE_SHOT_MODE, 0x2, sht->i2c_inbuff, 0x6) != HAL_OK){
			set_status_flag(IICE_BIT_POS);
			I2C_Deinit();
		};

	}
 	if(sht->rx_done_flag){
 		reset_status_flag(IICE_BIT_POS);
 		sht->rx_done_flag = false;

 	 	uint8_t temperature_crc = crc8(sht->i2c_inbuff, 2);
 	 	uint8_t humidity_crc = crc8(sht->i2c_inbuff + 3, 2);

 	 	if (temperature_crc == sht->i2c_inbuff[2] && humidity_crc == sht->i2c_inbuff[5]) {

 	 		unn->ch_val[1] = sht->i2c_inbuff[0];
			unn->ch_val[0] = sht->i2c_inbuff[1];
			float temperature = unn->w_val;

			unn->ch_val[1] = sht->i2c_inbuff[3];
			unn->ch_val[0] = sht->i2c_inbuff[4];
			float humidity = unn->w_val;

			temperature = (-45.0f + 175.0f * temperature / 65535.0f)*10;
			humidity = (100.0f * humidity / 65535.0f)*10;

			sht->temperature[sht->byte_counter] = temperature;
			sht->humidity[sht->byte_counter++] = humidity;

			sht->average_temperature = get_filtred_data( sht->temperature, I2C_FILTR_WINDOW);
			sht->average_humidity = get_filtred_data( sht->humidity, I2C_FILTR_WINDOW);

			RegBuff[2] = sht->average_temperature;
			RegBuff[3] = sht->average_humidity;

			if (sht->byte_counter >= I2C_FILTR_WINDOW){

		 	 		sht->byte_counter = 0;
		 	 	}

			reset_status_flag(THMNC_BIT_POS);
			reset_status_flag(CRCE_BIT_POS);

			if(sht->average_temperature > MAX_TEMP * 10 || sht->average_temperature < MIN_TEMP * 10)
				{
					set_status_flag(TOL_BIT_POS);
				}
				else
				{
					reset_status_flag(TOL_BIT_POS);
				}

				if(sht->average_humidity >MAX_HUM * 10 || sht->average_humidity < MIN_HUM * 10)
				{
					set_status_flag(HOL_BIT_POS);
				}
				else
				{
					reset_status_flag(HOL_BIT_POS);
				}

 	 	}
 	 	else
 	 	{

 	 		set_status_flag(CRCE_BIT_POS);
 	 	}



 	 	return true;

 	}
return false;
 }


void I2C_Deinit(void){
	HAL_I2C_Master_Abort_IT(&hi2c1,I2C_DEV_ADDR);

	HAL_TIM_Base_Stop_IT(&htim4);

	TIM_GET_CLEAR_IT(&htim4,TIM_IT_UPDATE);
	__HAL_TIM_SetCounter(&htim4,0x0);


	__HAL_I2C_ENABLE(&hi2c1);
	hi2c1.Instance->CR1|=1<<15;
	HAL_Delay(2);
	hi2c1.Instance->CR1&= ~(1<<15);
	HAL_I2C_MspDeInit(&hi2c1);
	HAL_I2C_MspInit(&hi2c1);

	 HAL_I2C_DeInit(&hi2c1);
	 HAL_I2C_Init(&hi2c1);
//	HAL_I2C_Master_Transmit_IT(&hi2c1, 0x0<<1, 0x06, 1);
	HAL_TIM_Base_Start_IT(&htim4);



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



