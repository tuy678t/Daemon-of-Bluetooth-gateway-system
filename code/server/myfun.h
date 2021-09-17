#ifndef MYFUN_H
#define MYFUN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>


#define _IP "0.0.0.0"
#define _PORT 666
#define HEAD 0xAAAAAAAA
#define TAIL 0xBBBBBBBB
#define PS_SIZE 12
#define PM_SIZE 116
#define PL_SIZE 1024
#define PATCH_SIZE 1008

typedef struct package_s
{
    uint32_t head;
    uint8_t cmd;
    uint8_t flag;
    uint8_t info[2];
    uint32_t tail;
} Package_S;

typedef struct package_m
{
    uint32_t head;
    uint8_t cmd;
    uint8_t flag;
    uint16_t hw_len;
    uint8_t hw_dat[32];
    uint16_t sw_len;
    uint8_t sw_dat[32];
    uint16_t fc_len;
    uint8_t fc_dat[32];
    uint32_t checksum;
    uint32_t tail;
} Package_M;

typedef struct package_l
{
    uint32_t head;
    uint8_t cmd;
    uint8_t flag;
    uint16_t len;
    uint8_t data[1008];
    uint32_t checksum;
    uint32_t tail; 
} Package_L;

void package_init_s(Package_S* psp,uint8_t cmd,uint8_t flag);

void package_init_m(Package_M* pmp,uint8_t cmd,uint8_t flag);

void package_init_l(Package_L* plp,uint8_t cmd,uint8_t flag);

int get_sock();

int seek_sock();

void get_checksum(int* buf, int size);

int do_checksum(int* buf, int size);

int transferd(int rfd,int wfd);

void writelog(const char* log);

#endif