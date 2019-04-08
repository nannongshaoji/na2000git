#ifndef CPLD_H
#define CPLD_H

#include "stm32f4xx.h"

void CPLD_Write(unsigned char addr,unsigned int date);
unsigned int CPLD_Read(unsigned char addr);

unsigned char has_sub_modules(void);
void get_hsc_status(void);
void set_pto_status(void);
void set_hsc_status(void);
void set_hsc_param(void);
//void TIM3_IRQ(void);
//void Init_CPLD(void);

//int pto_test(void);
#define INT_TIME1		1
#define INT_TIME2		2
#define INT_TIME3		3
#define INT_TIME4		4

#define INT_UP_DI13		5
#define INT_DOWN_DI13	6
#define INT_UP_DI14		7
#define INT_DOWN_DI14	8
#define INT_UP_DI15		9
#define INT_DOWN_DI15	10
#define INT_UP_DI16		11
#define INT_DOWN_DI16	12

#define INT_HSC1		21
#define INT_HSC2		22
#define INT_HSC3		23
#define INT_HSC4		24

#define INT_PTO1		31
#define INT_PTO2		32
#define INT_PTO3		33
#define INT_PTO4		34

#endif

