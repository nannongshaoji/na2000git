
#ifndef __MANAGE_H__
#define __MANAGE_H__

#include <sysapp.h>

/*POWER,RUN,ACTIVE,FAULT指示灯*/
#define SYS_GPIO_CLK_PORT         RCC_AHB1Periph_GPIOD 
#define SYS_GPIO_PORT             GPIOD
#define SYS_POWER_PIN          GPIO_Pin_13
#define SYS_RUN_PIN            GPIO_Pin_12
#define SYS_ACTIVE_PIN         GPIO_Pin_11
#define SYS_FAULT_PIN          GPIO_Pin_10

#define SYS_RUN_LED            1
#define SYS_ACTIVE_LED         2
#define SYS_FAULT_LED          3

/*拨码开关GPIO*/
#define SW_GPIO_CLK_PORT         RCC_AHB1Periph_GPIOG 
#define SW_GPIO_PORT             GPIOG
#define SW_DEBUG_PIN             GPIO_Pin_7
#define SW_RUN_PIN               GPIO_Pin_6

int Light(int LightNo, int OnOff);
int ResetCPU(void);
void SW_GPIO_Init(void);
int GetHardSW(void);

void SetIP(void);
void manage_main(void* param);


int RTCRead(CLOCK *p);
int RTCWrite(CLOCK *p) ;

int plc_data_init(void);
int serial_fd_init(void);
extern void Na2000_Timer_init(void);

extern void intprgthread(void *lp);
extern void taskprgthread(void *lp);
extern void lineprgthread(void *lp);

extern void comfree(void *lp);
extern void commodrtu(void *lp);
extern void commodmst(void *lp);
extern void commodmst_for_lcd(void *lp);

extern void DO_8_Write(unsigned int date);
extern void DO_Write(unsigned char ch,unsigned char date);
extern unsigned int DI_16_Read(void);
extern void can1mgr(void *lp);

#define CPLD_RUN 10
#define CPLD_COM 9
#define CPLD_ERR 8

#define MAX_INT_PRG_NUM			4
#define MAX_TASK_PRG_NUM		16


#define CPU2001_2401  41
#define CPU2001_2402  42
#define CPU2001_2403  43
#define CPU2001_2404  44
#define CPU2001_2411  45

extern unsigned char INT_PRG_ID[MAX_INT_PRG_NUM];
extern unsigned char TASK_PRG_ID[MAX_TASK_PRG_NUM];
extern unsigned char cputype;

#endif
