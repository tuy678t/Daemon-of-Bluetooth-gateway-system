#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <wait.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "myfun.h"

/*
    宏定义
    XML_NAME    为版本回退中会检查的list.xml文件的相对路径
    EXE_PATH    为守护进程所守护的网关主程序的相对路径
    FILE_NAME   为更新文件包被本地存放或者下载的位置的相对路径
    TMP_XML_NAME为更新文件包中的list.xml的相对路径
    FD_BUF_SIZE 为守护进程exec网关主程序所传递文件描述符的字符串缓冲数组的大小
    BUF_SIZE为  为通用缓冲区大小
    MAX_FILE_NUM为list.xml中最大的文件数量
    MAX_NAME_LEN为list.xml中最大的文件名长度
    数组
    fb          为传递文件描述符的字符串缓冲数组
    rbuf        为通用接收缓冲
    suf         为通用发送缓冲
    update_list 为通过解析list.xml获得的要更新文件的列表
*/

#define XML_NAME "service/list.xml"
#define EXE_PATH "service/ncp_host"
#define FILE_NAME "tmp/service.tar.gz"
#define TMP_XML_NAME "tmp/service/list.xml"
#define FD_BUF_SIZE 10
char fb[FD_BUF_SIZE];
#define BUF_SIZE 4096
char rbuf[BUF_SIZE];
char sbuf[BUF_SIZE];
#define MAX_FILE_NUM 100
#define MAX_NAME_LEN 200
char update_list[MAX_FILE_NUM][MAX_NAME_LEN];


/*
    service_change  版本回退中解析list.xml文件的过程中，检测有没有更新ncp_host，如果更新了，service_change为1，反之，service_change为0
    get_list 通过解析filename的xml文件，获得service_change和update_list的信息，返回值为update_list的有效条数

*/

int service_change=0;
int get_list(const char* filename){
    xmlDocPtr doc;
    xmlNodePtr curNode;
    int listcnt=0;

    doc = xmlParseFile(filename);
    if ( doc==NULL){perror("xmlParseFile failed\n");xmlFreeDoc(doc);return -1;}

    curNode = xmlDocGetRootElement(doc);
    if ( curNode==NULL){perror("xmlDocGetRootElement failed\n");xmlFreeDoc(doc);return -1;}

    curNode = curNode->children;
    
    while(curNode!=NULL){
        if (!(xmlStrcmp(curNode->name, (const xmlChar *)"file"))) {
            strcpy(update_list[listcnt++],(const char*)xmlNodeGetContent(curNode));
            if(strcmp((char *)xmlNodeGetContent(curNode),EXE_PATH)==0) service_change=1;
        }
        curNode=curNode->next;
    }

    xmlFreeDoc(doc);
    return listcnt;
}

/*
    sys_cmd_str 为执行回退的过程中，用于拼接命令所用的缓冲数组
    update      为更新函数，指定文件名，对于文件进行备份以及覆盖
*/

char sys_cmd_str[64];
int update(const char*filename){
    memset(sys_cmd_str,0,sizeof(sys_cmd_str));
    if(access(filename,F_OK)==0){
        if(strcmp(XML_NAME,filename)!=0){
            strcpy(sys_cmd_str,"cp ");
            strcat(sys_cmd_str,filename);
            strcat(sys_cmd_str," ");
            strcat(sys_cmd_str,filename);
            strcat(sys_cmd_str,".bk");
            system(sys_cmd_str);
            printf("[Daemon]\tDaemon system %s\n",sys_cmd_str);
        }
    }
    memset(sys_cmd_str,0,sizeof(sys_cmd_str));
    strcpy(sys_cmd_str,"cp tmp/");
    strcat(sys_cmd_str,filename);
    strcat(sys_cmd_str," ");
    strcat(sys_cmd_str,filename);
    system(sys_cmd_str);
    printf("[Daemon]\tDaemon system %s\n",sys_cmd_str);  
    return 0;
}

/*
    


*/

int recall(const char*filename){
    if(strcmp(filename,XML_NAME)==0){
        memset(sys_cmd_str,0,sizeof(sys_cmd_str));
        strcpy(sys_cmd_str,"rm -rf ");
        strcat(sys_cmd_str,filename);
        system(sys_cmd_str);
        printf("[Daemon]\tDaemon system %s\n",sys_cmd_str);  
        strcat(sys_cmd_str,".bk");
        system(sys_cmd_str);
        printf("[Daemon]\tDaemon system %s\n",sys_cmd_str);
    }else{
        memset(sys_cmd_str,0,sizeof(sys_cmd_str));
        strcpy(sys_cmd_str,"cp ");
        strcat(sys_cmd_str,filename);
        strcat(sys_cmd_str,".bk");
        strcat(sys_cmd_str," ");
        strcat(sys_cmd_str,filename);
        system(sys_cmd_str);
        printf("[Daemon]\tDaemon system %s\n",sys_cmd_str);  
        memset(sys_cmd_str,0,sizeof(sys_cmd_str));
        strcpy(sys_cmd_str,"rm -rf ");
        strcat(sys_cmd_str,filename);
        strcat(sys_cmd_str,".bk");
        system(sys_cmd_str);
        printf("[Daemon]\tDaemon system %s\n",sys_cmd_str);  
    }
    return 0;
}



int main(int argc,char *argv[]){

    if(daemon(1,1)<0){
        perror("daemon failed!\n");
        exit(1);
    }

    int fd[2];
    int ret = socketpair( AF_UNIX, SOCK_STREAM, 0, fd );
    if(ret<0){
        perror("socketpair failed!\n");
        exit(2);
    }

    int spid=fork();
    if(spid<0){
        perror("fork failed!\n");
        exit(3);        
    }else if(spid==0){
        close(fd[0]);
        memset(fb,0,sizeof(fb));
        snprintf(fb, sizeof(fb),"%d",fd[1]);
        if(execl(EXE_PATH,"ncp_host",fb,NULL) == -1){ perror("execl failed\n");exit(4);}
    }
    else{
        close(fd[1]);
        printf("[Daemon]\tDaemon start\n");
        writelog("[Daemon]\tDaemon start");

        Package_S send_pkg_s;
        Package_M send_pkg_m;
        Package_S recv_pkg_s;
        Package_M recv_pkg_m;
        Package_L recv_pkg_l;

        int ercnt=3;

        while (1) {
            printf("[Daemon]\tDaemon is reading\n");
            writelog("[Daemon]\tDaemon is reading");
            int read_ret=read(fd[0],rbuf,sizeof(rbuf));
            //sleep(3);
            //int read_ret=read(fd[0],&recv_pkg_m,sizeof(recv_pkg_m));
            printf("[Daemon]\tDaemon read ret:%d\n",read_ret);
            writelog("[Daemon]\tDaemon read return");
            if(read_ret==-1){
                perror("Daemon down!!!\n");
                exit(5);
            }else if(read_ret==PM_SIZE){
                memcpy(&recv_pkg_m,rbuf,read_ret);
                if(recv_pkg_m.flag==0x01||recv_pkg_m.flag==0x02){
                    ercnt=3;
                    int sockfd=-1;
                    while(ercnt--){
                        sockfd=seek_sock();
                        if(sockfd==-1) sleep(3);
                        else break;
                    }
                    if(sockfd==-1){
                        printf("[Daemon]\tDaemon cannot Server\n");
                        writelog("[Daemon]\tDaemon cannot connect server");
                        package_init_s(&send_pkg_s,0x02,0xFF);
                        send_pkg_s.info[1]=errno;
                        int write_ret=write(fd[0],&send_pkg_s,sizeof(send_pkg_s));
                        printf("[Daemon]\tDaemon write ret:%d\n",write_ret);
                        if(write_ret==-1){perror("write failed\n");exit(6);}
                        continue;
                    }
                    else{
                        printf("[Daemon]\tDaemon connect Server\n");
                        writelog("[Daemon]\tDaemon connect Server");


                        send_pkg_m=recv_pkg_m;
                        send_pkg_m.hw_len=htons(recv_pkg_m.hw_len);
                        send_pkg_m.sw_len=htons(recv_pkg_m.sw_len);
                        send_pkg_m.fc_len=htons(recv_pkg_m.fc_len);
                        send_pkg_m.checksum=0;
                        get_checksum((int*)&send_pkg_m,sizeof(send_pkg_m));

                        fd_set read_fdset;
                        struct timeval timeout;
                        FD_ZERO(&read_fdset);
                        FD_SET(sockfd,&read_fdset);
                        timeout.tv_sec=10;
                        timeout.tv_usec=0;
                        int sel_ret=-1;


                        while(1){
                            int write_ret=write(sockfd,&send_pkg_m,sizeof(send_pkg_m));
                            if(write_ret==-1) {perror("write error\n");exit(2);}
                            printf("[Daemon]\tDaemon write finish %d\n",write_ret);
                            do{
                                sel_ret=select(sockfd+1,&read_fdset,NULL,NULL,&timeout);
                            }while(sel_ret<0&&errno==EINTR);
                            if(sel_ret==0){
                                printf("[Daemon]\tServer no response, send again\n");
                                continue;
                            }
                            else if(sel_ret==-1) {perror("select error\n");exit(2);}
                            else{
                                int read_ret=read(sockfd,&recv_pkg_s,sizeof(recv_pkg_s));
                                if(read_ret==-1) {perror("read error\n");exit(2);}
                                else if(read_ret==PS_SIZE){
                                    if(recv_pkg_s.cmd==0xA2){
                                        if(recv_pkg_s.flag==0x00){
                                            printf("[Daemon]\tServer find file\n");
                                            break;
                                        }
                                        else if(recv_pkg_s.flag==0xFF){
                                            printf("[Daemon]\tServer do not find file\n");
                                            send_pkg_s=recv_pkg_s;
                                            send_pkg_s.cmd=0x02;
                                            int write_ret=write(fd[0],&send_pkg_s,sizeof(send_pkg_s));
                                            printf("[Daemon]\tDaemon write ret:%d\n",write_ret);
                                            if(write_ret==-1){perror("write failed\n");exit(6);}
                                        }
                                    }
                                    else{
                                        printf("[Daemon]\tDaemon get unknown cmd\n");
                                        continue;
                                    }
                                }
                                else {
                                    printf("[Daemon]\tDaemon read error\n");
                                }

                            }
                        }


                        package_init_s(&send_pkg_s,0x12,0x00);
                        int write_ret=write(sockfd,&send_pkg_s,sizeof(send_pkg_s));
                        printf("[Daemon]\tDaemon write ret:%d\n",write_ret);
                        if(write_ret==-1){perror("write failed\n");exit(6);}

                        int write_file_fd=open(FILE_NAME,O_CREAT|O_WRONLY|O_TRUNC);
                        printf("[Daemon]\tDaemon open succeed\n");        
                        while(1){
                            int read_ret=read(sockfd,rbuf,sizeof(rbuf));
                            if(read_ret==-1) {perror("read error\n");exit(2);}
                            else if(read_ret==PL_SIZE){
                                printf("[Daemon]\tDaemon read FILE package\n");
                                memcpy(&recv_pkg_l,rbuf,read_ret);
                                if(do_checksum((int*)&recv_pkg_l,PL_SIZE)==0){
                                    printf("[Daemon]\tFILE package checksum OK\n");
                                }else{
                                    printf("[Daemon]\tFILE package checksum FAILED\n");
                                    continue;
                                }
                                recv_pkg_l.len=ntohs(recv_pkg_l.len);
                                int write_ret=write(write_file_fd,recv_pkg_l.data,recv_pkg_l.len);
                                if(write_ret==-1){perror("write file failed\n");exit(6);}
                                package_init_s(&send_pkg_s,0x13,0x00);
                                write_ret=write(sockfd,&send_pkg_s,sizeof(send_pkg_s));
                                if(write_ret==-1){perror("write failed\n");exit(6);}

                            }
                            else if(read_ret==PS_SIZE){
                                package_init_s(&send_pkg_s,0x14,0x00);
                                int write_ret=write(sockfd,&send_pkg_s,sizeof(send_pkg_s));
                                if(write_ret==-1){perror("write failed\n");exit(6);}
                                close(write_file_fd);
                                close(sockfd);
                                break;
                            }
                        }
                        printf("[Daemon]\tDaemon transfer finish\n");
                        package_init_s(&send_pkg_s,0x02,0x00);
                        write_ret=write(fd[0],&send_pkg_s,sizeof(send_pkg_s));
                        printf("[Daemon]\tDaemon write ret:%d",write_ret);

                        if(recv_pkg_m.flag==0x01){
                            if(kill(spid,SIGKILL)<0){perror("kill failed\n");exit(9);}
                            printf("[Daemon]\tDaemon kill service\n");
                            wait(NULL);
                            close(fd[0]);
                        }

                        system("tar -xzvf tmp/service.tar.gz -C tmp");
                        printf("[Daemon]\tDaemon tar finish\n");
                        int listcnt=get_list(TMP_XML_NAME);
                        int i=0;
                        printf("[Daemon]\tDaemon update list\n");
                        for(i=0;i<listcnt;i++){
                            printf("[Daemon]\t%s\n",update_list[i]);
                            update(update_list[i]);
                        }

                        if(recv_pkg_m.flag==0x01){
                            ret = socketpair( AF_UNIX, SOCK_STREAM, 0, fd );
                            if(ret<0){
                                perror("socketpair failed!\n");
                                exit(2);
                            }
                            spid=fork();
                            if(spid<0){
                                perror("new fork failed!\n");
                                exit(3); 
                            }else if(spid==0){
                                close(fd[0]);
                                memset(fb,0,sizeof(fb));
                                snprintf(fb, sizeof(fb),"%d",fd[1]);
                                if(execl(EXE_PATH,"ncp_host",fb,NULL) == -1){ perror("execl failed\n");exit(4);}
                            }else{
                                close(fd[1]);
                            }
                        }
                        //system("rm -rf tmp/*");
                        printf("[Daemon]\tDaemon update service\n");
                    }
                }
                else if(recv_pkg_m.flag==0x03||recv_pkg_m.flag==0x04){
                    printf("[Daemon]\tDaemon local update service\n");
                    if(access(XML_NAME,F_OK)==0){
                        package_init_s(&send_pkg_s,0x02,0x00);
                        int write_ret=write(fd[0],&send_pkg_s,sizeof(send_pkg_s));
                        printf("[Daemon]\tDaemon write ret:%d",write_ret);
                    }
                    else{
                        package_init_s(&send_pkg_s,0x02,0xFF);
                        send_pkg_s.info[1]=errno;
                        int write_ret=write(fd[0],&send_pkg_s,sizeof(send_pkg_s));
                        printf("[Daemon]\tDaemon write ret:%d",write_ret);                        
                    }

                    if(recv_pkg_m.flag==0x03){
                        if(kill(spid,SIGKILL)<0){perror("kill failed\n");exit(9);}
                        printf("[Daemon]\tDaemon kill service\n");
                        wait(NULL);
                        close(fd[0]);
                    }


                    system("tar -xzvf tmp/service.tar.gz -C tmp");
                    printf("[Daemon]\tDaemon tar finish\n");
                    int listcnt=get_list(TMP_XML_NAME);
                    int i=0;
                    printf("[Daemon]\tDaemon update list\n");
                    for(i=0;i<listcnt;i++){
                        printf("[Daemon]\t%s\n",update_list[i]);
                        update(update_list[i]);
                    }

                    if(recv_pkg_m.flag==0x03){
                        ret = socketpair( AF_UNIX, SOCK_STREAM, 0, fd );
                        if(ret<0){
                            perror("socketpair failed!\n");
                            exit(2);
                        }
                        spid=fork();
                        if(spid<0){
                            perror("new fork failed!\n");
                            exit(3); 
                        }else if(spid==0){
                            close(fd[0]);
                            memset(fb,0,sizeof(fb));
                            snprintf(fb, sizeof(fb),"%d",fd[1]);
                            if(execl(EXE_PATH,"ncp_host",fb,NULL) == -1){ perror("execl failed\n");exit(4);}
                        }else{
                            close(fd[1]);
                        }
                    }
                    printf("[Daemon]\tDaemon local update service\n");
                }
            }
            else if(read_ret==0){
                printf("[Daemon]\tService Down\n");
                writelog("[Daemon]\tService Down");
                wait(NULL);
                close(fd[0]);
                ret = socketpair( AF_UNIX, SOCK_STREAM, 0, fd );
                if(ret<0){
                    perror("socketpair failed!\n");
                    exit(2);
                }
                spid=fork();
                if(spid<0){perror("new fork failed!\n");exit(3);}
                else if(spid==0){
                close(fd[0]);
                memset(fb,0,sizeof(fb));
                snprintf(fb, sizeof(fb),"%d",fd[1]);
                if(execl(EXE_PATH,"ncp_host",fb,NULL) == -1){ perror("execl failed\n");exit(4);}
                }else{
                    close(fd[1]);
                    continue;
                }
            }
            else if(read_ret==PS_SIZE){
                memcpy(&recv_pkg_s,rbuf,read_ret);
                if(recv_pkg_s.cmd==0x03){
                    service_change=0;
                    int xml_find=0;
                    if(access(XML_NAME,F_OK)==0) xml_find=1;

                    int listcnt=get_list(XML_NAME);
                    printf("[Daemon]\tDaemon get list\n");
                    if(xml_find==0){
                        printf("[Daemon]\tDaemon do not find list\n");
                        package_init_s(&send_pkg_s,0x04,0xFF);
                        int write_ret=write(fd[0],&send_pkg_s,sizeof(send_pkg_s));
                        printf("[Daemon]\tDaemon write ret:%d",write_ret);
                        continue;
                    }
                    else{
                        printf("[Daemon]\tDaemon do find list\n");
                        package_init_s(&send_pkg_s,0x04,0x00);
                        int write_ret=write(fd[0],&send_pkg_s,sizeof(send_pkg_s));
                        printf("[Daemon]\tDaemon write ret:%d",write_ret);
                    }
                    if(service_change==1){
                        if(kill(spid,SIGKILL)<0){perror("kill failed\n");exit(9);}
                        printf("[Daemon]\tDaemon kill service\n");
                        wait(NULL);
                        close(fd[0]);
                    }
                    int i=0;
                    printf("[Daemon]\tDaemon get updated list\n");
                    for(i=0;i<listcnt;i++){
                        printf("[Daemon]\t%s\n",update_list[i]);
                        recall(update_list[i]);
                    }
                    if(service_change==1){
                        ret = socketpair( AF_UNIX, SOCK_STREAM, 0, fd );
                        if(ret<0){
                            perror("socketpair failed!\n");
                            exit(2);
                        }
                        spid=fork();
                        if(spid<0){
                            perror("new fork failed!\n");
                            exit(3); 
                        }else if(spid==0){
                            close(fd[0]);
                            memset(fb,0,sizeof(fb));
                            snprintf(fb, sizeof(fb),"%d",fd[1]);
                            if(execl(EXE_PATH,"ncp_host",fb,NULL) == -1){ perror("execl failed\n");exit(4);}
                        }else{
                            close(fd[1]);
                        }
                    }
                    printf("[Daemon]\tDaemon recall service\n");
                    continue;
                }
            }
            else{
                printf("[Daemon]\tDaemon down\n");
                exit(5);
            }
        }
    }

}