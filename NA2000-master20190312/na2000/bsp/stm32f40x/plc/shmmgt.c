/* -------------------------------------------------------------------------------
                                RTU Software 
                      
				 Version	:	1.0                                                      
				Filename	:	shmmgt.c                             
   Originally Written By	:	Chen Sining                          
					Date	:	2017.10.31
       Application Notes	:	This file is for Ethernet debug task of single CPU.
         Updating Record	:	( Pls. Specify : Who , Where , When, Why , What )
    
--------------------------------------------------------------------------------- */
#include "stm32f4xx.h"
#include "sysapp.h"

/*
函数介绍：为变量malloc分配内存空间
返回值  ：函数成功执行返回0，否则返回-1
*/

DYNAMIC_DATA 	DynamicData = {0,};
STATIC_DATA		StaticData = {0,};
PRG_DATA			PrgData = {0,};
FB_DATA				FBData = {0,};
CLOCK					CCUClock = {0,};
unsigned char	fun_state[MAX_LD/8] = {0,};
FILE_STATE		file_state[MAX_FILE_STATE] = {{0,},};
FILE_ENTRY		file_entry[MAX_FILE_STATE] = {{0,},};
FUNCPTR				prg_entry[MAX_LD] = {0,};
FUNCPTR				fb_entry[MAX_FB] = {0,};
FB_INFO				FBInfo = {0,};
CONFIG_DATA		ConfigData = {0,};

void plc_data_init()
{
	pDynamicData = &DynamicData;
	pStaticData = &StaticData;
	
	/*掉电保持区数据*/
	pNvramData = (NVRAM_DATA *)(BKPSRAM_BASE);
	
	pPrgData = &PrgData;
	pFBData = &FBData;
	pCCUClock = &CCUClock;

	funstate = fun_state;
	FileState = file_state;
	FileEntry = file_entry;
	PrgEntry = prg_entry;
	FbEntry = fb_entry;
	pFBInfo = &FBInfo;
	pConfigData = &ConfigData;
}
