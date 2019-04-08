#ifndef   SEQ_H
#define   SEQ_H 

#define MAX_SEQ_BUF		16
#define MAXFCFILELENGTH 100 
#define MAXFCNUM		32

/* operand type */
#define tNONE           0x00
#define tCONST          0x01
#define tTAG            0x02
#define tID             0x03
#define tLAB            0x04
#define tSTR            0x05
#define tARG            0x06    
#define tSRC            0x07
#define tACC			0x08

/* data type */
#define dINT            0x01
#define dFLOAT          0x02
#define dSTR            0x03
#define dBOOL			0x04

/* instructor */
#define xPUSH           0x01
#define xPOP            0x02
#define xADD            0x03
#define xSUB            0x04
#define xMUL            0x05
#define xDIV            0x06
#define xMOD            0x07
#define xNOT            0x08
#define xLNOT           0x09
#define xAND            0x0a
#define xOR             0x0b
#define xLAND           0x0c
#define xLOR            0x0d
#define xLXOR           0x0e
#define xSHR            0x0f
#define xSHL            0x10
#define xGT             0x11
#define xGE             0x12
#define xLS             0x13
#define xLE             0x14
#define xNE             0x15
#define xEQ             0x16
#define xSIN            0x17
#define xCOS            0x18
#define xTAN            0x19
#define xASIN           0x1a
#define xACOS           0x1b
#define xATAN           0x1c
#define xABS            0x1d
#define xSQRT           0x1e
#define xPOW            0x1f
#define xEXP            0x20
#define xLG             0x21
#define xLN             0x22
#define xMAX            0x23
#define xMIN            0x24
#define xJMP            0x25
#define xJFALSE         0x26
#define xSLEEP			0x2b
#define xPULSE			0x2c
#define xCALLLAD		0x2d
#define xTRAP           0x2e
#define xQUIT           0x2f
#define xEXIT           0x30
#define xDELAY          0x31
#define xSTIMER         0x32
#define xRTIMER         0x33
#define xOPENDISP       0x34
#define xDISP           0x35
#define xLOG            0x36
#define xDIAG           0x37
#define xAUDIO          0x38
#define xSET            0x39
#define xINPUT          0x3a
#define xSTEP           0x3b
#define xSSTEP          0x3c
#define xCALL           0x3d
#define xEXECL          0x3e
#define xREXECL         0x3f
#define xKILL           0x40
#define xRKILL          0x41
#define xLOCK           0x42
#define xRLOCK          0x43
#define xUNLOCK         0x44
#define xRUNLOCK        0x45
#define xMSDELAY        0x46


typedef union
{
    int    iData;
	float  fData;
} CONST_DATA;

typedef struct
{
    int          DataType;
    CONST_DATA   ConstData;
} CONST_DEF;

typedef struct
{
    unsigned char	PointType;
    unsigned char   SubPointType;
	unsigned short  OccNo;
	unsigned short  SubOccNo;
	unsigned short  SubBlockNo;
} TAG_DEF;

typedef struct
{
	int          DataType;
    int          IdNo;
} ID_DEF;

typedef struct
{
    int          InstNo;
	int          Pad;
} LAB_DEF;

typedef struct
{
    int          StrNo;
    int          Pad;
} STR_DEF;
                            
typedef struct
{
    int          Pad1;
    int          Pad2;
} ARG_DEF;
                            
typedef struct
{
    int          Pad1;
    int          Pad2;
} SRC_DEF;
                            
typedef struct
{
    int          OpType;
	union
	{
		CONST_DEF  Const;               /***  OpType = 1  ***/
		TAG_DEF    Tag;                 /***  OpType = 2  ***/
		ID_DEF     Id;                  /***  OpType = 3  ***/
		LAB_DEF    Lab;                 /***  OpType = 4  ***/
		STR_DEF    Str;                 /***  OpType = 5  ***/
		ARG_DEF    Arg;                 /***  OpType = 6  ***/
		SRC_DEF    Src;                 /***  OpType = 7  ***/
	} OpValue;
} OPERAND_DEF;

typedef struct
{   
	unsigned short StepNo;
    unsigned short Operator;
	OPERAND_DEF    Operand;
} INST_DEF;

#endif
