#ifndef __SPI_BD_BOARD_H
#define __SPI_BD_BOARD_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_spi.h"

#include <rtthread.h>

#define		AD7323DATAW			1 << 15
#define		AD7323RS1			1 << 14
#define		AD7323RS2			1 << 13

#define		AD7232VALIDBIT		0x3FFF
#define		CHANAMASK			0x6000
#define		DATAMASK			0x1FFF
#define		CHANNAL0			0x0000
#define		CHANNAL1			0x2000
#define		CHANNAL2			0x4000
#define		CHANNAL3			0x6000
#define		NOTPOINTADDR		0x0000

#define		AD7323DATABITS		16
#define		RANNOTCAREBITS		5
#define		SEQNOTCAREBITS		9
#define		CTLNOTCAREBITS		2

#define		AD7232CTLADD1		1 << 9
#define		AD7232CTLADD0		1 << 8
#define		AD7232CTLM1			1 << 7
#define		AD7232CTLM0			1 << 6
#define		AD7232CTLPM1		1 << 5
#define		AD7232CTLPM0		1 << 4
#define		AD7232CTLCODE		1 << 3
#define		AD7232CTLREF		1 << 2
#define		AD7232CTLSEQ1		1 << 1
#define		AD7232CTLSEQ2		1 << 0

#define		AD7232SEQVIN0		1 << 3
#define		AD7232SEQVIN1		1 << 2
#define		AD7232SEQVIN2		1 << 1
#define		AD7232SEQVIN3		1 << 0

#define		AD7232RNGVIN0A		1 << 7
#define		AD7232RNGVIN0B		1 << 6
#define		AD7232RNGVIN1A		1 << 5
#define		AD7232RNGVIN1B		1 << 4
#define		AD7232RNGVIN2A		1 << 3
#define		AD7232RNGVIN2B		1 << 2
#define		AD7232RNGVIN3A		1 << 1
#define		AD7232RNGVIN3B		1 << 0


void init_bd_board(int type);
#endif
