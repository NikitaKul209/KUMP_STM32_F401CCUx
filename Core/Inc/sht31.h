
#ifndef SRC_SHT31_H_
#define SRC_SHT31_H_


const uint16_t I2C_DEV_ADDR = 0x44;

const unsigned short MEASUREMENT_PERIODIC_COMMANDS = 0x2737;
const unsigned short START_PERIODIC_READ_COMMANDS = 0xE000;
const unsigned short I2C_STOP_PERIODIC_READ_COMMAND = 0x3093;
const unsigned short I2C_RESET_COMMAND = 0x0006;

struct sht31_struct{

unsigned char in_buff[6];
float temperature;
float humidity;


};





unsigned char crc8(unsigned char *buff, unsigned int len);
extern void sht3x_send_command(unsigned short command, unsigned char dev_addr);





#endif
