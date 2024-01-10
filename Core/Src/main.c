/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */


#include "sht31.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

const unsigned short REG_INPUT_START = 0x0;
const unsigned short REG_INPUT_NREGS = 0x4;

const unsigned char FC_RD_INPUT_RG = 0x4;

const unsigned char EXCEPTION_CODE1 = 1;
const unsigned char EXCEPTION_CODE2 = 2;
const unsigned char EXCEPTION_CODE3 = 3;
const unsigned char EXCEPTION_CODE4 = 4;

const unsigned char FC_04_HLENGTH_WITHOUT_CRC = 0x3;
const unsigned char FC_04_HLENGTH_WITH_CRC = 0x5;

const unsigned char DEV_ADDR = 0x40;

const unsigned int ADU_MIN = 5;
const unsigned int ADU_MAX = 256;

unsigned short usRegInputStart = 0x0;
signed short usRegInputBuf[4] = {0x0};

unsigned short wreq_addr;
unsigned short wreq_dt;

bool FE_Error = false;
bool OE_Error = false;
bool PE_Error = false;
bool NE_Error = false;
int exception = 0;

enum states {
	start_uart_receive_data,
	start_uart_transmit_data,
	check_received_data,
	modbus_functions
};


union unn_t unn;

struct sht31_struct sht31 = {

.byte_counter = 0,
.humidity = { 0 },
.temperature = { 0 } ,
.i2c_inbuff = { 0 },
.rx_done_flag = false

};

struct Uart uart = {

.rx_done_flag = false,
.tx_done_flag = false,
.tx_ready_flag = false,
.uart_inbuf = { 0 },
.uart_outbuf = { 0 },
.p_uart_inbuf = uart.uart_inbuf,
.p_uart_outbuf = uart.uart_outbuf,
.receive_byte = 0,
.byte_to_send = 0,
.state = start_uart_receive_data
};

struct Adc adc_struct = {

.adc_counter = 0,
.adc_data_ready = false,
.adc = { 0 },
.adc_val = 0,
.pressure = 0,
.voltage = 0

};




/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
signed char Check_Uart_inbuff(struct Uart *RxTx);
void data_exchange(struct Uart *RxTx);
void modbus_function(struct Uart *RxTx);
unsigned char crc8(unsigned char *buff, unsigned int len);
char crc16in(unsigned char size, unsigned char *inbuf);
void crc16_out(unsigned char size, unsigned char *outbuf);
int ReadInputReg(struct Uart *RxTx, unsigned short usAddress,signed short sNRegs);
void Get_Temp_Humidity_Value();
float Get_Pressure_Value(struct Adc *adc_s);

//float get_filtred_data(float *buff,int window);


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM1_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */

  set_status_flag(PMNC_BIT_POS);
  set_status_flag(THMNC_BIT_POS);

  TIM_GET_CLEAR_IT(&htim1,TIM_IT_UPDATE);
  TIM_GET_CLEAR_IT(&htim2,TIM_IT_UPDATE);
  TIM_GET_CLEAR_IT(&htim3,TIM_IT_UPDATE);
  TIM_GET_CLEAR_IT(&htim4,TIM_IT_UPDATE);
  HAL_TIM_Base_Start_IT(&htim3);
  HAL_TIM_Base_Start_IT(&htim4);
  HAL_ADC_Start_IT(&hadc1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
		sht3x_read_temperature_and_humidity(&hi2c1, &sht31, &unn, usRegInputBuf);
		Get_Pressure_Value(&adc_struct);
		data_exchange(&uart);

	}

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void data_exchange(struct Uart *RxTx) {

	switch (RxTx->state) {

	case start_uart_receive_data:

		if(UART_Start_Receive_IT(&huart1, uart.p_uart_inbuf++, 1)==HAL_OK){

			RxTx->state = check_received_data;
		};

		break;

	case check_received_data:

		if (RxTx->rx_done_flag) {
			RxTx->rx_done_flag = false;
			RxTx->p_uart_inbuf = RxTx->uart_inbuf;

			if ((Check_Uart_inbuff(RxTx) == 0)
					&& (RxTx->uart_inbuf[0] == DEV_ADDR)) {
				RxTx->state = modbus_functions;
			} else {

				HAL_TIM_Base_Stop_IT(&htim2);
				RxTx->state = start_uart_receive_data;
			}
			FE_Error = 0;
			OE_Error = 0;
			PE_Error = 0;
			NE_Error = 0;
			RxTx->receive_byte = 0;

		}

		break;

	case modbus_functions:
		modbus_function(RxTx);
		RxTx->state = start_uart_transmit_data;
		break;

	case start_uart_transmit_data:

		if (RxTx->tx_ready_flag == true) {
			RxTx->tx_ready_flag = false;
			HAL_UART_Transmit_IT(&huart1, RxTx->uart_outbuf,
					RxTx->byte_to_send);
			RxTx->state = start_uart_receive_data;

		}
		break;

	}

}

void modbus_function(struct Uart *RxTx) {

////////////////READ_INPUT_REGISTERS//////////////////
	if (RxTx->uart_inbuf[1] == FC_RD_INPUT_RG) {

		unn.ch_val[1] = RxTx->uart_inbuf[2];
		unn.ch_val[0] = RxTx->uart_inbuf[3];
		wreq_addr = unn.w_val;
		unn.ch_val[1] = RxTx->uart_inbuf[4];
		unn.ch_val[0] = RxTx->uart_inbuf[5];
		wreq_dt = unn.w_val;
		exception = ReadInputReg(RxTx, wreq_addr, wreq_dt);

	} else {

		exception = EXCEPTION_CODE1;
	}
////////////////END_READ_INPUT_REGISTERS//////////////////
	if (exception) {

		RxTx->uart_outbuf[0] = DEV_ADDR;
		RxTx->uart_outbuf[1] = RxTx->uart_inbuf[1] | 0x80;
		RxTx->uart_outbuf[2] = exception;
		crc16_out(0x3, RxTx->uart_outbuf);
		RxTx->byte_to_send = 5;
		exception = 0;

	} else {

		RxTx->uart_outbuf[0] = DEV_ADDR;
		RxTx->uart_outbuf[1] = RxTx->uart_inbuf[1];
		RxTx->uart_outbuf[2] = wreq_dt << 1;
		RxTx->byte_to_send = (wreq_dt << 1) + FC_04_HLENGTH_WITH_CRC;
		crc16_out((wreq_dt << 1) + FC_04_HLENGTH_WITHOUT_CRC,RxTx->uart_outbuf);

	}

}

int ReadInputReg(struct Uart *RxTx, unsigned short usAddress,signed short sNRegs) {

	int iRegIndex = 0x0;
	int RegBufferIndex = 0x3;

	if ((sNRegs >= 0x0001) && (sNRegs <= REG_INPUT_NREGS)) {

		if ((usAddress >= REG_INPUT_START) && (usAddress + sNRegs <= REG_INPUT_START + REG_INPUT_NREGS)) {
			iRegIndex = (int) (usAddress - usRegInputStart);
			while (sNRegs > 0) {
				RxTx->p_uart_outbuf[RegBufferIndex++] = (uint8_t) (usRegInputBuf[iRegIndex] >> 8);
				RxTx->p_uart_outbuf[RegBufferIndex++] = (uint8_t) (usRegInputBuf[iRegIndex] & 0xFF);
				iRegIndex++;
				sNRegs--;
			}

		}

		else

		{

			exception = EXCEPTION_CODE2;
		}
	}

	else {

		exception = EXCEPTION_CODE3;
	}

	return exception;
}


float Get_Pressure_Value(struct Adc *adc_s) {

	if (adc_s->adc_data_ready) {
		adc_s->adc_data_ready = false;

		adc_s->adc_val = get_filtred_data(adc_s->adc, ADC_FILTR_WINDOW);
		adc_s->voltage = adc_s->adc_val * 3.3 / 4096;
		adc_s->pressure = (adc_s->voltage / 3.3 + 0.00842) / 0.002421;

		usRegInputBuf[1] = adc_s->pressure*10;
		reset_status_flag(PMNC_BIT_POS);

		if (adc_s->pressure > MAX_PRESS || adc_s->pressure < MIN_PRESS){

			set_status_flag(POL_BIT_POS);
		}
		else
		{
			reset_status_flag(POL_BIT_POS);
		}



	}

	return adc_s->adc_val;
}



float get_filtred_data(float *buff,int window) {

	float sum = 0;
	for (int i = 0; i < window; i++) {

		sum += buff[i];
	}
	return sum / window;

}



void set_status_flag(int flag_pos){

	usRegInputBuf[0] |= 1<<flag_pos;

}

void reset_status_flag(int flag_pos){

	usRegInputBuf[0] &= ~(1<<flag_pos);

}



signed char Check_Uart_inbuff(struct Uart *RxTx) {

	if (RxTx->receive_byte >= ADU_MAX) {

		return -4;
	}

	if (RxTx->receive_byte < ADU_MIN) {

		return -3;
	}

	if (FE_Error || OE_Error || PE_Error||NE_Error) {

		return -2;
	}

	if (crc16in(RxTx->receive_byte, RxTx->uart_inbuf) != 0) {

		return -1;
	}

	return 0;
}

char crc16in(unsigned char size, unsigned char *inbuf) {
	unsigned short w = 0xffff, w1;
	char shift_cnt, jj;
	unsigned short ii1 = 0;

	union {
		char c[2];
		unsigned short w;
	} u;

	size = size - 2;
	u.c[0] = inbuf[size];
	u.c[1] = inbuf[size + 1];
	jj = size;

	for (; jj > 0; jj--) {
		w1 = (w >> 8) << 8;
		w = (w1 + ((w - w1) ^ (unsigned short) inbuf[ii1++]));

		for (shift_cnt = 0; shift_cnt < 8; shift_cnt++) {
			if ((w & 0x01) == 1)
				w = ((w >> 1) ^ 0xa001);
			else
				w >>= 1;
		}
	}

	if (w == u.w)
		return 0;
	else
		return (char) -1;
}


void crc16_out(unsigned char size, unsigned char *outbuf) {
	unsigned short w = 0xffff, w1;
	char shift_cnt, jj;
	unsigned short ii2 = 0;

	jj = size;

	for (; jj > 0; jj--) {
		w1 = (w >> 8) << 8;
		w = (w1 + ((w - w1) ^ (unsigned short) outbuf[ii2++]));

		for (shift_cnt = 0; shift_cnt < 8; shift_cnt++) {
			if ((w & 0x01) == 1)
				w = ((w >> 1) ^ 0xa001);
			else
				w >>= 1;
		}
	}

	outbuf[size++] = (char) (w & 0x00ff);
	outbuf[size] = (char) (w >> 8);
}







void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c){

	if(hi2c == &hi2c1){

		sht31.rx_done_flag = true;

	}

}


HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c){

	sht31.i2c_ecode = HAL_I2C_GetError(hi2c);
//	I2C_FLAG_ARLO

}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {

	if (hadc == &hadc1) {

		adc_struct.adc[adc_struct.adc_counter++] = HAL_ADC_GetValue(&hadc1);
		adc_struct.adc_data_ready = true;

		if (adc_struct.adc_counter == 10) {
			adc_struct.adc_counter = 0;
		}

	}

}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == &huart1) {
		uart.tx_done_flag = true;

	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

	if (huart == &huart1) {

		HAL_TIM_Base_Stop_IT(&htim1);
		HAL_TIM_Base_Stop_IT(&htim2);

		__HAL_TIM_SetCounter(&htim1,0x0);
		__HAL_TIM_SetCounter(&htim2,0x0);

		HAL_TIM_Base_Start_IT(&htim1);
		HAL_TIM_Base_Start_IT(&htim2);

		uart.uart_ecode = HAL_UART_GetError(&huart1);
		uart.receive_byte++;
		UART_Start_Receive_IT(&huart1, uart.p_uart_inbuf++, 1);

	}

}


void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {

	if (huart == &huart1) {

		if (uart.uart_ecode & UART_FLAG_FE) {

			FE_Error = true;
		}
		if (uart.uart_ecode & UART_FLAG_PE) {

			PE_Error = true;
		}
		if (uart.uart_ecode & UART_FLAG_ORE) {

			OE_Error = true;
		}
		if (uart.uart_ecode &  USART_SR_NE ){

			NE_Error = true;
		}
		uart.uart_ecode = 0;

	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
