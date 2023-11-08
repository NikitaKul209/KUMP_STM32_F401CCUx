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

#include "image.h"
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

const uint16_t I2C_DEV_ADDR = 0x44;

const uint16_t I2C_START_READ_COMMAND = 0x2C06;
const uint16_t I2C_START_PERIODIC_READ_COMMAND = 0x2737;
const uint16_t I2C_STOP_PERIODIC_READ_COMMAND = 0x3093;
const uint16_t I2C_RESET_COMMAND = 0x0006;


const unsigned char DEV_ADDR = 0x40;

const unsigned int ADU_MIN = 5;
const unsigned int ADU_MAX = 256;

bool FE_Error = false;
bool OE_Error = false;
bool PE_Error = false;

float adc;

union unn_t {
	char ch_val[2];
	unsigned short w_val;
} unn;

enum states {
	start_uart_receive_data,
	start_uart_transmit_data,
	check_received_data,
	modbus_functions
};

uint8_t i2c_inbuf[256];

struct Uart uart = {

.rx_done_flag = false,
.tx_done_flag = false,
.tx_ready_flag = false,
.uart_inbuf = { 0 },
.uart_outbuf = { 0 },
.p_uart_inbuf =uart.uart_inbuf,
.receive_byte = 0,
.byte_to_send = 0,
.state = start_uart_receive_data
};




/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */
void SystemClock_Config(void);
signed char Check_Uart_inbuff(struct Uart *RxTx);
void data_exchange(struct Uart *RxTx);
char crc16in(unsigned char size, union unn_t *unn, unsigned char *inbuf);
void Get_Temp_Humidity_Value();
float Get_Pressure_Value();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
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
	/* USER CODE BEGIN 2 */

	TIM_GET_CLEAR_IT(&htim2,TIM_IT_UPDATE);
	TIM_GET_CLEAR_IT(&htim3,TIM_IT_UPDATE);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {

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
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

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
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

void data_exchange(struct Uart *RxTx) {

	switch (RxTx->state) {

	case start_uart_receive_data:

		HAL_UART_Receive_IT(&huart1, RxTx->p_uart_inbuf++, 1);
		RxTx->state = check_received_data;

		break;

	case check_received_data:

		if (RxTx->rx_done_flag) {
			RxTx->rx_done_flag = false;
			RxTx->p_uart_inbuf = RxTx->uart_inbuf;

			if ((Check_Uart_inbuff(RxTx) == 0) && (RxTx->uart_inbuf[0] == DEV_ADDR)) {
				RxTx->state = modbus_functions;
			}
			else {

				HAL_TIM_Base_Stop_IT(&htim3);
				RxTx->state = start_uart_receive_data;
			}
			RxTx->receive_byte = 0;

		}

		break;

	case modbus_functions:

		RxTx->state = start_uart_transmit_data;
		break;

	case start_uart_transmit_data:

		if (RxTx->tx_ready_flag == true) {
			RxTx->tx_ready_flag = false;
			HAL_UART_Transmit_IT(&huart1, (uint8_t*) "OK", 0x3);
			RxTx->state = start_uart_receive_data;

		}
		break;

	}

}


float Get_Pressure_Value() {

	HAL_ADC_Start(&hadc1);
	if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK) {
		adc = HAL_ADC_GetValue(&hadc1) * 3.3 / 4096;
		HAL_ADC_Stop(&hadc1);
	};
	return adc;
}


void Get_Temp_Humidity_Value(){



	HAL_I2C_Mem_Read_IT(&hi2c1, I2C_DEV_ADDR,I2C_START_PERIODIC_READ_COMMAND ,0x2 ,i2c_inbuf , 0x40);



}

signed char Check_Uart_inbuff(struct Uart *RxTx) {

	if (RxTx->receive_byte >= ADU_MAX) {

		return -4;
	}

	if (RxTx->receive_byte < ADU_MIN) {

		return -3;
	}

	if (FE_Error || OE_Error || PE_Error) {

		FE_Error = 0;
		OE_Error = 0;
		PE_Error = 0;

		return -2;
	}

	if (crc16in(RxTx->receive_byte, &unn, RxTx->uart_inbuf) != 0) {

		return -1;
	}

	return 0;
}

char crc16in(unsigned char size, union unn_t *unn, unsigned char *inbuf) {
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
	unn->w_val = w;

	if (w == u.w)
		return 0;
	else
		return (char) -1;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == &huart1) {
		uart.tx_done_flag = true;

	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

	if (huart == &huart1) {

		HAL_TIM_Base_Stop_IT(&htim2);
		HAL_TIM_Base_Stop_IT(&htim3);

		__HAL_TIM_SetCounter(&htim2,0x0);
		__HAL_TIM_SetCounter(&htim3,0x0);

		HAL_TIM_Base_Start_IT(&htim2);
		HAL_TIM_Base_Start_IT(&htim3);
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_12);

		uart.error_code = HAL_UART_GetError(&huart1);
		HAL_UART_Receive_IT(&huart1, uart.p_uart_inbuf++, 1);
		uart.receive_byte++;

	}

}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {

	if (huart == &huart1) {

		if (uart.error_code & UART_FLAG_FE) {

			FE_Error = true;
//		 			 	 HAL_UART_Transmit_IT(&huart1, (uint8_t*) "FE", 0x3);
		}
		if (uart.error_code & UART_FLAG_PE) {

			PE_Error = true;
//		 			 	 HAL_UART_Transmit_IT(&huart1, (uint8_t*) "PE", 0x3);
		}
		if (uart.error_code & UART_FLAG_ORE) {

			OE_Error = true;
//		 			    HAL_UART_Transmit_IT(&huart1, (uint8_t*) "OE", 0x3);

		}
		uart.error_code = 0;

	}
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
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
