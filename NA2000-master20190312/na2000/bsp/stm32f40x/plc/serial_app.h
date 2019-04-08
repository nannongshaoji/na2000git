#ifndef SERIAL_H
#define SERIAL_H

extern int openport(char *Dev)  ;
extern int setport(int fd, int baud,int databits,int stopbits,int parity);
extern int readport(int fd, char *buf, int len, int maxwaittime, int maxinterval);
extern int writeport(int fd,char *buf,int len) ;
extern void clearport(int fd);
extern void closeport(int fd);

#endif
