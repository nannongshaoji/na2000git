#include "system.h"
#include "appcnst.h" 
#include "seq.h"
#include "seqext.h"
#include "database.h"

/* using DATATYPE_XXX as its item to get number of bits */
unsigned char BitCnt[] = { 1,8,16,32,8,16,32,32,8,16,32,0,0,0,0,0,0,0,0,0 };

const unsigned char ORBIT_ARRAY[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
const unsigned char CLRBIT_ARRAY[8]={0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f};
const unsigned short WORBIT_ARRAY[16]= {0x0001,0x0002,0x0004,0x0008,0x0010,0x0020,0x0040,0x0080,0x0100,0x0200,0x0400,0x0800,0x1000,0x2000,0x4000,0x8000};
const unsigned short WCLRBIT_ARRAY[16]={0xfffe,0xfffd,0xfffb,0xfff7,0xffef,0xffdf,0xffbf,0xff7f,0xfeff,0xfdff,0xfbff,0xf7ff,0xefff,0xdfff,0xbfff,0x7fff};

//global data
DYNAMIC_DATA *pDynamicData=0;
STATIC_DATA  *pStaticData=0;
NVRAM_DATA *pNvramData=0;
PRG_DATA *pPrgData=0;
FB_DATA *pFBData=0;
CLOCK *pCCUClock=0;
unsigned char *funstate=0;
FILE_STATE *FileState=0;
FILE_ENTRY *FileEntry=0;
FUNCPTR *PrgEntry=0;
FUNCPTR *FbEntry=0;
FB_INFO *pFBInfo=0;

CONFIG_DATA *pConfigData=0;

unsigned char mon_module=0;
unsigned char debug_flag;

