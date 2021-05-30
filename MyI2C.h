#ifndef __I2C_H__
#define __I2C_H__
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

//TM4C123GH6PM
//Data rate is set to 100kbps

#define I2C0
//SCL : PB2
//SDA : PB3
//#define I2C1
//SCL : PA6
//SDA : PA7
//#define I2C2
//SCL : PE4
//SDA : PE5
//#define I2C3
//SCL : PD0
//SDA : PD1

extern void InitI2C0(void);
extern void I2CSend(uint8_t slave_addr, uint8_t num_of_args, ...);
extern uint32_t I2CReceive(uint32_t slave_addr, uint8_t reg);
extern void I2CSend2(uint32_t slave_addr, uint8_t reg);
#endif
