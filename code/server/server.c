#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "myfun.h"

#define SIM_FILE "service.tar.gz"

#define SERVER_BUF_SIZE 2048
uint8_t r_buf[SERVER_BUF_SIZE];
uint8_t s_buf[SERVER_BUF_SIZE];

int main(){
    Package_S sps;
    Package_M spm;
    Package_L spl;
    Package_S rps;
    Package_M rpm;
    Package_L rpl;
    int pre_pkg=0;
    int connfd=-1;
    int rdffd=-1;
    
    int listen_sock=get_sock();
    while(1){
        connfd = accept(listen_sock, (struct sockaddr *)NULL, NULL);
        if(connfd==-1){perror("accept failed\n");continue;}
        printf("[Server] Server is connected\n");

        fd_set read_fdset;
        struct timeval timeout;
        FD_ZERO(&read_fdset);
        FD_SET(connfd,&read_fdset);
        timeout.tv_sec=5;
        timeout.tv_usec=0;
        int sel_ret=-1;

        while(1){
            do{
                sel_ret=select(connfd+1,&read_fdset,NULL,NULL,&timeout);
            }while(sel_ret<0&&errno==EINTR);
            if(sel_ret==0){
                if(pre_pkg==0){
                    printf("[Server] Client no package, wait again\n");
                }
                else if(pre_pkg==PS_SIZE){
                    int write_ret=write(connfd,&sps,sizeof(sps));
                    if(write_ret==-1) {perror("write error\n");exit(2);}
                    printf("[Server] Server write finish %d\n",write_ret);
                }
                else if(pre_pkg==PM_SIZE){
                    int write_ret=write(connfd,&spm,sizeof(spm));
                    if(write_ret==-1) {perror("write error\n");exit(2);}
                    printf("[Server] Server write finish %d\n",write_ret);
                }
                else if(pre_pkg==PL_SIZE){
                    int write_ret=write(connfd,&spl,sizeof(spl));
                    if(write_ret==-1) {perror("write error\n");exit(2);}
                    printf("[Server] Server write finish %d\n",write_ret);               
                }
                continue;
            }
            else if(sel_ret==-1) {perror("select error\n");exit(2);}
            else{
                int read_ret=read(connfd,r_buf,sizeof(r_buf));
                if(read_ret==-1) {perror("read error\n");exit(2);}
                else if(read_ret==PS_SIZE){
                    memcpy(&rps,r_buf,read_ret);
                    if(rps.cmd==0x12||rps.cmd==0x13){
                        //printf("[Server] Client ACK 0x12||0x13 received\n");   
                        int file_read_ret=read(rdffd,s_buf,PATCH_SIZE);
                        if(file_read_ret==-1) {perror("file read error\n");exit(2);}
                        else if(file_read_ret==0){
                            printf("[Server] Send end package\n");
                            close(rdffd);
                            package_init_s(&sps,0xA4,0x00);
                            int write_ret=write(connfd,&sps,sizeof(sps));
                            if(write_ret==-1) {perror("write error\n");exit(2);}
                            printf("[Server] Server write finish %d\n",write_ret);
                            pre_pkg=PS_SIZE;
                        }
                        else {
                            printf("[Server] Send new package\n");
                            package_init_l(&spl,0xA3,0x00);
                            spl.len=htons(file_read_ret);
                            memcpy(spl.data,s_buf,file_read_ret);
                            get_checksum((int*)&spl,PL_SIZE);
                            int write_ret=write(connfd,&spl,sizeof(spl));
                            if(write_ret==-1) {perror("write error\n");exit(2);}
                            printf("[Server] Server write finish %d\n",write_ret);
                            pre_pkg=PL_SIZE;
                        }
                    }else if(rps.cmd==0x14){
                        printf("[Server] Transfer finished\n");
                        break;
                    }
                }
                else if(read_ret==PM_SIZE){
                    //接受配置项包
                    memcpy(&rpm,r_buf,read_ret);
                    if(do_checksum((uint32_t*)&rpm,PM_SIZE)==-1) {printf("[Server] checksum failed\n");exit(2);}
                    else printf("[Server] checksum succeed\n");
                    rpm.hw_len=ntohs(rpm.hw_len);
                    rpm.sw_len=ntohs(rpm.sw_len);
                    rpm.fc_len=ntohs(rpm.fc_len);
                    printf("\thardware: %s,%d\n",rpm.hw_dat,rpm.hw_len);
                    printf("\tsoftware: %s,%d\n",rpm.sw_dat,rpm.sw_len);
                    printf("\tfactory: %s,%d\n",rpm.fc_dat,rpm.fc_len);;
                    //查找本地文件
                    int find_file=0;
                    find_file=1;
                    rdffd=open(SIM_FILE,O_RDONLY);
                    if(rdffd==-1) {perror("open error\n");exit(2);}
                    //发送查找结果包
                    if(find_file==1){
                        package_init_s(&sps,0xA2,0x00);
                        int write_ret=write(connfd,&sps,sizeof(sps));
                        if(write_ret==-1) {perror("write error\n");exit(2);}
                        printf("[Server] Server write finish %d\n",write_ret);
                        pre_pkg=PS_SIZE;
                    }
                    else if(find_file==0){
                        package_init_s(&sps,0xA2,0xFF);
                        sps.info[1]=ENOENT;
                        int write_ret=write(connfd,&sps,sizeof(sps));
                        if(write_ret==-1) {perror("write error\n");exit(2);}
                        printf("[Server] Server write finish %d\n",write_ret);
                        pre_pkg=PS_SIZE;
                    }
                    //break;
                }
            }
        }
        close(connfd);
    }
}
