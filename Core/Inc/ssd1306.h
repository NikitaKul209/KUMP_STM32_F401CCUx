/*
 * ssd1306.h
 *
 *  Created on: Oct 23, 2023
 *      Author: nkulikov
 */

#ifndef SRC_SSD1306_H_
#define SRC_SSD1306_H_
#define SSD1306_I2C_TIMEOUT                                             100
#define SSD1306_I2C_ADDRESS                                             0x78
#define SSD1306_I2C_CONTROL_BYTE_COMMAND                                0x00
#define SSD1306_I2C_CONTROL_BYTE_DATA                                   0x40


#define SSD1306_X_SIZE                                                  128
#define SSD1306_Y_SIZE                                                  64
#define SSD1306_BUFFER_SIZE                                             (SSD1306_X_SIZE *  SSD1306_Y_SIZE) / 8


typedef enum
{
  SSD1306_READY = 0x00,
  SSD1306_BUSY  = 0x01
} SSD1306_State;



extern void SendCommand(uint8_t* data, uint8_t size);
extern void SendData(uint8_t *data, uint16_t size);
extern void SSD1306_Init();

extern void SetPixel(uint8_t x, uint8_t y);
extern void SSD1306_ClearScreen();
extern void SSD1306_UpdateScreen();
extern void SSD1306_DrawFilledRect(uint8_t xStart, uint8_t xEnd, uint8_t yStart, uint8_t yEnd);
extern uint8_t SSD1306_IsReady();




#endif /* SRC_SSD1306_H_ */
