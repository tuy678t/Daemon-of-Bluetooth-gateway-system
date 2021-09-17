#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "myfun.h"

int main(int argc,char *argv[]){
    printf("[Service]\tservice start\n");
    int fd = atoi(argv[1]);
    
    /*
    //测监控功能
    while(1){
       printf("[Service]\told ncp_host is on\n");
       sleep(5);
    }
    */

    //测回退功能
    sleep(5);
    Package_S send_pkg_s;
    Package_S recv_pkg_s;
    package_init_s(&send_pkg_s,0x03,0x00);
    write(fd,&send_pkg_s,sizeof(send_pkg_s));
    printf("[Service]\tService send recall request\n");
    read(fd,&recv_pkg_s,sizeof(recv_pkg_s));
    if(recv_pkg_s.flag==0x00) printf("[Service]\tDaemon can recall\n");
    else if(recv_pkg_s.flag==0xFF) printf("[Service]\tDaemon cannot recall\n");
    while(1){sleep(3);}

   /*
   //测更新功能
    sleep(5);
    Package_M send_pkg_m;
    Package_S recv_pkg_s;
    package_init_m(&send_pkg_m,0x01,0x03);
    send_pkg_m.hw_len=5;
    memcpy(send_pkg_m.hw_dat,"v1.0",5);
    send_pkg_m.sw_len=7;
    memcpy(send_pkg_m.sw_dat,"v3.0.2",7);
    send_pkg_m.fc_len=13;
    memcpy(send_pkg_m.fc_dat,"v5.6.7.123.5",13);
    write(fd,&send_pkg_m,sizeof(send_pkg_m));
    printf("[Service]\tService send update request\n");
    read(fd,&recv_pkg_s,sizeof(recv_pkg_s));
    if(recv_pkg_s.flag==0x00) printf("[Service]\tDaemon get update package\n");
    else if(recv_pkg_s.flag==0xFF) printf("[Service]\tDaemon cannot update\n");
    while(1) sleep(3);
    */
    return 0;
}