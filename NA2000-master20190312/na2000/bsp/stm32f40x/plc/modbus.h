#ifndef MODBUS_H
#define MODBUS_H

extern int MODBUS_Read_CoilStatus(int fd, unsigned char SlaveAdd, unsigned short RegAdd, unsigned short RegNum, unsigned char *value, int maxwaittime, int maxinterval);
extern int MODBUS_Read_InputStatus(int fd, unsigned char SlaveAdd, unsigned short RegAdd, unsigned short RegNum, unsigned char *value, int maxwaittime, int maxinterval);
extern int MODBUS_Read_HoldingRegister(int fd, unsigned char SlaveAdd, unsigned short RegAdd, unsigned short RegNum, unsigned char *value, int maxwaittime, int maxinterval);
extern int MODBUS_Read_InputRegister(int fd, unsigned char SlaveAdd, unsigned short RegAdd, unsigned short RegNum, unsigned char *value, int maxwaittime, int maxinterval);
extern int MODBUS_Write_SingleCoil(int fd, unsigned char SlaveAdd, unsigned short RegAdd, unsigned char *value, int maxwaittime, int maxinterval);
extern int MODBUS_Write_MultiCoil(int fd, unsigned char SlaveAdd, unsigned short RegAdd, unsigned short RegNum, unsigned char *value, int maxwaittime, int maxinterval);
extern int MODBUS_Write_SingleRegister(int fd, unsigned char SlaveAdd, unsigned short RegAdd, unsigned char *value, int maxwaittime, int maxinterval);
extern int MODBUS_Write_MultiRegister(int fd, unsigned char SlaveAdd, unsigned short RegAdd, unsigned short RegNum, unsigned char *value, int maxwaittime, int maxinterval);
#endif
